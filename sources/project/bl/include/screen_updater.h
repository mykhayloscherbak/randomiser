
#ifndef SOURCES_PROJECT_BL_INCLUDE_SCREEN_UPDATER_H_
#define SOURCES_PROJECT_BL_INCLUDE_SCREEN_UPDATER_H_

#include <stdbool.h>
#include "FreeRTOS.h"
#include "semphr.h"
#include "project_config.h"

typedef struct
{
	uint8_t Buf[FB_SIZE];
	bool isChanged;
	uint16_t size;
	SemaphoreHandle_t semaphore;
} Protected_FB_t;

void ScreenUpdate_Task_create(void);
Protected_FB_t * ScreenUpdate_Task_get_FB(void);
void ScreenUpdate_Task_update(void);




#endif /* SOURCES_PROJECT_BL_INCLUDE_SCREEN_UPDATER_H_ */
