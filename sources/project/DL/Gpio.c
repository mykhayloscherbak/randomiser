/**
 * @file Gpio.c
 * @brief Contains init and access functions for Gpio
 * @author Mikl Scherbak
 * @em mikl74@yahoo.com
 * @date 18-07-2020
 * @version 1.1
 */

#include "stm32f1xx.h"
#include <stddef.h>
#include "Gpio.h"


typedef enum
{
	GPIO_MODE_IN =0,
	GPIO_MODE_OUT,
	GPIO_MODE_DO_NOT_TOUCH
} Gpio_Mode_t;

typedef void (*powerDown_t) (void);

#pragma pack(1)
/**
 * @brief Gpio pin configuration
 */
typedef struct
{
	GPIO_TypeDef *Port; /**< Pointer to port */
	uint8_t Pin; /**< PinNumber */
	Gpio_Mode_t Mode; /**< Mode (in,out, analog) */
	uint8_t InitialValue; /**< Initial value of OUT pin */
	powerDown_t powerDown; /**< Function that is called to set pin to power down state */
}Gpio_Config_t;

static void heartBeatPD(void);
static void nssPD(void);
static void lcdResetPD(void);
static void beeperPD(void);
static void dcPD(void);

static const Gpio_Config_t Gpio_Config[GPIO_TOTAL]=
{
		[GPIO_HEARTBEAT] = {.Port = GPIOC,.Pin = 13 ,.Mode = GPIO_MODE_OUT,0,heartBeatPD}, //GPIO_HEARTBEAT
		[GPIO_NSS] = {.Port = GPIOB,.Pin = 12,.Mode = GPIO_MODE_OUT,1,nssPD}, //GPIO_NSS
		[GPIO_BUTTON] = {.Port = GPIOB,.Pin = 1,.Mode = GPIO_MODE_IN,0,NULL},   //GPIO_BUTTON
		[GPIO_RESET] = {.Port = GPIOB,.Pin = 0, .Mode = GPIO_MODE_OUT, 0,lcdResetPD}, //GPIO_RESET
		[GPIO_BEEPER] = {.Port = GPIOB, .Pin = 11, .Mode = GPIO_MODE_OUT,0,beeperPD}, //GPIO_BEEPER
		[GPIO_SCK] = {.Port = GPIOB, .Pin = 13, .Mode = GPIO_MODE_DO_NOT_TOUCH, 0,  NULL},
		[GPIO_MOSI] = {.Port = GPIOB, .Pin = 15, .Mode = GPIO_MODE_DO_NOT_TOUCH, 0, NULL},
		[GPIO_DC]    = {.Port = GPIOB,.Pin = 14,  .Mode = GPIO_MODE_OUT, 0,dcPD}  //GPIO_DC
};


