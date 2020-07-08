/**
 * @file ds3231.c
 * @author Mykhaylo Shcherbak
 * @e mikl74@yahoo.com
 * @date 12-04-2019
 * @version 1.0
 */
#define DS3231M_ADDR 0xD0

#include <stdint.h>
#include <wchar.h>
#include "i2c_blocking.h"
#include "project_config.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "uc1701x.h"
#include "arial8.h"
#include "ds3231m.h"
#include "time_t.h"
#include "semphr.h"
#include "ublox7.h"

typedef enum
{
	DS3231M_Control_A1IE 	= 		1 << 0,
	DS3231M_Control_A2IE 	= 		1 << 1,
	DS3231M_Control_INTCN 	=		1 << 2,
	DS3231M_Control_CONV	=		1 << 5,
	DS3231M_Control_BBSQW   = 		1 << 6,
	DS3231M_Control_EOSC	=		1 << 7
} DS3231M_Control_t;

typedef enum
{
	DS3231M_Status_A1F		=		1 << 0,
	DS3231M_Status_A2F		=		1 << 1,
	DS3231M_Status_BUSY	    =		1 << 2,
	DS3231M_Status_EN32KHZ  = 		1 << 3,
	DS3231M_Status_OSF		= 		1 << 7
} DS3231M_Status_t;

#pragma pack (push,1)
typedef struct
{
	uint8_t SecondsBCD;
	uint8_t MinutesBCD;
	uint8_t HoursBCD;
	uint8_t Day;
	uint8_t DateBCD;
	uint8_t MonthBCD;
	uint8_t YearBCD;
} DateTimeBCD_t;

typedef struct {
	uint8_t	 Control;
	uint8_t  Status;
	uint8_t Aging;
	uint16_t Temperature;
} Extra_t;


typedef struct {
	DateTimeBCD_t DateTimeBCD;
	uint8_t		Alarms[7];
	Extra_t Extra;
} DS3231MFull_t;
#pragma pack(pop)


static void DS3231M_Configure(const uint8_t control)
{
	uint8_t data[2];
	data[0] = control;
	data[1] = DS3231M_Status_A1F | DS3231M_Status_A2F | DS3231M_Status_BUSY | DS3231M_Status_OSF; /* Always clean SQW32 */
	I2c_blocking_write(DS3231M_ADDR,0xE,data,sizeof(data));
}

static void DS3231M_ConvertTime(const DateTimeBCD_t * const DateTimeBCD, time_t * const time)
{
	time->hundredth = 0;
	time->secs 	= (DateTimeBCD->SecondsBCD & 0xF) + ((DateTimeBCD->SecondsBCD >> 4) & 0xF) * 10;
	time->mins 	= (DateTimeBCD->MinutesBCD & 0xF) + ((DateTimeBCD->MinutesBCD >> 4) & 0xF) * 10;
	time->hours = (DateTimeBCD->HoursBCD & 0xF) +  ((DateTimeBCD->HoursBCD >> 4) & 0x1) * 10;
	const uint8_t ampm = (DateTimeBCD->HoursBCD >> 5) & 0x3; /* Compensate am/pm. We always use "military" time */
	const uint8_t compensation[4] = { 0, 20, 0, 12 };
	time->hours += compensation[ampm];
}

static uint32_t __attribute__((used)) DS3231M_GetHundredth(const DateTimeBCD_t * const DateTimeBCD)
{
	time_t time;
	DS3231M_ConvertTime(DateTimeBCD, &time);
	return time.hours * 360000ul + time.mins * 6000ul + time.secs * 100 + time.hundredth;
}

static void DS3231M_CorrectTime(DateTimeBCD_t * const DateTimeBCD, const time_t * const time)
{
	DateTimeBCD->SecondsBCD = ((time->secs / 10 ) << 4) + time->secs % 10;
	DateTimeBCD->MinutesBCD = ((time->mins / 10 ) << 4) + time->mins % 10;

	if ((DateTimeBCD->HoursBCD & 0x40) != 0) /* 12h mode */
	{
		const uint8_t ampm_hour[24] =
		{0x52 /* 12am */,0x41 /* 1am */,0x42,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x50,0x51,
		 0x72 /* 12pm */,0x61 /* 1pm */,0x62,0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x70,0x71
		};
		DateTimeBCD->HoursBCD = ampm_hour[time->hours];
	}
	else
	{
		DateTimeBCD->HoursBCD = ((time->hours / 10 ) << 4) + time->hours % 10;
	}
}

static StackType_t DS_Stack[configMINIMAL_STACK_SIZE + 0x12];
static StaticTask_t DS_TCB;
static QueueHandle_t queueExti;

static void DS3231M_Process(void * p __attribute__((unused)))
{
	DateTimeBCD_t DateTime;
	time_t time;
	I2c_blocking_init();
	DS3231M_Configure(0);
	uint8_t firstTime = 1;

	while(1)
	{
		static wchar_t Buf[100];
		uint16_t ticks;
		xQueueReceive(queueExti,&ticks,portMAX_DELAY);
		I2C_Blocking_read(DS3231M_ADDR,0,(uint8_t *)&DateTime,sizeof(DateTime));
		time_t Correction;
		if (firstTime != 0 && gpsGetUTC(&Correction) != 0)
		{
			firstTime = 0;
			DS3231M_CorrectTime(&DateTime,&Correction);
			I2c_blocking_write(DS3231M_ADDR,0,(uint8_t *)&DateTime,sizeof(DateTime));
		}
		DS3231M_ConvertTime(&DateTime,&time);
		swprintf(Buf,100,L"RTC:%2d:%02d.%02d t=%6d ",time.hours,time.mins,time.secs,ticks);
		uc1701x_puts(0,0,&arial_8ptFontInfo,Buf);
	}
}

void DS3231M_add_to_queue(const uint16_t ticks)
{
	if (queueExti != NULL)
	{
		xQueueSendFromISR(queueExti,&ticks,NULL);
	}
}


void Create_DS3231_Task(void)
{
	static uint8_t qBuf[20];
	static StaticQueue_t queueBuf;
	queueExti = xQueueCreateStatic(10,2,qBuf,&queueBuf);
	xTaskCreateStatic(DS3231M_Process,"DS3231",configMINIMAL_STACK_SIZE,NULL,DS3231M_PRIORITY,DS_Stack,&DS_TCB);
}
