/**
 * @file Clock.c
 * @brief Contains clock init and functions
 * @author Mikl Scherbak (mikl74@yahoo.com)
 * @date 7-04-2016
 * @version 1.0
 */
#include "stm32f1xx.h"
#include "Clock.h"
#include "Gpio.h"


/**
 * @brief Inits HSE as a main clock. PLL is setup to *9
 */
void Clock_HSE_Init(void)
{
	RCC->CIR = 0x009F0000;
	FLASH->ACR |= FLASH_ACR_LATENCY_1;
	RCC->CR |= RCC_CR_HSION |RCC_CR_CSSON;
	while ( ! (RCC->CR & RCC_CR_HSIRDY) )
	{
	}
	RCC->CFGR &= ~RCC_CFGR_SW; /* USE HSI during setup */
	RCC->CFGR |=  RCC_CFGR_PLLSRC | RCC_CFGR_ADCPRE_DIV8 | RCC_CFGR_PPRE1_2 | RCC_CFGR_PLLMULL9;
	RCC->CR |= RCC_CR_HSEON;
	while ( ! (RCC->CR & RCC_CR_HSERDY) )
	{
	}

	RCC->CR |= RCC_CR_PLLON;
	while ( ! (RCC->CR & RCC_CR_PLLRDY) )
	{
	}
	RCC->CFGR &= ~RCC_CFGR_SW_0;
	RCC->CFGR |=  RCC_CFGR_SW_1;
	RCC->CR &= ~RCC_CR_HSION;
}

void Clock_Tim2_Init(void)
{
	RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
	TIM2->PSC = 7199; /*/7200 div and counter will count get 10000hz freq */
	TIM2->ARR = 65535; /* Count to MAX */
	TIM2->CR1 = 0; /* Upcount */
	TIM2->CR2 = 0;
	TIM2->SMCR = 0;
	TIM2->DIER = 0;
	TIM2->CR1 |= TIM_CR1_CEN;
}

