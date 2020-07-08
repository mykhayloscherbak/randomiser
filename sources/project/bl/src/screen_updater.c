#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stddef.h>
#include "FreeRTOS.h"
#include "task.h"
#include "project_config.h"
#include "Gpio.h"
#include "uc1701x.h"
#include "Spi.h"
#include "semphr.h"
#include "screen_updater.h"
#include "ublox7.h"

#define STACK_SIZE (configMINIMAL_STACK_SIZE + 32)


static SemaphoreHandle_t screenSema;
static Protected_FB_t Protected_FB = {.isChanged = false, .size = FB_SIZE,.semaphore = NULL};
static void ScreenUpdate_hook(void * p __attribute__((unused)))
{
	SPI_Init(&Protected_FB);
	uc1701x_init();
	uc1701x_set_contrast(40);
	while(1)
	{
		if (xSemaphoreTake(screenSema,1000) == pdPASS)
		{
			xSemaphoreGive(screenSema);
		}
		uc1701x_set_coordinates(0,0);
		SPI_Transfer();
	}
}

void ScreenUpdate_Task_create(void)
{
	static StackType_t Screen_Stack[STACK_SIZE];
	static StaticTask_t Screen_TCB;
	static StaticSemaphore_t FBMutex_buf;
	static StaticSemaphore_t screenSema_buf;


	screenSema = xSemaphoreCreateBinaryStatic(&screenSema_buf);
	Protected_FB.semaphore = xSemaphoreCreateBinaryStatic(&FBMutex_buf);
	xSemaphoreGive(Protected_FB.semaphore);
	if (Protected_FB.semaphore != NULL)
	{
		xTaskCreateStatic(ScreenUpdate_hook,"Screen",STACK_SIZE,NULL,SCREEN_UPDATE_PRIORITY,Screen_Stack,&Screen_TCB);
	}
}

void ScreenUpdate_Task_update(void)
{
	xSemaphoreGive(screenSema);
}

Protected_FB_t * ScreenUpdate_Task_get_FB(void)
{
	return &Protected_FB;
}
