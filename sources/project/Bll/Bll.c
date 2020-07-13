/**
 * @file Bll.c
 * @brief Contains board ID reading and heartbeat functions
 * @author Mikl Scherbak
 * @em mikl74@yahoo.com
 * @date 16-04-2016
 */
#define ID_NO_ID	0x7 /**< No ID switch installed, all pins are pulled-up */
#define POS_BRIGHTNESS 10 /**< Wiper must be in this pos and button must be pressed to switch to brightness mode */
#define POS_NO_POS 99 /**< Illegal position */
#define BAR_NO_BAR 9  /**< Illegal bar */
#define BRIGHTNESS_NO_BRIGHTNESS 15 /**< Illegal brightness */

#include <stm32f1xx.h>
#include <stddef.h>
#include <string.h>
#include <stdint.h>
#include <wchar.h>
#include "BLL.h"
#include "Can_Id.h"
#include "../DL/Gpio.h"
#include "../DL/Adc.h"
#include "../DL/Clock.h"
#include "../DL/buttons.h"
#include "../DL/Spi.h"
#include "../DL/uc1701x.h"
#include "../fonts/arial72.h"
#include "../fonts/arial8.h"
/**
 * @brief Task table element
 */
typedef struct
{
	uint32_t Period; //!< Period of task in tics (10ms per tick)
	uint32_t Phase;  //!< Task phase (remain of division)
	void (*Task)(void); //!< Task function
} Task_table_t;


static void Toggle_Heartbeat (void)
{
	static uint8_t flag = 0;

	if (flag)
	{
		Gpio_Set_Bit(GPIO_HEARTBEAT);
	}
	else
	{
		Gpio_Clear_Bit(GPIO_HEARTBEAT);
	}
	flag = !flag;
}
typedef enum
{
	ST_First_time = 0,
	ST_Greeting,
	ST_Wait_Button_p,
	ST_Wait_Button_rel,
	ST_Wait_Pause
} BL_State_t;

static void goToSleep(void)
{
	Gpio_Clear_Bit(GPIO_RESET);

}

static void wakeUp(void)
{
	uc1701x_init();
}
static void Bl_process(void)
{
	static uint8_t Width = 0;
	static uint8_t Height = 0;

	static BL_State_t state = ST_First_time;
	const wchar_t Line1[]=L"Натисніть";
	const wchar_t Line2[] = L"кнопку";
	const uint8_t x = 10;
	const uint8_t y = 30;
	static uint32_t Timer = 0;

	switch (state)
	{
	case ST_First_time:
		Width = uc1701x_get_font_width(&arial_72ptFontInfo);
		Height = uc1701x_get_font_height(&arial_72ptFontInfo);
		uc1701x_setMirror(UC1701X_MIRROR_none);
		state = ST_Wait_Button_rel;
		break;
	case ST_Wait_Button_rel:
		if (IsSteadyReleased(B_GEN) != 0)
		{
			state = ST_Greeting;
		}
		break;
	case ST_Greeting:
		uc1701x_cls();
		uc1701x_puts(10,48,&arial_8ptFontInfo,Line1);
		uc1701x_puts(15,60,&arial_8ptFontInfo,Line2);
		SetTimer(&Timer,3000);
		state = ST_Wait_Button_p;
		break;
	case ST_Wait_Button_p:
		if (IsSteadyPressed(B_GEN) != 0)
		{
			uc1701x_cls();
			uint8_t random = GetTicksCounter() % 3 + 1;
			wchar_t Buf[2];
			Buf[0]= random + '0';
			Buf[1]= 0;
			const uint8_t x1 = uc1701x_puts(x,y,&arial_72ptFontInfo, Buf);

			for (uint8_t xx = x1; xx < x + Width * 2; xx++ )
			{
				for (uint8_t yy = y; yy < y + Height; yy++ )
				{
					uc1701x_pixel(xx,yy,0);
				}
			}
			SetTimer(&Timer,500);
			state = ST_Wait_Pause;
 		}
		else
		{
			if (IsTimerPassed(Timer) != 0)
			{
				goToSleep();
//				__WFE();
				if (IsPressed(B_GEN) != 0)
				{
					wakeUp();
					state = ST_First_time;
				}
			}
		}
		break;
	case ST_Wait_Pause:
		if (IsTimerPassed(Timer) != 0)
		{
			state = ST_Wait_Button_rel;
		}
		break;
	default:
		state = ST_Wait_Button_rel;
		break;
	}
}


static void BLL_Process_Buttons(void)
{
	Buttons_Process(B_GEN);
}

void BLL_Init(void)
{
}


static void BLL_Test_LCD(void)
{

	uc1701x_set_coordinates(0,0);
	uc1701x_set_contrast(42);
	Bl_process();
	SPI_Transfer(1);
}

uint8_t MainLoop_Iteration(void)
{
	static const Task_table_t TaskTable[]={
			{1,0, BLL_Process_Buttons},
			{50,1,Toggle_Heartbeat},
			{10,2,BLL_Test_LCD},
			{0,0,NULL}
	};
	static uint32_t OldTicksCounter = 0;
	uint32_t TicsCounter = GetTicksCounter();
	uint8_t RetVal = 0;

	if (TicsCounter != OldTicksCounter)
	{
		uint8_t Counter = 0;
		do
		{
			if (((TicsCounter % TaskTable[Counter].Period) == TaskTable[Counter].Phase) &&
					TaskTable[Counter].Task != NULL)
			{
				(*TaskTable[Counter].Task)();
			}
		} while (TaskTable[++Counter].Task != NULL);
		OldTicksCounter = TicsCounter;
		RetVal = 1;
	}
	return RetVal;
}
