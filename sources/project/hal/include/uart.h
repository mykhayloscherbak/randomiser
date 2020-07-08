
#ifndef SOURCES_PROJECT_HAL_INCLUDE_UART_H_
#define SOURCES_PROJECT_HAL_INCLUDE_UART_H_
#include "project_config.h"
#include <stdint.h>
#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"

typedef struct
{
	void (*rxCallback)(const uint8_t c,void * Data);
	uint8_t (*txCallback)(uint8_t * pC,void * Data);
} Uart_Callbacks_t;

void Uart_Init(Uart_Callbacks_t * const callBacks, void * Data);
void Uart_EnableTX(void);
void Uart_DisableTX(void);

#endif /* SOURCES_PROJECT_HAL_INCLUDE_UART_H_ */
