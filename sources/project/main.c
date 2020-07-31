/**
 * @file main.c
 * @brief Contains main function
 * @author mikl74@yahoo.com
 */

#include "uc1701x.h"
#include "stm32f1xx.h"
#include "DL/Clock.h"
#include "DL/Gpio.h"
#include "Bll/BLL.h"
#include "DL/Spi.h"
#include "DL/buttons.h"
#include "DL/i2c.h"
#include "DL/st75256.h"


static void Init(void)
{

  Clock_HSI_Init();
  Gpio_Init();
  Clock_Timebase_Init();
  i2cInit();
  LCD_Desc_t desc;
  st75256_init(&desc);
  DelayMsTimer(10);
  //  SPI_Init();
//  Buttons_Init();
//  uc1701x_init();
//  BLL_Init();

}

void main(void)
{
	Init();
	st75256_test();
	while ( 1 )
	{
	//	st75256_test();
		__asm volatile ("nop");
	}
}

