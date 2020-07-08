#include "FreeRTOS.h"
#include "heartbeat.h"
#include "screen_updater.h"
#include "task.h"
#include "Clock.h"
#include "Gpio.h"
#include "ds3231m.h"
#include "i2c_blocking.h"
#include "pwm.h"
#include "ublox7.h"
#include "buttons.h"
#include "stm32f1xx.h"

void main(void)
{
	Clock_HSE_Init();
	Gpio_Init();
	Init_PWM();
	Set_PWM(1);
	Ublox7_init();
	Clock_Tim2_Init();
	Buttons_init();
	Create_Heartbeat_Task();
    ScreenUpdate_Task_create();
    Create_DS3231_Task();
 	vTaskStartScheduler();
	while(1)
	{

	}
}
