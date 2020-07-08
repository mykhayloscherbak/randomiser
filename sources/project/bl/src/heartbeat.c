#include <stdint.h>
#include "FreeRTOS.h"
#include "task.h"
#include "project_config.h"
#include "heartbeat.h"
#include "Gpio.h"

static StackType_t HB_Stack[configMINIMAL_STACK_SIZE];
static StaticTask_t HB_TCB;

static void HeartBeat(void * p __attribute__((unused)))
{
	while(1)
	{
		Gpio_Set_Bit(GPIO_HEARTBEAT);
		vTaskDelay(500);
		Gpio_Clear_Bit(GPIO_HEARTBEAT);
		vTaskDelay(500);
	}
}

void Create_Heartbeat_Task(void)
{
	xTaskCreateStatic(HeartBeat,"HB",configMINIMAL_STACK_SIZE,NULL,HEARTBEAT_PRIORITY,HB_Stack,&HB_TCB);
}
