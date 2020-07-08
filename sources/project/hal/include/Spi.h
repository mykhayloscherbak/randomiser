/**
 * @file Spi.h
 * @brief Contains Spi driver prototyped
 * @author Mikl Scherbak
 * @em mikl74@yahoo.com
 * @date 26-04-2017
 *
 */
#ifndef SOURCE_DL_SPI_H_
#define SOURCE_DL_SPI_H_

#include <stdint.h>
#include "screen_updater.h"
#include "FreeRTOS.h"
#include "semphr.h"


/**
 * @brief Inits SPI1 bus
 */
void SPI_Init(Protected_FB_t * const FB);

/**
 * @brief Sends 16bit value to bus
 * @param Data Value
 */
void SPI_Send_One(const uint8_t Data);

void SPI_Transfer(void);
void SPI_lock(void);
void SPI_unlock(void);

#endif /* SOURCE_DL_SPI_H_ */
