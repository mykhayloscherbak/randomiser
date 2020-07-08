/**
 * @file Gpio.c
 * @brief Contains init and access functions for Gpio
 * @author Mikl Scherbak
 * @em mikl74@yahoo.com
 * @date 14-01-2019
 * @version 1.1
 */

#include <stddef.h>
#include "stm32f1xx.h"
#include "project_config.h"
#include "Gpio.h"
#include "ds3231m.h"


typedef enum
{
	GPIO_MODE_IN =0,
	GPIO_MODE_OUT,
	GPIO_MODE_DO_NOT_TOUCH
} Gpio_Mode_t;


/**
 * @brief Gpio pin configuration
 */
typedef struct
{
	GPIO_TypeDef *Port; /**< Pointer to port */
	uint8_t Pin; /**< PinNumber */
	Gpio_Mode_t Mode; /**< Mode (in,out, analog) */
}Gpio_Config_t;

static const Gpio_Config_t Gpio_Config[GPIO_TOTAL]=
{
		[GPIO_HEARTBEAT] 	= {.Port = GPIOA,.Pin = 1,	.Mode = GPIO_MODE_OUT	},
		[GPIO_CAN_CTL]  	= {.Port = GPIOA,.Pin = 3,	.Mode = GPIO_MODE_OUT	},
		[GPIO_LCD_CS]  		= {.Port = GPIOA,.Pin = 4,	.Mode = GPIO_MODE_OUT	},
		[GPIO_LCD_SCK]		= {.Port = GPIOA,.Pin = 5,  .Mode = GPIO_MODE_DO_NOT_TOUCH},
		[GPIO_LCD_RS]		= {.Port = GPIOA,.Pin = 6,	.Mode = GPIO_MODE_OUT	},
		[GPIO_LCD_DATA]		= {.Port = GPIOA,.Pin = 7,  .Mode = GPIO_MODE_DO_NOT_TOUCH},
		[GPIO_SPEAKER]		= {.Port = GPIOA,.Pin = 8,	.Mode = GPIO_MODE_OUT	},
		[GPIO_GPS_TX]		= {.Port = GPIOA,.Pin = 9,  .Mode = GPIO_MODE_DO_NOT_TOUCH},
		[GPIO_GPS_RX]		= {.Port = GPIOA,.Pin = 10,  .Mode = GPIO_MODE_DO_NOT_TOUCH},
		[GPIO_SQW]			= {.Port = GPIOA,.Pin = 15,	.Mode = GPIO_MODE_DO_NOT_TOUCH},
		[GPIO_S0]			= {.Port = GPIOB,.Pin = 0,  .Mode = GPIO_MODE_IN	},
		[GPIO_S1]			= {.Port = GPIOB,.Pin = 1,  .Mode = GPIO_MODE_IN	},
		[GPIO_S2]			= {.Port = GPIOB,.Pin = 2,  .Mode = GPIO_MODE_IN	},
		[GPIO_S3]			= {.Port = GPIOB,.Pin = 3,  .Mode = GPIO_MODE_IN	},
		[GPIO_B0]			= {.Port = GPIOB,.Pin = 4,  .Mode = GPIO_MODE_IN	},
		[GPIO_B1]			= {.Port = GPIOB,.Pin = 5,  .Mode = GPIO_MODE_IN	},
		[GPIO_B2]			= {.Port = GPIOB,.Pin = 10, .Mode = GPIO_MODE_IN	},
		[GPIO_LCD_PWM]		= {.Port = GPIOB,.Pin = 6,	.Mode = GPIO_MODE_DO_NOT_TOUCH},
		[GPIO_GPS_PWR]	   	= {.Port = GPIOB,.Pin = 7,  .Mode = GPIO_MODE_OUT	},
		[GPIO_GPS_PPS]		= {.Port = GPIOB,.Pin = 11, .Mode = GPIO_MODE_IN	},
		[GPIO_LCD_RESET]	= {.Port = GPIOB,.Pin = 12, .Mode = GPIO_MODE_OUT	},
		[GPIO_I2C_SCL]	    = {.Port = GPIOB,.Pin = 8,  .Mode = GPIO_MODE_DO_NOT_TOUCH},
		[GPIO_I2C_SDA]	    = {.Port = GPIOB,.Pin = 9,  .Mode = GPIO_MODE_DO_NOT_TOUCH}
};

