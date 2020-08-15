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

/**
 * @brief Inits SPI2 bus
 */
void SPI_Init(void);
void SPI_Deinit(void);
/**
 * @brief Sends 16bit value to bus
 * @param Data Value
 */
void SPI_Send_One(const uint8_t Data);

void WaitTransfer(void);
void SPI_Transfer(uint8_t * const FrameBuf, uint16_t const size);

#endif /* SOURCE_DL_SPI_H_ */
