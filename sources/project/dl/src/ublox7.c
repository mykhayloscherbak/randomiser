/**
 * @file ublox7.c
 * @brief Contains driver layer for ublox7 gps receiver
 * @date 16-08-2019
 * @author Mykhaylo Shcherbak
 * @version 1.0
 * @em mikl74@yahoo.com
 */

#include "project_config.h"
#include "uart.h"
#include "ublox7.h"
#include "ublox7_data.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"
#include "time_t.h"
#include "timezone.h"
#include <string.h>

#include "uc1701x.h"
#include "arial8.h"

#define JOIN_CLASS_ID(CLASS,ID) (((CLASS & 0xFF) << 8) | (ID & 0xFF))

#define STACK_SIZE (configMINIMAL_STACK_SIZE + 64)

typedef enum
{
	rxWaitingB5 = 0,
	rxWaiting62,
	rxGettingHeader,
	rxGettingData,
	rxGettingCRC
} rxState;

typedef enum
{
	udsIDLE = 0,
	udsCFGPRT_ACK_WAIT,
	udsCFGMSGRATE,
	udsCFGTP5,
	udsWorking
}Ublox_Driver_state_t;

typedef struct
{
	uint8_t prtAck;
	uint8_t msgRateAck;
	uint8_t tpAck;
} Ack_t;

static Ack_t Ack = {0,0,0};
static struct ublox_NAV_TIMEUTC_s utc;
static xSemaphoreHandle gpsTimeSem = NULL;

typedef void (*filler_t)(uint8_t * const pTX,void * const pData);

static void rxCallBack(const uint8_t c,void * ubloxData);
static uint8_t txCallBack(uint8_t * pC,void * ubloxData);

static ublox_uart_data_t ubxData = {.txSize = 0, .txPos = 0, .state = TX_IDLE};

static void rxCallBack(const uint8_t c,void * Data) /* This function is executed in ISR context */
{
	ublox_uart_data_t * const pUbloxData = Data;
	static uint8_t currMsg[MAX_UBLOX_MSG_SIZE];
	static rxState state = rxWaitingB5;
	static uint16_t counter;
	static uint16_t dataLength;
	static const uint8_t headerLength = 4;
	static const uint8_t crcLength = 2;
	switch (state)
	{
	case rxWaitingB5:
		if (0xB5 == c)
		{
			state = rxWaiting62;
		}
		break;
	case rxWaiting62:
		if (0x62 == c)
		{
			state = rxGettingHeader;
			counter = headerLength;
		}
		break;
	case rxGettingHeader:
	  currMsg[headerLength - counter] = c;
		if (--counter == 0)
		{
			dataLength = currMsg[headerLength - 1] << 8 | currMsg[headerLength - 2];
			if (dataLength + headerLength + crcLength + 2 > MAX_UBLOX_MSG_SIZE)
			{
				state = rxWaitingB5;
			}
			else
			{
				counter = dataLength;
				state = rxGettingData;
			}
		}
		break;
	case rxGettingData:
	  currMsg[headerLength + dataLength - counter] = c;
		if (--counter == 0)
		{
			counter = crcLength;
			state = rxGettingCRC;
		}
		break;
	case rxGettingCRC:
	  currMsg[headerLength + dataLength + crcLength - counter] = c;
		if (--counter == 0)
		{
			state = rxWaitingB5;
			xQueueSendFromISR(pUbloxData->rxQueue,currMsg,NULL);
		}
		break;
	default:
		state = rxWaitingB5;
		break;
	}
}

static uint8_t txCallBack(uint8_t * const pC,void * Data)
{
	ublox_uart_data_t * const pUbloxData = Data;
	uint8_t retVal = 0;
	if (pUbloxData != NULL)
	{
		if (TX_IDLE == pUbloxData->state)
		{
			xSemaphoreTakeFromISR(pUbloxData->TXSem,NULL);
			pUbloxData->state = TX_PROCESS;
		}
		if (TX_PROCESS == pUbloxData->state && pUbloxData->txPos < pUbloxData->txSize)
		{
			*pC = pUbloxData->txMsgBuffer[pUbloxData->txPos++];
			retVal = !0;
		}
		else
		{
			Uart_DisableTX();
			pUbloxData->state = TX_IDLE;
			xSemaphoreGiveFromISR(pUbloxData->TXSem,NULL);
		}
	}
	return retVal;
}

static uint16_t calcCRC(uint8_t * Data, uint16_t len)
{
	uint8_t CK_A = 0;
	uint8_t CK_B = 0;
	for (uint16_t i = 0; i < len; i++)
	{
		CK_A += Data[i];
		CK_B += CK_A;
	}
	return CK_B << 8 | CK_A;
}

