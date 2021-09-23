/**
 * @file Bll.c
 * @brief contains all business logic
 * @author Mykhaylo Shcherbak
 * @em mikl74@yahoo.com
 * @date 18-07-2020
 */

#include <stm32f1xx.h>
#include <stddef.h>
#include <string.h>
#include <stdint.h>
#include <wchar.h>
#include "Bll.h"
#include "backlight.h"
#include "../DL/Gpio.h"
#include "../DL/Clock.h"
#include "../DL/buttons.h"
#include "../DL/Spi.h"
#include "../DL/uc1701x.h"
#include "../DL/power.h"
#include "../fonts/arial72.h"
#include "../fonts/arial8.h"

#define VERSION L"V1.20"
#define WIDE2(x) L##x
#define WIDE(x) WIDE2(x)

typedef enum
{
	contrast = 36,
	freezeTimeout = 5,
	sleepTimeout = 10 * 60,
	maxSeqLen = 2,
	barY1 = 128 - 15,
	barY2 = 127
} Constants;

/**
 * @brief Task table element
 */
typedef struct
{
	uint32_t Period; //!< Period of task in tics (10ms per tick)
	uint32_t Phase;  //!< Task phase (remain of division)
	void (*Task)(void); //!< Task function
} Task_table_t;

static inline void beeperOn(void)
{
	Gpio_Set_Bit(GPIO_BEEPER);
}

static inline void beeperOff(void)
{
	Gpio_Clear_Bit(GPIO_BEEPER);
}

static void Heartbeat (void)
{
	static uint8_t flag = 0;
	static uint32_t Timer = 0;
	if (!flag)
	{
		Gpio_Set_Bit(GPIO_HEARTBEAT);
	}
	else
	{
		Gpio_Clear_Bit(GPIO_HEARTBEAT);
	}

	if (Timer == 0)
	{
		SetTimer(&Timer,1);
	}
	if (IsTimerPassed(Timer ) != 0)
	{
		if (flag)
		{
			SetTimer(&Timer,500);
		}
		else
		{
			SetTimer(&Timer,20);
		}
		flag = !flag;
	}
}

typedef enum
{
	ST_First_time = 0,
	ST_Version,
	ST_Greeting,
	ST_Wait_Button_p,
	ST_Wait_Button_rel,
	ST_Wait_Pause
} BL_State_t;

static void Sleep(void)
{
	Gpio_Clear_Bit(GPIO_RESET);
	powerSave();
	backlight_init();
	uc1701x_init();
	uc1701x_setMirror(UC1701X_MIRROR_none);
	uc1701x_set_contrast(contrast);
}

static void versionAndModeDisplay(const uint8_t mode)
{
	uc1701x_cls();
	const wchar_t Line1[] = VERSION;
	const wchar_t Line2[] = WIDE(__DATE__);
	const wchar_t Line3[] = WIDE(__TIME__);
	const wchar_t Line4[] = L"Режим";
	const FONT_INFO * const pFont = &arial_8ptFontInfo;
	const uint8_t height = uc1701x_get_font_height(pFont);
	uc1701x_puts(0,0,pFont,Line1);
	uc1701x_puts(0,height,pFont,Line2);
	uc1701x_puts(0,height * 2,pFont,Line3);
	uc1701x_puts(0,height * 3,pFont,Line4);
	uc1701x_putchar(35,height * 3,pFont,mode + '0');
}


static uint32_t calcRandom(const uint32_t seed)
{
	static uint32_t result = 0;
	result = (134775813 * ((seed == 0) ? result : seed) + 1);
	return result;
}

static void drawBars(const uint8_t mode)
{
	const uint8_t barPos = 64 / (mode  + 1);
	for (uint8_t bar = 1; bar <= mode; bar++)
	{
		uc1701x_filled_rect(bar * barPos, barY1, bar * barPos + 1, barY2);
	}
}

