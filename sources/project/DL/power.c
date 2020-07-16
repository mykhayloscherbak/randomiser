#include "stm32f1xx.h"
#include "Gpio.h"
#include "Spi.h"
#include "Clock.h"
#include "power.h"

void sleep(void)
{
	Clock_Timebase_deinit();
	SPI_Deinit();
	Gpio_DeInit();
	EXTI->FTSR |= EXTI_FTSR_FT1;
	EXTI->EMR |= EXTI_EMR_EM1;
	AFIO->EXTICR[1] = AFIO_EXTICR1_EXTI1_PB;
	SCB->SCR |= SCB_SCR_SEVONPEND_Msk  | SCB_SCR_SLEEPDEEP_Msk ;
	SCB->SCR &= ~(SCB_SCR_SLEEPONEXIT_Msk);
	PWR->CR &= ~(PWR_CR_PDDS); /* Stop mode */
	PWR->CR |= PWR_CR_LPDS; /* Regulator off */
	PWR->CR |= PWR_CR_LPDS;
}

void wakeup(void)
{
	EXTI->FTSR &= ~(EXTI_FTSR_FT1);
	EXTI->EMR &= ~(EXTI_EMR_EM1);
	EXTI->PR &= ~(EXTI_PR_PIF1);
	NVIC_ClearPendingIRQ(EXTI1_IRQn);
}