static uint8_t * ublox_start(void)
{
	xSemaphoreTake(ubxData.TXSem,portMAX_DELAY);
	ubxData.state = TX_IDLE;
	ubxData.txPos = 0;
	uint16_t ptr = 0;
	ubxData.txMsgBuffer[ptr++] = 0xB5;
	ubxData.txMsgBuffer[ptr++] = 0x62;
	return ubxData.txMsgBuffer + ptr;
}

static void ublox_end(void)
{
	uint16_t ptr = 4;
	const uint16_t size = (ubxData.txMsgBuffer[ptr] & 0xFF) | (ubxData.txMsgBuffer[ptr+1] << 8);
	ptr += size + 2; /* ptr = start_of_buffer + 2 (signature) + 2(class and id) + 2(size) + size */
	uint16_t CRC = calcCRC(ubxData.txMsgBuffer + 2,size + 4);
	ubxData.txMsgBuffer[ptr++] = CRC & 0xff;
	ubxData.txMsgBuffer[ptr++] = CRC >> 8;
	ubxData.txSize = size + 8;
	Uart_EnableTX();
	xSemaphoreGive(ubxData.TXSem);
}


static void setAck(const Ublox_Class_t class, const Ublox_Id_t id,const uint8_t isAck)
{
	switch(JOIN_CLASS_ID(class,id))
	{
	case JOIN_CLASS_ID(ucCFG,uiCFGPRT):
					Ack.prtAck = isAck;
	break;
	case JOIN_CLASS_ID(ucCFG,uiCFGMSG):
					Ack.msgRateAck = isAck;
	break;

	case JOIN_CLASS_ID(ucCFG,uiCFGTP5):
			Ack.tpAck = isAck;
	break;

	default:
		break;
	}
}

static void splitClock(struct ublox_NAV_TIMEUTC_s * const pl)
{
	xSemaphoreTake(gpsTimeSem,portMAX_DELAY);
	memcpy(&utc,pl,sizeof(struct ublox_NAV_TIMEUTC_s));
	xSemaphoreGive(gpsTimeSem);
}



static void parse_msg(
		const Ublox_Class_t class,
		const Ublox_Id_t id,
		const uint16_t plLength,
		uint8_t * const payload
		)
{
	switch(JOIN_CLASS_ID(class,id))
	{
	case JOIN_CLASS_ID(ucACK,uiACKACK):
			if (plLength == 2)
			{
				setAck(payload[0],payload[1],1);
			}
	break;
	case JOIN_CLASS_ID(ucACK,uiACKNAK):
				if (plLength == 2)
				{
					setAck(payload[0],payload[1],0);
				}
	break;
	case JOIN_CLASS_ID(ucNAV,uiNAVTIMEUTC):
			if (plLength == 20)
			{
				splitClock((struct ublox_NAV_TIMEUTC_s *)payload);
			}
	break;
	default:
		break;
	}
}


static void configPort(void)
{
	uint8_t * const pTx = ublox_start();
	Ack.prtAck = 0;
	pTx[0] = ucCFG;
	pTx[1] = uiCFGPRT;
	pTx[2] = sizeof(struct ublox_CFG_PRT_s);
	struct ublox_CFG_PRT_s * const pPl = (struct ublox_CFG_PRT_s *)(pTx + 4);
	memset(pPl,0,sizeof(struct ublox_CFG_PRT_s));
	pPl->portID = 1; /* Uart */
	pPl->txReady_en = 0;
	pPl->bfMode_charLen = cfgChars_8;
	pPl->bfMode_parity = cfgParity_none;
	pPl->bfMode_nstop = cfgStop_1;
	pPl->baudRate = 9600;
	pPl->InProto_UBX = 1; /* UBX */
	pPl->OutProto_UBX = 1; /* UBX */
	ublox_end();
}

static void configMsgRate(const uint8_t Class, const uint8_t Id,const uint8_t Rate)
{
	uint8_t * const pTx = ublox_start();
	Ack.msgRateAck = 0;
	pTx[0] = ucCFG;
	pTx[1] = uiCFGMSG;
	pTx[2] = sizeof(struct ublox_CFG_MSGRATE_s);
	struct ublox_CFG_MSGRATE_s * const pPl = (struct ublox_CFG_MSGRATE_s *)(pTx + 4);
	memset(pPl,0,sizeof(struct ublox_CFG_MSGRATE_s));
	pPl->Class = Class;
	pPl->Id = Id;
	pPl->Rate = Rate;
	ublox_end();
}