static void Gpio_Init_Exti(const Gpio_Desc_t gpio)
{
	GPIO_TypeDef *port;
	uint8_t pin;
	Gpio_Get_Alt_PortPin(gpio,&port,&pin);
	if ( pin < 8 )
	{
		port->CRL &= ~( 0b11u<< ( pin * 4));
		port->CRL &= ~( 0b01u<< ( pin * 4 + 2));
		port->CRL |=  ( 0b10u<< ( pin * 4 + 2));
		port->ODR |=  ( 1u<< pin ); /* Pull-Up */
	}
	else
	{
		port->CRH &= ~( 0b11u<< ( (pin - 8) * 4));
		port->CRH &= ~( 0b01u<< ( (pin - 8) * 4 + 2));
		port->CRH |=  ( 0b10u<< ( (pin - 8) * 4 + 2));
		port->ODR |=  ( 1u<< pin ); /* Pull-Up */
	}
	const GPIO_TypeDef * const ports[] = {GPIOA,GPIOB,GPIOC,GPIOD,GPIOE};
	uint8_t portnum = 0;
	for (portnum = 0; portnum < sizeof(ports) / sizeof(ports[0]); portnum++)
	{
		if (port == ports[portnum])
		{
			break;
		}
	}
	const uint8_t shift = (pin & 0x3) * 4;
	AFIO->EXTICR[pin >> 2] |= portnum << shift;
	AFIO->EXTICR[pin >> 2] &= ( portnum << shift ) | ~(0xF << shift);
	EXTI->RTSR |= 1u << pin;
	EXTI->IMR  |= 1u << pin;
	const IRQn_Type IrqNs[] = {
			EXTI0_IRQn,EXTI1_IRQn,EXTI2_IRQn,EXTI3_IRQn,EXTI4_IRQn,
			EXTI9_5_IRQn,EXTI9_5_IRQn,EXTI9_5_IRQn,EXTI9_5_IRQn,EXTI9_5_IRQn,
			EXTI15_10_IRQn,EXTI15_10_IRQn,EXTI15_10_IRQn,EXTI15_10_IRQn,EXTI15_10_IRQn,EXTI15_10_IRQn
	};
	NVIC_SetPriority(IrqNs[pin],LOW_SYSTEM_INTERRUPT_PRIORITY - 1);
	NVIC_EnableIRQ(IrqNs[pin]);
}

static void process_exti(const uint8_t Exti_No)
{
	GPIO_TypeDef * port;
	uint8_t pin;
	Gpio_Get_Alt_PortPin(GPIO_SQW,&port,&pin);
	if (Exti_No == pin)
	{
		const uint16_t savedCNT = TIM2->CNT;
		TIM2->CNT = 0;
		DS3231M_add_to_queue(savedCNT);
	}
	Gpio_Get_Alt_PortPin(GPIO_GPS_PPS,&port,&pin);
	if (Exti_No == pin)
	{
//		__asm__("bkpt 1");
	}

}

/**
 * Processes exti.
 * @param EXTI_Number 0-4 are real numbers, 5 and 10 are for 5-9 and 10-15
 */
static void Gpio_process_exti(const uint8_t EXTI_Number)
{
	uint8_t realEXTI = 0;
	uint8_t i;
	switch(EXTI_Number)
	{
	case 0:
	case 1:
	case 2:
	case 3:
	case 4:
		realEXTI = EXTI_Number;
		break;
	case 5:
		for (i = 5; i <= 9; i++)
		{
			if ((EXTI->PR & (1u << i)) != 0)
			{
				break;
			}
		}
		realEXTI = i;
		break;
	case 10:
		for (i = 10; i <= 15; i++)
		{
			if ((EXTI->PR & (1u << i)) != 0)
			{
				break;
			}
		}
		realEXTI = i;
		break;
	default:
		realEXTI = 16;
		break;
	}
	if (realEXTI != 16)
	{
		process_exti(realEXTI);
		EXTI->PR = 1u << realEXTI;
	}
}

static void Gpio_Init_Extis(void)
{
	Gpio_Init_Exti(GPIO_SQW);
	Gpio_Init_Exti(GPIO_GPS_PPS);
}

void EXTI0_IRQHandler(void);
void EXTI0_IRQHandler(void)
{
	Gpio_process_exti(0);
}

void EXTI1_IRQHandler(void);
void EXTI1_IRQHandler(void)
{
	Gpio_process_exti(1);
}

void EXTI2_IRQHandler(void);
void EXTI2_IRQHandler(void)
{
	Gpio_process_exti(2);
}

void EXTI3_IRQHandler(void);
void EXTI3_IRQHandler(void)
{
	Gpio_process_exti(3);
}

void EXTI4_IRQHandler(void);
void EXTI4_IRQHandler(void)
{
	Gpio_process_exti(4);
}

void EXTI9_5_IRQHandler(void);
void EXTI9_5_IRQHandler(void)
{
	Gpio_process_exti(5);
}

void EXTI15_10_IRQHandler(void);
void EXTI15_10_IRQHandler(void)
{
	Gpio_process_exti(10);
}


