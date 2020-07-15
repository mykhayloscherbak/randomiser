/**
 * @file random.c
 */
#include "stm32f1xx.h"
#include "random.h"

void initRandom(void)
{
	DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}
uint16_t getRandom(const uint16_t max)
{
	return ((DWT->CYCCNT) >> 3) % max;
}