static void configTimePulse(uint32_t Freq)
{
	uint8_t * const pTx = ublox_start();
		Ack.tpAck = 0;
		pTx[0] = ucCFG;
		pTx[1] = uiCFGTP5;
		pTx[2] = sizeof(struct ublox_CFG_TP5_s);
		struct ublox_CFG_TP5_s * const pPl = (struct ublox_CFG_TP5_s *)(pTx + 4);
		memset(pPl,0,sizeof(struct ublox_CFG_TP5_s));
		pPl->tpIdx = 0;
		pPl->freqPeriod = Freq;
		pPl->freqPeriodLocked = Freq;
		pPl->pulseLenRatio = 0x7FFFFFFF; /* 50% */
		pPl->pulseLenRatioLocked = 0x7FFFFFFF;
		pPl->active = 1;
		pPl->lockGpsFreq = 0;
		pPl->lockOtherSet = 0;
		pPl->isFreq = 1;
		pPl->isLength = 0;
		pPl->alignToTow = 1;
		pPl->gridUtcGps = 1;
		ublox_end();
}

static void ublox_hook(void * p __attribute__((unused)))
{
	static uint8_t currMsg[MAX_UBLOX_MSG_SIZE];
	Ublox_Driver_state_t driverState = udsIDLE;

	while (1)
	{
		if (xQueueReceive(ubxData.rxQueue,currMsg,pdMS_TO_TICKS(2000)))
		{
			uint8_t counter = 0;
			const Ublox_Class_t class = currMsg[counter++];
			const Ublox_Id_t id = currMsg[counter++];
			const uint16_t length = currMsg[counter] | (currMsg[counter+1] << 8);
			counter += 2;
			const uint16_t crc = calcCRC(currMsg,length + counter);
			const uint16_t receivedCrc = currMsg[counter + length]| (currMsg[counter + length +1] << 8);
			if (crc == receivedCrc)
			{
				parse_msg(class,id,length,currMsg + counter);
			}
			if (Ack.prtAck != 0 && udsCFGPRT_ACK_WAIT == driverState)
			{
				Ack.prtAck = 0;
				driverState = udsCFGMSGRATE;
				configMsgRate(ucNAV,uiNAVTIMEUTC,1); /* Every second */
			}
			if (Ack.msgRateAck != 0 && udsCFGMSGRATE == driverState)
			{
				Ack.msgRateAck = 0;
				driverState = udsCFGTP5;
				configTimePulse(100);
			}
			if (Ack.tpAck != 0 && udsCFGTP5 == driverState)
			{
				Ack.tpAck = 0;
				driverState = udsWorking;
			}
			if (udsWorking == driverState)
			{
				static wchar_t Buf[100];
				time_t utc_conv =
				{
						.year = utc.year - 2000,
						.month = utc.month,
						.day = utc.day,
						.hours = utc.hour,
						.mins = utc.min,
						.secs = utc.sec
				};
				gmt2local(&utc_conv,tzData + 29);
				swprintf(Buf,100,L"GPS: %2d:%02d.%02d %c ",utc_conv.hours,utc_conv.mins,utc_conv.secs,(utc.validUTC !=0)?'+':'-');
				uc1701x_puts(0,16,&arial_8ptFontInfo,Buf);
			}
		}
		else
		{
			if (udsIDLE == driverState)
			{
				configPort();
				driverState = udsCFGPRT_ACK_WAIT;
			}
			else
			{
				driverState = udsIDLE;
			}
		}
	}
}

void Ublox7_init(void)
{
	static StaticSemaphore_t UartMutex_buf;
	ubxData.TXSem = xSemaphoreCreateMutexStatic(&UartMutex_buf);
	configASSERT(ubxData.TXSem != NULL);

	static StaticSemaphore_t gpsTimeSem_buf;
	gpsTimeSem = xSemaphoreCreateMutexStatic(&gpsTimeSem_buf);
	configASSERT(gpsTimeSem != NULL);

	static uint8_t queueBuf[MAX_RX_MSGS * MAX_UBLOX_MSG_SIZE];
	static StaticQueue_t queStruct;
	ubxData.rxQueue = xQueueCreateStatic(MAX_RX_MSGS,MAX_UBLOX_MSG_SIZE,queueBuf,&queStruct);
	configASSERT(ubxData.rxQueue != NULL);
	static Uart_Callbacks_t callBackData = {.rxCallback = rxCallBack,.txCallback = txCallBack};
	Uart_Init(&callBackData,(void *)&ubxData);

	static StackType_t ublox_Stack[STACK_SIZE];
	static StaticTask_t ublox_TCB;
	xTaskCreateStatic(ublox_hook,"ublox",STACK_SIZE,NULL,UBLOX_PRIORITY,ublox_Stack,&ublox_TCB);
}

uint8_t gpsGetUTC(time_t  * const ptime)
{
	uint8_t retVal = 0;
	if (gpsTimeSem != NULL)
	{
		xSemaphoreTake(gpsTimeSem,portMAX_DELAY);
		if (utc.validUTC != 0)
		{
			retVal = 1;
			ptime->hours = utc.hour;
			ptime->mins = utc.min;
			ptime->secs = utc.sec;
			ptime->hundredth = 0;
		}
		xSemaphoreGive(gpsTimeSem);
	}
	return retVal;
}

