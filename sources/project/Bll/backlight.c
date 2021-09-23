#include <stdint.h>
#include "../DL/pwm.h"
#include "backlight.h"
#include "../DL/Clock.h"
typedef enum
{
	BL_OFF,
	BL_UP,
	BL_DOWN,
	BL_ON
} BL_State_t;

static BL_State_t blState = BL_OFF;
static uint8_t blStep = 0;
static const uint8_t percent[] = {0,1,2,4,6,10,16,25,40,63,100};
static const uint8_t nsteps = sizeof(percent) / sizeof(percent[0]);
static uint32_t timer = 0;
static const uint32_t stepTime = 3;


void backLight_on(void)
{
	if (BL_DOWN == blState || BL_OFF == blState)
	{
		blState = BL_UP;
		SetTimer(&timer,stepTime);
	}
}

void backLight_off(void)
{
	if (BL_UP == blState || BL_ON == blState)
	{
		blState = BL_DOWN;
		SetTimer(&timer,stepTime);
	}
}

void backlight_init(void)
{
	Set_PWM(0);
	blStep = 0;
	blState = BL_OFF;

}

void processBacklight(void)
{
	if (BL_OFF != blState && BL_ON != blState)
	{
		if (IsTimerPassed(timer) != 0)
		{
			SetTimer(&timer,stepTime);
			Set_PWM(percent[blStep]);
			if (BL_DOWN == blState && 0 == blStep)
			{
				blState = BL_OFF;
			}

			if (BL_DOWN == blState && blStep > 0)
			{
				blStep--;
			}

			if (BL_UP == blState && (nsteps - 1) == blStep)
			{
				blState = BL_ON;
			}

			if (BL_UP == blState && blStep < (nsteps - 1))
			{
				blStep++;
			}
		}
	}
}
