/**
 * @brief contains low-level uart driver
 */

/* USART1 Pin RX = PA10, Pin TX = PA9 */

#include "stm32f1xx.h"
#include "Gpio.h"
#include "uart.h"
#include "project_config.h"
#include "FreeRTOS.h"
#include "semphr.h"

static Uart_Callbacks_t * pCallbacksData = NULL;
static void *pData = NULL;

static inline void Init_pins(void)
{
	GPIO_TypeDef *PortTX;
	GPIO_TypeDef *PortRX;
	uint8_t PinTX;
	uint8_t PinRX;
	Gpio_Get_Alt_PortPin(GPIO_GPS_TX,&PortTX,&PinTX);
	Gpio_Get_Alt_PortPin(GPIO_GPS_RX,&PortRX,&PinRX);
	if ( PinTX < 8 )
	{
		PortTX->CRL |=  (0b11u<< (PinTX * 4)); /* 0b11 = 50Mhz */
		PortTX->CRL |=  (0b10u<< (PinTX * 4 + 2)); /* 0b10 Af PP */
		PortTX->CRL &=  ~((0b01u<< (PinTX * 4 + 2))); /* 0b10 Af PP */
	}
	else
	{
		PinTX -= 8u;
		PortTX->CRH |=  (0b11u<< (PinTX * 4)); /* 0b11 = 50Mhz */
		PortTX->CRH |=  (0b10u<< (PinTX * 4 + 2)); /* 0b10 Af PP */
		PortTX->CRH &=  ~((0b01u<< (PinTX * 4 + 2))); /* 0b10 Af PP */

	}
	if ( PinRX < 8 )
	{
		PortRX->CRL &=  ~((0b11u<< (PinRX * 4))); /* 0b00 = In */
		PortRX->CRL |=  (0b01u<< (PinRX * 4 + 2)); /* 0b01 floating in */
		PortRX->CRL &=  ~((0b10u<< (PinRX * 4 + 2))); /* 0b01 floating in */
	}
	else
	{
		PinRX -= 8u;
		PortRX->CRH &=  ~((0b11u<< (PinRX * 4))); /* 0b00 = In */
		PortRX->CRH |=  (0b01u<< (PinRX * 4 + 2)); /* 0b01 floating in */
		PortRX->CRH &=  ~((0b10u<< (PinRX * 4 + 2))); /* 0b01 floating in */

	}
}

void Uart_Init(Uart_Callbacks_t * const callBacks, void * Data)
{
	Init_pins();
	RCC->APB2ENR |= RCC_APB2ENR_USART1EN;
	USART1->BRR = 7500; /* 72000000 / 115200 */
	USART1->CR1 = USART_CR1_UE|USART_CR1_RXNEIE|USART_CR1_TE|USART_CR1_RE;
	USART1->CR2 = 0;
	USART1->CR3 = 0;
	if (callBacks != NULL)
	{
		pCallbacksData = callBacks;
	}
	pData = Data;
	NVIC_SetPriority(USART1_IRQn,USART_INTERRUPT_PRIORITY - 1);
	NVIC_EnableIRQ(USART1_IRQn);
}

void Uart_EnableTX(void)
{
	USART1->CR1 |= USART_CR1_TXEIE;
}

void Uart_DisableTX(void)
{
	USART1->CR1 &= ~USART_CR1_TXEIE;
}
void USART1_IRQHandler(void);

void USART1_IRQHandler(void)
{
	uint32_t tmpSR = USART1->SR;
	if ( (USART1->CR1 & USART_CR1_TXEIE) != 0 && (tmpSR & USART_SR_TXE) != 0)
	{
		if (pCallbacksData != NULL)
		{
			uint8_t c;
			if ((*pCallbacksData->txCallback)(&c,pData) != 0)
			{
				USART1->DR = c;
			}
		}
	}
	if ((tmpSR & USART_SR_RXNE) != 0)
	{
		uint8_t d = USART1->DR;
		if ((tmpSR & USART_SR_FE) == 0)
		{
			if (pCallbacksData != NULL && pCallbacksData->rxCallback != NULL)
			{
				(*pCallbacksData->rxCallback)(d,pData);
			}
		}
	}
}
