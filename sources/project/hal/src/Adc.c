/**
 * @file Adc.c
 * @brief Contains ADC driver form one channel
 * @author Mikl Scherbak
 * @em mikl74@yahoo.com
 * @date 02-05-2016
 *
 */

#include <stm32f1xx.h>
#include "Adc.h"

static volatile uint16_t Adc_Buf[ADC_AVG];

void Adc_Init( void )
{
	RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;
	RCC->AHBENR  |= RCC_AHBENR_DMA1EN;

	ADC1->CR2|=ADC_CR2_ADON; /* From power down mode */

	ADC1->CR2|=ADC_CR2_RSTCAL; /* Reset calibration */
	while (ADC1->CR2&ADC_CR2_RSTCAL)
	{

	}
	ADC1->CR2|=ADC_CR2_CAL; /* Calibrate */
	while (ADC1->CR2&ADC_CR2_CAL)
	{

	}

	/* Channels 0  sample rate is 239.5 cycles */
	ADC1->SMPR2=ADC_SMPR2_SMP0_0|ADC_SMPR2_SMP0_1|ADC_SMPR2_SMP0_2;

	/*  Sequence is 10-11-12-13-4-1-0 (RM p.238) */

	ADC1->SQR3=0;
	ADC1->SQR1=0; /* 1 Channel */

	ADC1->CR2|=ADC_CR2_CONT|ADC_CR2_DMA;

	ADC1->CR1|=ADC_CR1_SCAN;

	/* DMA Initialization */
	DMA1_Channel1->CPAR=(uint32_t)(&(ADC1->DR));
	DMA1_Channel1->CMAR=(uint32_t)Adc_Buf;
	DMA1_Channel1->CNDTR=sizeof(Adc_Buf)/sizeof(uint16_t);
	/* RG p. 277 */
	DMA1_Channel1->CCR|=DMA_CCR_MINC| /* Memory increment */
						DMA_CCR_MSIZE_0| /* Memory and per. size = 16 bit */
						DMA_CCR_PSIZE_0|
						DMA_CCR_CIRC;	 /* Circular */

	DMA1_Channel1->CCR|=DMA_CCR_EN;
	ADC1->CR2|=ADC_CR2_ADON;

}

static uint16_t GetAdcRaw(void)
{
	uint32_t RetVal = 0;

	for (uint8_t i = 0; i < ADC_AVG; i++ )
	{
		RetVal +=Adc_Buf[i];
	}
	return (uint16_t)(RetVal / ADC_AVG);
}

uint16_t GetAdc_Voltage(void)
{
	uint16_t Raw = GetAdcRaw();
	uint32_t MilliVolts_In = Raw * 3300ul/4095ul;
	return (uint16_t)(MilliVolts_In * 6 + 800u); /* 1/6 divider, .8 volts are dropped on diode */
}