void Gpio_Init(void)
{
	Gpio_Desc_t Pin;
	RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_IOPBEN | RCC_APB2ENR_IOPCEN | RCC_APB2ENR_AFIOEN;
	AFIO->MAPR |= AFIO_MAPR_SWJ_CFG_1;
	AFIO->MAPR &= ~(AFIO_MAPR_SWJ_CFG_0 | AFIO_MAPR_SWJ_CFG_2);
	for ( Pin = 0; Pin < GPIO_TOTAL; Pin++)
	{
		if ( Gpio_Config[Pin].Pin < 8 )
		{
			if ( Gpio_Config[Pin].Mode == GPIO_MODE_IN)
			{
				(Gpio_Config[Pin].Port)->CRL &= ~( 0b11<< ( Gpio_Config[Pin].Pin * 4));
				(Gpio_Config[Pin].Port)->CRL &= ~( 0b01<< ( Gpio_Config[Pin].Pin * 4 + 2));
				(Gpio_Config[Pin].Port)->CRL |=  ( 0b10<< ( Gpio_Config[Pin].Pin * 4 + 2));
				(Gpio_Config[Pin].Port)->ODR |=  ( 1<< Gpio_Config[Pin].Pin ); /* Pull-Up */
			}
			else
			{
				if ( Gpio_Config[Pin].Mode == GPIO_MODE_OUT )
				{
					uint8_t Shift = Gpio_Config[Pin].Pin;
					if (Gpio_Config[Pin].InitialValue == 0)
					{
						Shift+=16; /* Select BS or BR bits */
					}
					(Gpio_Config[Pin].Port)->BSRR =  ( 1 << Shift );
					(Gpio_Config[Pin].Port)->CRL &= ~( 0b10<< ( Gpio_Config[Pin].Pin * 4));
					(Gpio_Config[Pin].Port)->CRL |=  ( 0b10<< ( Gpio_Config[Pin].Pin * 4));
					(Gpio_Config[Pin].Port)->CRL &= ~( 0b11<< ( Gpio_Config[Pin].Pin * 4 + 2));
				}
			}
		}
		else
		{
			if ( Gpio_Config[Pin].Mode == GPIO_MODE_IN)
			{
				(Gpio_Config[Pin].Port)->CRH &= ~( 0b11<< ( (Gpio_Config[Pin].Pin - 8) * 4));
				(Gpio_Config[Pin].Port)->CRH &= ~( 0b01<< ( (Gpio_Config[Pin].Pin - 8) * 4 + 2));
				(Gpio_Config[Pin].Port)->CRH |=  ( 0b10<< ( (Gpio_Config[Pin].Pin - 8) * 4 + 2));
				(Gpio_Config[Pin].Port)->ODR |=  ( 1<< Gpio_Config[Pin].Pin ); /* Pull-Up */
			}
			else
			{
				if ( Gpio_Config[Pin].Mode == GPIO_MODE_OUT )
				{
					uint8_t Shift = Gpio_Config[Pin].Pin;
					if (Gpio_Config[Pin].InitialValue == 0)
					{
						Shift+=16; /* Select BS or BR bits */
					}
					(Gpio_Config[Pin].Port)->BSRR =  ( 1 << Shift );
					(Gpio_Config[Pin].Port)->CRH &= ~( 0b10<< ( (Gpio_Config[Pin].Pin - 8) * 4));
					(Gpio_Config[Pin].Port)->CRH |=  ( 0b10<< ( (Gpio_Config[Pin].Pin - 8) * 4));
					(Gpio_Config[Pin].Port)->CRH &= ~( 0b11<< ( (Gpio_Config[Pin].Pin -8 ) * 4 + 2));
				}
			}
		}
	}
}

static void setPortPD(GPIO_TypeDef * const GPIO)
{
	for (uint8_t pin = 0; pin < 16; pin++)
	{
		for (Gpio_Desc_t Pin = 0; Pin < GPIO_TOTAL; Pin++)
		{
			if ((Gpio_Config[Pin].Pin != Pin) && (Gpio_Config[Pin].Port != GPIO))
			{
				if ( Gpio_Config[pin].Pin < 8 )
				{
					(Gpio_Config[pin].Port)->CRL &= ~( 0b11<< ( Gpio_Config[pin].Pin * 4));
					(Gpio_Config[pin].Port)->CRL &= ~( 0b01<< ( Gpio_Config[pin].Pin * 4 + 2));
					(Gpio_Config[pin].Port)->CRL |=  ( 0b10<< ( Gpio_Config[pin].Pin * 4 + 2));
					(Gpio_Config[pin].Port)->ODR |=  ( 1<< Gpio_Config[pin].Pin ); /* Pull-Up */
				}
				else
				{
					(Gpio_Config[pin].Port)->CRH &= ~( 0b11<< ( (Gpio_Config[pin].Pin - 8) * 4));
					(Gpio_Config[pin].Port)->CRH &= ~( 0b01<< ( (Gpio_Config[pin].Pin - 8) * 4 + 2));
					(Gpio_Config[pin].Port)->CRH |=  ( 0b10<< ( (Gpio_Config[pin].Pin - 8) * 4 + 2));
					(Gpio_Config[pin].Port)->ODR |=  ( 1<< Gpio_Config[pin].Pin ); /* Pull-Up */
				}
			}
		}
	}
}


