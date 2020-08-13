/**
 * @file i2c.c
 * @author Mykhaylo Shcherbak
 * @date 19-07-2020
 * @em mikl74@yahoo.com
 */

/* USE DMA1.6 */
#include "stm32f1xx.h"
#include "Gpio.h"
#include "i2c.h"
#include "Clock.h"

static volatile uint8_t TransferComplete = 0;
static void gpioInit(const Gpio_Desc_t i2cPin)
{
	uint8_t pin = Gpio_Get_Pin(i2cPin);
	GPIO_TypeDef * const port = Gpio_Get_Port(i2cPin);
	port->BSRR = 1 << pin;
	if ( pin < 8 ) /* AF OD */
	{
		port->CRL &= ~( 0b1111<< ( pin * 4 ) );
		port->CRL |= ( 0b1110 << ( pin * 4 ) );
	}
	else
	{
		pin -= 8;
		port->CRH &= ~( 0b1111<< ( pin * 4 ) );
		port->CRH |= ( 0b1110 << ( pin * 4 ) );
	}
}

#if 0
static void i2cScanBus(void)
{
	I2C1->CR1 |= I2C_CR1_PE;
	volatile uint8_t buf[127] = {[0 ... 126] = 0};
	uint8_t ptr = 0;
	for (uint8_t addr = 0; addr < 254; addr+=2)
	{
		I2C1->CR1 |= I2C_CR1_START;
		while ((I2C1->SR1 & I2C_SR1_SB) == 0)
		{
			__asm volatile("nop");
		}
		I2C1->DR = addr;
		while ((I2C1->SR2 & I2C_SR2_BUSY) == 0)
		{
			__asm volatile("nop");
		}
		if (I2C1->SR1 & I2C_SR1_ADDR)
		{
			buf[ptr++] = addr;
		}
		I2C1->CR1 |= I2C_CR1_STOP;
		while ((I2C1->SR2 & I2C_SR2_BUSY) != 0)
		{

		}
	}
	__asm volatile ("bkpt 1");
	(void)buf;
}
#endif

void i2cInit(void)
{
	RCC->AHBENR |= RCC_AHBENR_DMA1EN;
	RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;
	RCC->APB2ENR |= RCC_APB2ENR_AFIOEN;
	AFIO->MAPR |= AFIO_MAPR_I2C1_REMAP;
	gpioInit(GPIO_SCL);
	gpioInit(GPIO_SDA);
	I2C1->CR2 = 0x28; /* 40Mhz */
	I2C1->CCR = 2 | I2C_CCR_DUTY | I2C_CCR_FS; /* 5uS, 400kHz, 9/16 DC */
	I2C1->TRISE = 5; /* 300nS */

 //   i2cScanBus();
}

uint8_t i2cSend(const uint8_t addr,uint8_t * const buf,const uint32_t count)
{
	uint8_t retval = 0;
	I2C1->CR1 |= I2C_CR1_PE| I2C_CR1_NOSTRETCH;
	I2C1->CR1 |= I2C_CR1_START;
	while ((I2C1->SR1 & I2C_SR1_SB) == 0)
	{
	}
	I2C1->DR = addr;
	DelayMsTimer(2);
	if ((I2C1->SR1 & I2C_SR1_ADDR) != 0)
	{
		I2C1->SR2; /* Clean addr bit */
		I2C1->CR2 |= I2C_CR2_DMAEN;
		DMA1_Channel6->CCR = DMA_CCR_MINC |   /* Memory increment */
							 DMA_CCR_TCIE| /* Transfer complete interrupt enable */
							 DMA_CCR_DIR;  /* Direction from memory to peDelayMsTimer(1);	ripheral */
		TransferComplete = 0;
		DMA1_Channel6->CNDTR = count;
		DMA1_Channel6->CPAR = (uint32_t)(&I2C1->DR);
		DMA1_Channel6->CMAR = (uint32_t)buf;
		NVIC_EnableIRQ(DMA1_Channel6_IRQn);
		DMA1_Channel6->CCR |= DMA_CCR_EN;
		retval = 1;

	}
	else
	{
		I2C1->CR1 &= ~I2C_CR1_PE;
		TransferComplete = 1;
	}
	return retval;
}

void waitTransfer(void)
{
	while (TransferComplete == 0)
	{

	}
}

void DMA1_Channel6_IRQHandler(void);
void DMA1_Channel6_IRQHandler(void)
{
	I2C1->CR2 &= ~I2C_CR2_DMAEN;
	NVIC_EnableIRQ(I2C1_EV_IRQn);
	I2C1->CR2 |= I2C_CR2_ITEVTEN;
	DMA1->IFCR = DMA_IFCR_CGIF6;
	NVIC_DisableIRQ(DMA1_Channel6_IRQn);
}

void I2C1_EV_IRQHandler(void);
void I2C1_EV_IRQHandler(void)
{
	if ((I2C1->SR1 & I2C_SR1_BTF) != 0)
	{
		TransferComplete = 1;
		I2C1->CR2 &= ~I2C_CR2_ITEVTEN;
		I2C1->CR1 |= I2C_CR1_STOP;
		I2C1->CR1 &= ~I2C_CR1_PE;
		NVIC_DisableIRQ(I2C1_EV_IRQn);
	}
}
