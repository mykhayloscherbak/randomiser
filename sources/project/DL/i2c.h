/*
 * i2c.h
 *
 *  Created on: Jul 23, 2020
 *      Author: mikl
 */

#ifndef SOURCES_PROJECT_DL_I2C_H_
#define SOURCES_PROJECT_DL_I2C_H_


void i2cInit(void);
uint8_t i2cSend(const uint8_t addr,uint8_t * const buf,const uint32_t count);
void waitTransfer(void);


#endif /* SOURCES_PROJECT_DL_I2C_H_ */