static void Bl_process(void)
{

	static BL_State_t state = ST_First_time;
	const wchar_t Line1[]=L"Натисніть";
	const wchar_t Line2[] = L"кнопку";
	const uint8_t y = 30;
	static uint32_t Timer = 0;
	static uint32_t blTimer = 0;
	static uint8_t beepCounter;
	static uint32_t beepTimer;
	static uint8_t randomInited = 0;
	static uint8_t mode = 3;
	uint8_t random;

	switch (state)
	{
	case ST_First_time:
		uc1701x_setMirror(UC1701X_MIRROR_none);
		uc1701x_set_contrast(contrast);
		if (IsPressed(B_GEN) != 0)
		{
			mode = 2;
		}
		backLight_on();
		versionAndModeDisplay(mode);
		SetTimer(&Timer,100);
		state = ST_Version;
		break;
	case ST_Version:
		if (IsTimerPassed(Timer) != 0)
		{
			state = ST_Wait_Button_rel;
		}
		break;
	case ST_Wait_Button_rel:
		if (IsSteadyReleased(B_GEN) != 0)
		{
			beeperOn();
			backLight_on();
			SetTimer(&Timer,50);
			SetTimer(&blTimer,300); /* 3s for backlight */
			state = ST_Greeting;
		}
		break;
	case ST_Greeting:
		if (IsTimerPassed(Timer) != 0)
		{
			beeperOff();
			uc1701x_cls();
			uc1701x_puts(10,48,&arial_8ptFontInfo,Line1);
			uc1701x_puts(15,60,&arial_8ptFontInfo,Line2);
			SetTimer(&Timer,sleepTimeout * 100);
			state = ST_Wait_Button_p;
		}
		break;
	case ST_Wait_Button_p:
		random = (calcRandom(0) >> 5) % mode + 1;
		if (IsTimerPassed(blTimer) != 0)
		{
			backLight_off();
			SetTimer(&blTimer,0xFFFFFFFF);
		}
		if (IsSteadyPressed(B_GEN) != 0)
		{
			backLight_on();
			if (randomInited == 0)
			{
				calcRandom(GetTicksCounter());
				randomInited = 1;
			}
			if (3 == mode)
			{
				static uint8_t previousRandom = 0;
				static uint8_t seqLen = 0;
				if (previousRandom == random)
				{
					seqLen++;
				}
				else
				{
					previousRandom = random;
					seqLen = 0;
				}
				if (seqLen >= maxSeqLen)
				{
					while (random == previousRandom)
					{
						random = (calcRandom(0) >> 5) % mode + 1;
					}
					previousRandom = random;
					seqLen = 0;
				}

			}
			uc1701x_cls();
			beepCounter = (random - 1) * 2;
			wchar_t Buf[2];
			Buf[0]= random + '0';
			Buf[1]= 0;
			uc1701x_cls();
			const uint8_t x = (64 - uc1701x_get_symbol_width(&arial_72ptFontInfo,Buf[0])) / 2;
			uc1701x_puts(x,y,&arial_72ptFontInfo, Buf);
			drawBars(mode);
			SetTimer(&Timer,freezeTimeout * 100);
			SetTimer(&beepTimer,10);
			beeperOn();
			state = ST_Wait_Pause;
 		}
		else
		{
			if (IsTimerPassed(Timer) != 0)
			{
				Sleep();
				state = ST_Wait_Button_rel;
			}
		}
		break; /* Regulator off */
	case ST_Wait_Pause:
		if (beepCounter > 0)
		{
			if (IsTimerPassed(beepTimer) != 0)
			{
				if ((beepCounter & 1) == 0)
				{
					beeperOff();
					SetTimer(&beepTimer,50);
				}
				else
				{
					beeperOn();
					SetTimer(&beepTimer,10);
				}
				beepCounter--;
			}
		}
		else
		{
			beeperOff();

			if (IsTimerPassed(Timer) != 0)
			{
				state = ST_Wait_Button_rel;
			}
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


static void BLL_iteration_and_display(void)
{

	uc1701x_set_coordinates(0,0);
	Bl_process();
	SPI_Transfer(1);
}

uint8_t MainLoop_Iteration(void)
{
	static const Task_table_t TaskTable[]={
			{1,0, BLL_Process_Buttons},
			{1,0, processBacklight},
			{10,1,Heartbeat},
			{10,2,BLL_iteration_and_display},
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
