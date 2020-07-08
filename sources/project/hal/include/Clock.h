/**
 * @file Clock.h
 * @brief Contains definitions for @ref Clock.c
 * @author Mikl Scherbak
 * @em mikl74@yahoo.com
 * @date 07-04-2016
 * @version 1.0
 */

#ifndef SOURCE_DL_CLOCK_H_
#define SOURCE_DL_CLOCK_H_

#define CPU_FREQ 72000000ul

/**
 * @brief Inits HSE as a main clock. PLL is setup to *9
 */
void Clock_HSE_Init(void);
/**
 * @brief Initialises TIM2 as a timebase for 10kHz PWM
 */
void Clock_Timebase_Init ( void );

/**
 * @brief Returns number of milliseconds from timebase start
 * @return milliscenonds
 */
uint32_t GetTicksCounter(void);

/**
 * @brief Initializes timer variable
 * @param pTimer pointer to the timer
 * @param Delta Number of milliseconds for timer to trigger
 */
void SetTimer(uint32_t *pTimer,uint32_t Delta);
/**
 * @brief Returns true if timer passed
 * @param Timer Timer variable
 * @return true if timer passed
 */
uint8_t IsTimerPassed(uint32_t Timer);

/**
 * @brief Sets new value for PWM in percent multiplied by 10
 * @param PWM10 new value.
 */
void Clock_Set_PWM10(uint16_t PWM10);

/**
 * Inits and starts tim2 timer which is used to for SQW pin capturing. Capturig is done by SW
 */
void Clock_Tim2_Init(void);

#endif /* SOURCE_DL_CLOCK_H_ */
