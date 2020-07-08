/**
 * @file ublox7.h
 * @brief Contains driver layer for ublox7 gps receiver prototypes
 * @date 16-08-2019
 * @author Mykhaylo Shcherbak
 * @version 1.0
 * @em mikl74@yahoo.com
 */

#ifndef SOURCES_PROJECT_DL_INCLUDE_UBLOX7_H_
#define SOURCES_PROJECT_DL_INCLUDE_UBLOX7_H_
#include "time_t.h"

typedef enum
{
	TX_IDLE = 0,
	TX_PROCESS
} TX_State_t;

typedef enum
{
	ucNAV = 0x01,
	ucRXM = 0x02,
	ucINF = 0x04,
	ucACK = 0x05,
	ucCFG = 0x06,
	ucMON = 0x0A,
	ucAID = 0x0B,
	ucTIM = 0x0D,
	ucLOG = 0x21
}Ublox_Class_t;

typedef enum
{
	uiCFGMSG = 0x01,
	uiCFGPRT = 0x00,
	uiACKACK = 0x01,
	uiACKNAK = 0x00,
	uiNAVTIMEUTC = 0x21,
	uiNAVCLOCK = 0x22,
	uiCFGTP5 = 0x31
}Ublox_Id_t;

struct ublox_uart_data_s;
struct ublox_uart_data_s
{
	uint8_t txMsgBuffer[MAX_UBLOX_MSG_SIZE];
	uint8_t txSize;
	uint8_t txPos;
	TX_State_t state;
	xSemaphoreHandle TXSem;
	QueueHandle_t rxQueue;
};
typedef struct ublox_uart_data_s ublox_uart_data_t;

void Ublox7_init(void);
uint8_t gpsGetUTC(time_t  * const ptime);

#endif /* SOURCES_PROJECT_DL_INCLUDE_UBLOX7_H_ */
