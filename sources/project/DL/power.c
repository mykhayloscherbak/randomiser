#include "stm32f1xx.h"
#include "Gpio.h"
#include "Spi.h"
#include "Clock.h"
#include "power.h"

static void sleep(void)
{
	Clock_Timebase_deinit();
	SPI_Deinit();
	Gpio_DeInit();
	EXTI->FTSR |= EXTI_FTSR_FT1;
	EXTI->EMR |= EXTI_EMR_EM1;
	AFIO->EXTICR[0] = AFIO_EXTICR1_EXTI1_PB;
	SCB->SCR |= SCB_SCR_SLEEPDEEP | SCB_SCR_SEVONPEND;
	PWR->CR &= ~PWR_CR_PDDS; /* Stop mode */
	PWR->CR &= ~PWR_CR_LPDS;  /* Regulator on */
}

static void wakeup(void)
{
	EXTI->FTSR &= ~(EXTI_FTSR_FT1);
	EXTI->EMR &= ~(EXTI_EMR_EM1);
	EXTI->PR &= ~(EXTI_PR_PIF1);
	Clock_HSI_Init();
	Clock_Timebase_Init();
	Gpio_Init();
	SPI_Init();
}

void powerSave(void)
{
	sleep();
	__DSB();
	__ISB();
	__WFE();
	__WFE();
	__asm volatile ("nop"); /* Workaround for errata. Wakeup in debug mode can skip instruction */
	wakeup();
}
