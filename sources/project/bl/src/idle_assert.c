#include "FreeRTOS.h"
#include "task.h"
#include "idle_assert.h"
void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize );

void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize )
{
	static StaticTask_t TCB;
	*ppxIdleTaskTCBBuffer = &TCB;
	static StackType_t Stack[configMINIMAL_STACK_SIZE];
	*pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
	*ppxIdleTaskStackBuffer = Stack;
}

void vAssertCalled( unsigned long ulLine, const char * const pcFileName )
{
//static portBASE_TYPE xPrinted = pdFALSE;
volatile uint32_t ulSetToNonZeroInDebuggerToContinue = 0;

    /* Parameters are not used. */
    ( void ) ulLine;
    ( void ) pcFileName;

    taskENTER_CRITICAL();
    {
        /* You can step out of this function to debug the assertion by using
        the debugger to set ulSetToNonZeroInDebuggerToContinue to a non-zero
        value. */
        while( ulSetToNonZeroInDebuggerToContinue == 0 )
        {
        }
    }
    taskEXIT_CRITICAL();
}