void Gpio_DeInit(void)
{
	for (Gpio_Desc_t Pin = 0; Pin < GPIO_TOTAL; Pin++)
	{
		if (Gpio_Config[Pin].powerDown != NULL)
		{
			Gpio_Config[Pin].powerDown();
		}
	}
	setPortPD(GPIOA);
	setPortPD(GPIOB);
	setPortPD(GPIOC);

}

void Gpio_Set_Bit(Gpio_Desc_t Gpio)
{
	if ( Gpio < GPIO_TOTAL )
	{
		(Gpio_Config[Gpio].Port)->BSRR = 1<<Gpio_Config[Gpio].Pin;
	}
}

void Gpio_Clear_Bit(Gpio_Desc_t Gpio)
{
	if ( Gpio < GPIO_TOTAL )
	{
		(Gpio_Config[Gpio].Port)->BSRR = 1<<(Gpio_Config[Gpio].Pin + 16);
	}
}

uint8_t Gpio_Read_Bit ( Gpio_Desc_t Gpio)
{
	uint8_t RetVal = 0;
	if ( Gpio < GPIO_TOTAL )
	{
		if ( ((Gpio_Config[Gpio].Port)->IDR & ( 1<<(Gpio_Config[Gpio].Pin))) !=0 )
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

static void configPinIn(const Gpio_Desc_t pin, const uint8_t pullUp)
{
	if ( Gpio_Config[pin].Pin < 8 )
	{
		(Gpio_Config[pin].Port)->CRL &= ~( 0b11<< ( Gpio_Config[pin].Pin * 4));
		if (pullUp != 0)
		{
			(Gpio_Config[pin].Port)->CRL &= ~( 0b01<< ( Gpio_Config[pin].Pin * 4 + 2));
			(Gpio_Config[pin].Port)->CRL |=  ( 0b10<< ( Gpio_Config[pin].Pin * 4 + 2));
			(Gpio_Config[pin].Port)->ODR |=  ( 1<< Gpio_Config[pin].Pin ); /* Pull-Up */
		}
		else /* Floating in */
		{
			(Gpio_Config[pin].Port)->CRL &= ~( 0b10 << ( Gpio_Config[pin].Pin * 4 + 2));
			(Gpio_Config[pin].Port)->CRL |=  ( 0b01<< ( Gpio_Config[pin].Pin * 4 + 2));
			(Gpio_Config[pin].Port)->ODR &=  ~( 1<< Gpio_Config[pin].Pin ); /* Pull-Up */
		}
	}
	else
	{
		(Gpio_Config[pin].Port)->CRH &= ~( 0b11<< ( (Gpio_Config[pin].Pin - 8) * 4));
		if (pullUp != 0)
		{
			(Gpio_Config[pin].Port)->CRH &= ~( 0b01<< ( (Gpio_Config[pin].Pin - 8) * 4 + 2));
			(Gpio_Config[pin].Port)->CRH |=  ( 0b10<< ( (Gpio_Config[pin].Pin - 8) * 4 + 2));
			(Gpio_Config[pin].Port)->ODR |=  ( 1<< Gpio_Config[pin].Pin ); /* Pull-Up */
		}
		else
		{
			(Gpio_Config[pin].Port)->CRH &= ~( 0b10<< ( (Gpio_Config[pin].Pin - 8) * 4 + 2));
			(Gpio_Config[pin].Port)->CRH |=  ( 0b01<< ( (Gpio_Config[pin].Pin - 8) * 4 + 2));
			(Gpio_Config[pin].Port)->ODR &=  ~( 1<< Gpio_Config[pin].Pin ); /* Pull-Up */
		}
	}
}

static void heartBeatPD(void)
{
	configPinIn(GPIO_HEARTBEAT,1);
}

static void nssPD(void)
{
	configPinIn(GPIO_NSS,1);
}

static void lcdResetPD(void)
{
	configPinIn(GPIO_RESET,0);
}

static void beeperPD(void)
{
	configPinIn(GPIO_BEEPER,0);
}

static void dcPD(void)
{
	configPinIn(GPIO_DC,1);
}
