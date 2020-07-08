/**
 * @file buttons.c
 * @brief Contains driver layer for buttons and maybe sensors
 * @date 28-11-2019
 * @author Mykhaylo Shcherbak
 * @version 1.0
 * @em mikl74@yahoo.com
 */

#include "project_config.h"
#include "FreeRTOS.h"
#include "task.h"
#include "buttons.h"
#include "Gpio.h"
#define STACK_SIZE (configMINIMAL_STACK_SIZE + 64)

typedef enum
{
	BUTTON_STATE_UNKNOWN = 0,
	BUTTON_STATE_PRESSED,
	BUTTON_STATE_STABLE_PRESSED,
	BUTTON_STATE_LONG_PRESSED,
	BUTTON_STATE_RELEASED,
	BUTTON_STATE_STABLE_RELEASED,
	BUTTON_STATE_LONG_RELEASED
} Button_State_t;

typedef enum
{
	BUTTON_B0 = 0,
	BUTTON_B1,
	BUTTON_B2,
	BUTTON_TOTAL
} Buttons_t;

typedef struct
{
	uint8_t counter;
	Button_State_t state;
}Current_State_t;

static Current_State_t buttonStates[BUTTON_TOTAL] = { [0 ... BUTTON_TOTAL - 1] = {.counter = 0, .state = BUTTON_STATE_UNKNOWN}};
static const Gpio_Desc_t buttonPins[BUTTON_TOTAL]={GPIO_B0,GPIO_B1,GPIO_B2};

static inline void beepOn(void)
{
	Gpio_Set_Bit(GPIO_SPEAKER);
}

static inline void beepOff(void)
{
	Gpio_Clear_Bit(GPIO_SPEAKER);
}

static inline uint8_t isPressedRaw(const Buttons_t button)
{
	uint8_t retVal = 0;
	if (button < BUTTON_TOTAL)
	{
		retVal = !Gpio_Read_Bit(buttonPins[button]);
	}
	return retVal;
}

static void processButton(const Buttons_t button)
{
	if (button < BUTTON_TOTAL)
	{
		Current_State_t * const bState = buttonStates + button;
		if (isPressedRaw(button) != 0) /* Pressed */
		{
			switch (bState->state)
			{
			case BUTTON_STATE_UNKNOWN:
			case BUTTON_STATE_RELEASED:
			case BUTTON_STATE_STABLE_RELEASED:
			case BUTTON_STATE_LONG_RELEASED:
				bState->counter = 0;
				bState->state = BUTTON_STATE_PRESSED;
				break;
			case BUTTON_STATE_PRESSED:
				if (bState->counter++ >= BUTTON_DEBOUNCE)
				{
					beepOn();
					bState->state = BUTTON_STATE_STABLE_PRESSED;
				}
				break;
			case BUTTON_STATE_STABLE_PRESSED:
				if (bState->counter++ >= BUTTON_LONG)
				{
					bState->state = BUTTON_STATE_LONG_PRESSED;
				}
				break;
			case BUTTON_STATE_LONG_PRESSED:
				break;
			default:
				bState->state = BUTTON_STATE_UNKNOWN;
				bState->counter = 0;
				break;
			}
		}
		else /* Released */
		{
			switch (bState->state)
			{
			case BUTTON_STATE_UNKNOWN:
			case BUTTON_STATE_PRESSED:
			case BUTTON_STATE_STABLE_PRESSED:
			case BUTTON_STATE_LONG_PRESSED:
				bState->counter = 0;
				bState->state = BUTTON_STATE_RELEASED;
				break;
			case BUTTON_STATE_RELEASED:
				if (bState->counter++ >= BUTTON_DEBOUNCE)
				{
					bState->state = BUTTON_STATE_STABLE_RELEASED;
				}
				break;
			case BUTTON_STATE_STABLE_RELEASED:
				if (bState->counter++ >= BUTTON_LONG)
				{
					bState->state = BUTTON_STATE_LONG_RELEASED;
				}
				break;
			case BUTTON_STATE_LONG_RELEASED:
				break;
			default:
				bState->state = BUTTON_STATE_UNKNOWN;
				bState->counter = 0;
				break;
			}
		}

	}
}

static void buttons_hook(void * p __attribute__((unused)))
{
	while(1)
	{
		beepOff();
		processButton(BUTTON_B0);
		processButton(BUTTON_B1);
		processButton(BUTTON_B2);
		vTaskDelay(50);
	}
}

void Buttons_init(void)
{
	static StackType_t buttons_Stack[STACK_SIZE];
	static StaticTask_t buttons_TCB;
	xTaskCreateStatic(buttons_hook,"buttons",STACK_SIZE,NULL,BUTTONS_PRIORITY,buttons_Stack,&buttons_TCB);

}
