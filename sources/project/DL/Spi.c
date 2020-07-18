/**
 * @file Spi.c
 * @brief Contains Spi driver
 * @author Mikl Scherbak
 * @em mikl74@yahoo.com
 * @date 26-04-2017
 *
 */
#include "uc1701x.h"
#include "Spi.h"
#include "Gpio.h"
#include <stm32f1xx.h>

static volatile uint8_t TransferComplete = 0xff;

static void DMA_Spi_Init(void)
{
	RCC->AHBENR |= RCC_AHBENR_DMA1EN;
	DMA1_Channel5->CCR = DMA_CCR_MINC |   /* Memory increment */
						 DMA_CCR_TCIE| /* Transfer complete interrupt enable */
						 DMA_CCR_DIR;  /* Direction from memory to peripheral */
}

void SPI_Init(void)
{
	RCC->APB1ENR |= RCC_APB1ENR_SPI2EN;
	RCC->APB2ENR |= RCC_APB2ENR_IOPBEN | RCC_APB2ENR_AFIOEN;
	/* PB13(CLK) and PB15 (MOSI) are af PP */
	GPIOB->CRH |=  GPIO_CRH_CNF13_1 | GPIO_CRH_CNF15_1 | GPIO_CRH_MODE13 | GPIO_CRH_MODE15;
	GPIOB->CRH &= ~(GPIO_CRH_CNF13_0 | GPIO_CRH_CNF15_0);
	/* Prescaler = 8, Bidi out, 8bit, SW nss control, idle low, 1 edge, master */
	SPI2->CR1 = SPI_CR1_BIDIMODE | SPI_CR1_BIDIOE | /* SPI_CR1_DFF | */ SPI_CR1_BR_1 | /* SPI_CR1_BR_0 | */ SPI_CR1_MSTR | SPI_CR1_SSM | SPI_CR1_SSI;
	SPI2->CR2 = 0; // SPI_CR2_SSOE;
	SPI2->CR1 |= SPI_CR1_SPE ;
	DMA_Spi_Init();
	NVIC_EnableIRQ(DMA1_Channel5_IRQn);
	NVIC_EnableIRQ(SPI2_IRQn);
}

void SPI_Deinit(void)
{ //TODO: Change this to GPIO_Desc_t
	GPIOB->CRH |=  GPIO_CRH_CNF13_1 | GPIO_CRH_CNF15_1;
    GPIOB->CRH &= ~(GPIO_CRH_CNF13_0 | GPIO_CRH_CNF15_0 | GPIO_CRH_MODE13 | GPIO_CRH_MODE15);
    GPIOB->ODR |= (1<<13) | (1<<15);
    RCC->APB1ENR &= ~(RCC_APB1ENR_SPI2EN);
    RCC->AHBENR &= ~(RCC_AHBENR_DMA1EN);
    NVIC_DisableIRQ(DMA1_Channel5_IRQn);
    NVIC_DisableIRQ(SPI2_IRQn);
}

void DMA1_Channel5_IRQHandler(void);
void DMA1_Channel5_IRQHandler(void)
{
	SPI2->CR2 &= ~(SPI_CR2_TXDMAEN);
	DMA1->IFCR |= DMA_IFCR_CTCIF5;
	DMA1_Channel5->CCR &= ~(DMA_CCR_EN);
	SPI2->CR2 |= SPI_CR2_TXEIE; /* Allow interrupt for TXIE which will wait for busy flag and set CS */
}

void SPI2_IRQHandler(void);
void SPI2_IRQHandler(void)
{
	while ((SPI2->SR & SPI_SR_BSY) !=0 )
	{

	}
	Gpio_Set_Bit(GPIO_NSS);
	TransferComplete = 1;
	SPI2->CR2 &= ~(SPI_CR2_TXEIE);

}


void SPI_Transfer(uint8_t wait)
{
	if (wait != 0)
	{
		while (TransferComplete == 0)
		{
		}
	}
	Gpio_Set_Bit(GPIO_DC);
	Gpio_Clear_Bit(GPIO_NSS);
	TransferComplete = 0;
	DMA1_Channel5->CNDTR = FRAME_BUF_SIZE;
	DMA1_Channel5->CPAR = (uint32_t)(&SPI2->DR);
	DMA1_Channel5->CMAR = (uint32_t)&FrameBuf[0];
	SPI2->CR2 |= SPI_CR2_TXDMAEN;
	DMA1_Channel5->CCR |= DMA_CCR_EN;
}

void SPI_Send_One(const uint8_t Data)
{
	while ((SPI2->SR & SPI_SR_BSY) !=0 )
	{

	}
	SPI2->DR = Data;
	while ((SPI2->SR & SPI_SR_TXE) ==0 )
	{

	}
	while ((SPI2->SR & SPI_SR_BSY) !=0 )
	{

	}
}
