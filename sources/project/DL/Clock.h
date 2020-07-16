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

#define CPU_FREQ 8000000ul

/**
 * @brief Inits HSE as a main clock. PLL is setup to *9
 */
void Clock_HSI_Init(void);
/**
 * @brief Initialises TIM2 as a timebase
 */
void Clock_Timebase_Init ( void );

/**
 * @brief De-initialises TIM2 before going to sleep mode
 */
void Clock_Timebase_deinit(void);


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

void DelayMsTimer(const uint32_t delay);
#endif /* SOURCE_DL_CLOCK_H_ */
