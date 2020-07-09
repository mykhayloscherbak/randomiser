/**
 * @file main.c
 * @brief Contains main function
 * @author mikl74@yahoo.com
 */

#include <st75256.h>
#include "stm32f1xx.h"
#include "DL/Clock.h"
#include "DL/Gpio.h"
#include "Bll/BLL.h"
#include "Bll/Can_Id.h"
#include "DL/Adc.h"
#include "DL/Spi.h"
#include "DL/buttons.h"

static void Init(void)
{
  Clock_HSI_Init();
  Gpio_Init();
  Adc_Init();
  Clock_Timebase_Init();
  SPI_Init();
  Buttons_Init();
  st75256_init();
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

