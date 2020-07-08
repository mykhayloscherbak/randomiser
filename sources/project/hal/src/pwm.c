
/* LCD PWM is PB6 = Tim4,ch1 */

#include <stdint.h>
#include "stm32f1xx.h"
#include "Gpio.h"
#include "pwm.h"

/**
 * @brief inits pwm pin as AF push-pull
 */
static inline void Init_pin(void)
{
	GPIO_TypeDef *Port;
	uint8_t Pin;
	Gpio_Get_Alt_PortPin(GPIO_LCD_PWM,&Port,&Pin);
	if ( Pin < 8 )
	{
			Port->CRL &= ~(0b01u<< (Pin * 4)); /* Mode = 0b10 = out 2Mhz */
			Port->CRL |=   (0b10u<< (Pin * 4));
			Port->CRL &= ~( 0b01u<< (Pin * 4 + 2)); /* Af push-pull */
			Port->CRL |=  ( 0b10u<< (Pin * 4 + 2));
	}
	else
	{
				Pin -= 8u;
		        Port->CRH &= ~(0b01u<<  (Pin * 4)); /* Mode = 0b10 = out 2Mhz */
				Port->CRH |=   (0b10u<< (Pin * 4));
				Port->CRH &= ~( 0b01u<< (Pin * 4 + 2)); /* Af push-pull */
				Port->CRH |=  ( 0b10u<< (Pin * 4 + 2));
	}
}

void Init_PWM(void)
{ /* Timer 4, ch1, PWM */
	Init_pin();
	RCC->APB1ENR |= RCC_APB1ENR_TIM4EN;
	TIM4->PSC = 719; /*/720 div and counter will count to 100 */
	TIM4->ARR = 100; /*/100 div */
	TIM4->CR1 = TIM_CR1_ARPE | TIM_CR1_CMS_0; /* Auto-reload and center-aligned 1 */
	TIM4->CR2 = 0;
	TIM4->SMCR = 0;
	TIM4->DIER = 0;
	TIM4->CCMR1 = TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1PE;
	TIM4->CCER |= TIM_CCER_CC1E | TIM_CCER_CC1P;
	TIM4->CCR1 = 0;
	TIM4->CR1 |= TIM_CR1_CEN;
}

void Set_PWM(uint8_t Percent)
{
	if (Percent > 100)
	{
		Percent = 100;
	}
	TIM4->CCR1 = Percent;
}
