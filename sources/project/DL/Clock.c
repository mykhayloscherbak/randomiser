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
#include "../Bll/BLL.h"

static volatile uint32_t TicksCounter = 0;

void TIM2_IRQHandler(void);

/**
 * @brief Inits HSE as a main clock. PLL is setup to *9
 */
void Clock_HSI_Init(void)
{
	FLASH->ACR |= FLASH_ACR_LATENCY_1;
	RCC->CR |= RCC_CR_HSION|RCC_CR_CSSON;
	while ( ! (RCC->CR & RCC_CR_HSIRDY) )
	{
	}
	RCC->CFGR &= ~RCC_CFGR_SW; /* USE HSI during setup */
	RCC->CFGR |= /* for HSE: RCC_CFGR_PLLSRC | */ RCC_CFGR_ADCPRE_DIV8 | RCC_CFGR_PPRE1_2 | RCC_CFGR_PLLMULL9;
//	RCC->CR |= RCC_CR_HSEON;
//	while ( ! (RCC->CR & RCC_CR_HSERDY) )
//	{
//	}
	RCC->CR |= RCC_CR_PLLON;
	while ( ! (RCC->CR & RCC_CR_PLLRDY) )
	{
	}
	RCC->CFGR &= ~RCC_CFGR_SW_0;
	RCC->CFGR |=  RCC_CFGR_SW_1;
//	RCC->CR &= ~RCC_CR_HSION;
}

/* Tim2 interrupts must be at 10Khz */
void Clock_Timebase_Init ( void )
{
	DBGMCU->CR|=DBGMCU_CR_DBG_TIM2_STOP;
	RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
	TIM2->PSC = 39; /*/80 div and counter will count to 99 */
	TIM2->ARR = 9999; /*/100 div */
	TIM2->CR1 = TIM_CR1_URS;
	TIM2->CR2 = 0;
	TIM2->SMCR = 0;
	TIM2->DIER = TIM_DIER_UIE;
	TIM2->CCR1 = 0;
	TIM2->CR1 |= TIM_CR1_CEN;
	NVIC_EnableIRQ( TIM2_IRQn );
}

void TIM2_IRQHandler(void)
{
	if ((TIM2->SR & TIM_SR_UIF) !=0)
	{
		TIM2->SR &= ~(TIM_SR_UIF);
		TicksCounter++;
	}
}

uint32_t GetTicksCounter(void)
{
	return TicksCounter;
}

void SetTimer(uint32_t *pTimer,uint32_t Delta)
{
	*pTimer=Delta + TicksCounter;
}

uint8_t IsTimerPassed(uint32_t Timer)
{
	return TicksCounter >= Timer;
}

void DelayMsTimer(const uint32_t delay)
{
	uint32_t timer;
	SetTimer(&timer,delay);
	while (IsTimerPassed(timer) == 0)
	{

	}
}
