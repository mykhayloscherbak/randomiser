/**
 * @file Spi.c
 * @brief Contains Spi driver
 * @author Mikl Scherbak
 * @em mikl74@yahoo.com
 * @date 16-01-2019
 *
 */
#include <stm32f1xx.h>
#include <stddef.h>
#include "stdbool.h"
#include "Spi.h"
#include "Gpio.h"
#include "project_config.h"
#include "FreeRTOS.h"
#include "semphr.h"

static Protected_FB_t *FB = NULL;
static SemaphoreHandle_t SPI_semaphore;

static void DMA_Spi_Init(void)
{
	RCC->AHBENR |= RCC_AHBENR_DMA1EN;
	DMA1_Channel3->CCR = DMA_CCR_MINC |   /* Memory increment */
						 DMA_CCR_TCIE| /* Transfer complete interrupt enable */
						 DMA_CCR_DIR;  /* Direction from memory to peripheral */
}

static inline void Init_pin(void)
{
	GPIO_TypeDef *Port[2];
	uint8_t Pin[2];
	Gpio_Get_Alt_PortPin(GPIO_LCD_SCK,&Port[0],&Pin[0]);
	Gpio_Get_Alt_PortPin(GPIO_LCD_DATA,&Port[1],&Pin[1]);
	for (uint8_t i = 0; i < 2; i++)
	if ( Pin[i] < 8 )
	{
		Port[i]->CRL |=   (0b11u<< (Pin[i] * 4)); /* 0b11 = 50Mhz */
		Port[i]->CRL &= ~( 0b01u<< (Pin[i] * 4 + 2)); /* Af push-pull */
		Port[i]->CRL |=  ( 0b10u<< (Pin[i] * 4 + 2));
	}
	else
	{
		Pin[i] -= 8u;
		Port[i]->CRH |=   (0b11u<< (Pin[i] * 4)); /* 0b11 = 50Mhz */
		Port[i]->CRH &= ~( 0b01u<< (Pin[i] * 4 + 2)); /* Af push-pull */
		Port[i]->CRH |=  ( 0b10u<< (Pin[i] * 4 + 2));
	}
}


void SPI_Init(Protected_FB_t * const Protected_FB)
{
	static StaticSemaphore_t SPI_semaphore_buf;
	SPI_semaphore = xSemaphoreCreateBinaryStatic(&SPI_semaphore_buf);
	if (Protected_FB != NULL && Protected_FB->size > 0)
	{
		FB = Protected_FB;
		RCC->APB2ENR |= RCC_APB2ENR_SPI1EN | RCC_APB2ENR_AFIOEN;
		/* PA5(CLK) and PA7 (MOSI) are af PP */
		Init_pin();
		/* Prescaler = 16, Bidi out, 16bit, SW nss control, idle low, 1 edge, master */
		SPI1->CR1 = SPI_CR1_BIDIMODE | SPI_CR1_BIDIOE | /* SPI_CR1_DFF | */ SPI_CR1_BR_1 | /* SPI_CR1_BR_0 | */ SPI_CR1_MSTR | SPI_CR1_SSM | SPI_CR1_SSI;
		SPI1->CR2 = 0; // SPI_CR2_SSOE;
		SPI1->CR1 |= SPI_CR1_SPE ;
		DMA_Spi_Init();
		NVIC_SetPriority(DMA1_Channel3_IRQn,LOW_SYSTEM_INTERRUPT_PRIORITY);
		NVIC_SetPriority(SPI1_IRQn,LOW_SYSTEM_INTERRUPT_PRIORITY);
		NVIC_EnableIRQ(DMA1_Channel3_IRQn);
		NVIC_EnableIRQ(SPI1_IRQn);
	}
	SPI_unlock();
}

void DMA1_Channel3_IRQHandler(void);
void DMA1_Channel3_IRQHandler(void)
{
	SPI1->CR2 &= ~(SPI_CR2_TXDMAEN);
	DMA1->IFCR |= DMA_IFCR_CTCIF3;
	DMA1_Channel3->CCR &= ~(DMA_CCR_EN);
	SPI1->CR2 |= SPI_CR2_TXEIE; /* Allow interrupt for TXIE which will wait for busy flag and set CS */
}

void SPI1_IRQHandler(void);
void SPI1_IRQHandler(void)
{
	while ((SPI1->SR & SPI_SR_BSY) !=0 )
	{

	}
	Gpio_Set_Bit(GPIO_LCD_CS);
	SPI1->CR2 &= ~(SPI_CR2_TXEIE);
	FB->isChanged = false;
	xSemaphoreGiveFromISR(FB->semaphore,NULL); /* unlock buf */
	xSemaphoreGiveFromISR(SPI_semaphore,NULL); /* Inform task about transaction complete */
}


void SPI_Transfer(void)
{
	if (FB != NULL && FB->size >0)
	{
		xSemaphoreTake(FB->semaphore,portMAX_DELAY);
		if (!FB->isChanged)
		{
			xSemaphoreGive(FB->semaphore);
		}
		else
		{
			SPI_lock();
			Gpio_Set_Bit(GPIO_LCD_RS);
			Gpio_Clear_Bit(GPIO_LCD_CS);
			DMA1_Channel3->CNDTR = FB->size;
			DMA1_Channel3->CPAR = (uint32_t)(&SPI1->DR);
			DMA1_Channel3->CMAR = (uint32_t)FB->Buf;
			SPI1->CR2 |= SPI_CR2_TXDMAEN;
			DMA1_Channel3->CCR |= DMA_CCR_EN;
		}
	}
}

void SPI_Send_One(const uint8_t Data)
{
	while ((SPI1->SR & SPI_SR_BSY) !=0 )
	{

	}
	SPI1->DR = Data;
	while ((SPI1->SR & SPI_SR_TXE) ==0 )
	{

	}
	while ((SPI1->SR & SPI_SR_BSY) !=0 )
	{

	}
}

void SPI_lock(void)
{
	xSemaphoreTake(SPI_semaphore,portMAX_DELAY);
}

void SPI_unlock(void)
{
	xSemaphoreGive(SPI_semaphore);
}