void Gpio_Init(void)
{
	Gpio_Desc_t Pin;
	RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_IOPBEN | RCC_APB2ENR_AFIOEN;
	AFIO->MAPR |= AFIO_MAPR_SWJ_CFG_1;
	AFIO->MAPR &= ~(AFIO_MAPR_SWJ_CFG_0 | AFIO_MAPR_SWJ_CFG_2);
	for ( Pin = 0; Pin < GPIO_TOTAL; Pin++)
	{
		if ( Gpio_Config[Pin].Pin < 8 )
		{
			if ( Gpio_Config[Pin].Mode == GPIO_MODE_IN)
			{
				(Gpio_Config[Pin].Port)->CRL &= ~( 0b11u<< ( Gpio_Config[Pin].Pin * 4));
				(Gpio_Config[Pin].Port)->CRL &= ~( 0b01u<< ( Gpio_Config[Pin].Pin * 4 + 2));
				(Gpio_Config[Pin].Port)->CRL |=  ( 0b10u<< ( Gpio_Config[Pin].Pin * 4 + 2));
				(Gpio_Config[Pin].Port)->ODR |=  ( 1u<< Gpio_Config[Pin].Pin ); /* Pull-Up */
			}
			else
			{
				if ( Gpio_Config[Pin].Mode == GPIO_MODE_OUT )
				{
					(Gpio_Config[Pin].Port)->CRL &= ~( 0b10u<< ( Gpio_Config[Pin].Pin * 4));
					(Gpio_Config[Pin].Port)->CRL |=  ( 0b10u<< ( Gpio_Config[Pin].Pin * 4));
					(Gpio_Config[Pin].Port)->CRL &= ~( 0b11u<< ( Gpio_Config[Pin].Pin * 4 + 2));
					(Gpio_Config[Pin].Port)->BRR |=  ( 1u<< Gpio_Config[Pin].Pin );
				}
			}
		}
		else
		{
			if ( Gpio_Config[Pin].Mode == GPIO_MODE_IN)
			{
				(Gpio_Config[Pin].Port)->CRH &= ~( 0b11u<< ( (Gpio_Config[Pin].Pin - 8) * 4));
				(Gpio_Config[Pin].Port)->CRH &= ~( 0b01u<< ( (Gpio_Config[Pin].Pin - 8) * 4 + 2));
				(Gpio_Config[Pin].Port)->CRH |=  ( 0b10u<< ( (Gpio_Config[Pin].Pin - 8) * 4 + 2));
				(Gpio_Config[Pin].Port)->ODR |=  ( 1u<< Gpio_Config[Pin].Pin ); /* Pull-Up */
			}
			else
			{
				if ( Gpio_Config[Pin].Mode == GPIO_MODE_OUT )
				{
					(Gpio_Config[Pin].Port)->CRH &= ~( 0b10u<< ( (Gpio_Config[Pin].Pin - 8) * 4));
					(Gpio_Config[Pin].Port)->CRH |=  ( 0b10u<< ( (Gpio_Config[Pin].Pin - 8) * 4));
					(Gpio_Config[Pin].Port)->CRH &= ~( 0b11u<< ( (Gpio_Config[Pin].Pin -8 ) * 4 + 2));
					(Gpio_Config[Pin].Port)->BRR |=  ( 1u<< Gpio_Config[Pin].Pin );
				}
			}
		}
	}
	Gpio_Init_Extis();
}

void Gpio_Set_Bit(Gpio_Desc_t Gpio)
{
	if ( Gpio < GPIO_TOTAL )
	{
		(Gpio_Config[Gpio].Port)->BSRR = 1u<<Gpio_Config[Gpio].Pin;
	}
}

void Gpio_Clear_Bit(Gpio_Desc_t Gpio)
{
	if ( Gpio < GPIO_TOTAL )
	{
		(Gpio_Config[Gpio].Port)->BSRR = 1u<<(Gpio_Config[Gpio].Pin + 16);
	}
}


void Gpio_Clear_PortB( void )
{
	GPIOB->BRR = 0xFFFF;
}

void Gpio_Set_PortB_Mask ( uint16_t Mask)
{
	GPIOB->BSRR = Mask;
}

void Gpio_PortB_Data (uint16_t Data)
{
	GPIOB->ODR = Data;
}

uint8_t Gpio_Read_Bit ( Gpio_Desc_t Gpio)
{
	uint8_t RetVal = 0;
	if ( Gpio < GPIO_TOTAL )
	{
		if ( ((Gpio_Config[Gpio].Port)->IDR & ( 1u<<(Gpio_Config[Gpio].Pin))) !=0 )
		{
			RetVal = 1;
		}
		else
		{
			RetVal = 0;
		}
	}
	return RetVal;
}

void Gpio_Get_Alt_PortPin(const Gpio_Desc_t Gpio,GPIO_TypeDef ** const Port,uint8_t * const Pin)
{
	if (Gpio < GPIO_TOTAL)
	{
		if (NULL != Port && NULL != Pin)
		{
			*Port = Gpio_Config[Gpio].Port;
			*Pin = Gpio_Config[Gpio].Pin;
		}
	}
}
