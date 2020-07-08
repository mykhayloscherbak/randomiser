/**
 * @file i2c_blocking.c
 * @author Mykhaylo Shcherbak
 * @date 8-Mar-2019
 * @e mikl74@yahoo.com
 * @version 1.00
 * Contains functions for i2c in blocking mode.
 */
#include <stdbool.h>
#include <stddef.h>
#include "stm32f1xx.h"
#include "i2c_blocking.h"
#include "Gpio.h"


static inline void Init_pins(void)
{
	GPIO_TypeDef *Port[2];
	uint8_t Pin[2];
	Gpio_Get_Alt_PortPin(GPIO_I2C_SCL,&Port[0],&Pin[0]);
	Gpio_Get_Alt_PortPin(GPIO_I2C_SDA,&Port[1],&Pin[1]);
	Gpio_Set_Bit(GPIO_I2C_SCL);
	Gpio_Set_Bit(GPIO_I2C_SDA);
	for (uint8_t i = 0; i < 2; i++)
	{
		if ( Pin[i] < 8 )
		{
			Port[i]->CRL |=  (0b11u<< (Pin[i] * 4)); /* 0b11 = 50Mhz */
			Port[i]->CRL |=  (0b11u<< (Pin[i] * 4 + 2)); /* Af open drain */
		}
		else
		{
			Pin[i] -= 8u;
			Port[i]->CRH |=  (0b11u<< (Pin[i] * 4)); /* 0b11 = 50Mhz */
			Port[i]->CRH |=  (0b11u<< (Pin[i] * 4 + 2)); /* Af open drain */
		}
	}
}

void I2c_blocking_init(void)
{
	/* PB8 = I2C1_SCL, PB9 = I2C1_SDA */
	Init_pins();
	AFIO->MAPR |= AFIO_MAPR_I2C1_REMAP; /* Move I2C1 to p8/p9 */
	RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;
	I2C1->CR1 &= ~(I2C_CR1_PE);
	I2C1->CR1 |= I2C_CR1_NOSTRETCH;
	I2C1->CR2 = 36; /* APB1 is 36Mhz, 27,7ns */
	I2C1->CCR = I2C_CCR_FS | I2C_CCR_DUTY | ( 4 << I2C_CCR_CCR_Pos); /* Fast, 9/16 duty, ccr = 4 */
	I2C1->TRISE = 10; /* Check by oscilloscope */

}

static void I2C_blocking_start(void)
{
	I2C1->CR1 |= I2C_CR1_START;
	while ((I2C1->SR1 & I2C_SR1_SB) == 0)
	{

	}
}

static bool I2C_blocking_send_addr(const uint8_t Addr)
{
	I2C1->DR = Addr;
	while ((I2C1->SR1 & I2C_SR1_ADDR) == 0)
	{

	}
	(void)I2C1->SR2;
	return (I2C1->SR1 & I2C_SR1_TXE) != 0;
}

static void I2C_blocking_stop(void)
{
	I2C1->CR1 |= I2C_CR1_STOP;
	while ((I2C1->SR2 & I2C_SR2_BUSY) != 0)
	{

	}
	I2C1->CR1 &= ~(I2C_CR1_PE);
}


bool I2c_blocking_write(const uint8_t Device_Addr,const uint8_t Reg_Addr,uint8_t * const Buf,const uint8_t Nbytes)
{
	I2C1->CR1 |= I2C_CR1_PE;
	I2C_blocking_start();
	const uint8_t Count = (Buf == NULL) ? 0 : Nbytes;
	bool ack = I2C_blocking_send_addr(Device_Addr);
	if (ack)
	{
		for (int8_t i = -1; i< (int8_t)Count; i++)
		{
			I2C1->DR = (i == -1) ? Reg_Addr: Buf[i];
			uint32_t sr1;
			do
			{
				sr1 = I2C1->SR1;
			} while ((sr1 & (I2C_SR1_TXE | I2C_SR1_AF)) == 0);
			if ((sr1 & I2C_SR1_AF) != 0)
			{
				ack = false;
				break;
			}

		}
	}
	if (Count > 0)
	{
		I2C_blocking_stop();
	}
	return ack;
}

bool I2C_Blocking_read(const uint8_t deviceAddr,const uint8_t regAddr,uint8_t * const data,const uint8_t length)
{
	bool ack = I2c_blocking_write(deviceAddr, regAddr, NULL, 0);
	if (ack)
	{
		I2C_blocking_start(); /* Repeated start */
		ack = I2C_blocking_send_addr(deviceAddr | 1);
		I2C1->CR1 |= I2C_CR1_ACK;
		for (uint8_t i = 0; i < length; i++)
		{
			if (length - i == 1)
			{
				I2C1->CR1 |= I2C_CR1_STOP;
				I2C1->CR1 &= ~(I2C_CR1_ACK);
			}
			while ((I2C1->SR1 & I2C_SR1_RXNE) == 0)
			{

			}
			data[i] = I2C1->DR;


		}
	}
	return ack;
}
