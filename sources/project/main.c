/**
 * @file main.c
 * @brief Contains main function
 * @author mikl74@yahoo.com
 */

#include "uc1701x.h"
#include "stm32f1xx.h"
#include "DL/Clock.h"
#include "DL/Gpio.h"
#include "Bll/Bll.h"
#include "DL/Spi.h"
#include "DL/buttons.h"
#include "DL/pwm.h"


static void Init(void)
{
  Clock_HSI_Init();
  Gpio_Init();
  Clock_Timebase_Init();
  PWM_Init();
  SPI_Init();
  Buttons_Init();
  uc1701x_init();
  BLL_Init();

}

void main(void)
{
	Init();
	while ( 1 )
	{
		MainLoop_Iteration();
	}
}

