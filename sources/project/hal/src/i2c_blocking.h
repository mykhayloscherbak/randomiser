/**
 * @file i2c_blocking.h
 * @author Mykhaylo Shcherbak
 * @date 8-Mar-2019
 * @e mikl74@yahoo.com
 * @version 1.00
 * Contains functions prototypes for i2c in blocking mode.
 */

#ifndef SOURCES_PROJECT_HAL_SRC_I2C_BLOCKING_H_
#define SOURCES_PROJECT_HAL_SRC_I2C_BLOCKING_H_

#include <stdbool.h>

void I2c_blocking_init(void);
/**
 * @brief Sends data to i2c device. If no data (NULL or @Nbytes == 0) then the "stop" state is not generated.
 *
 * @param Device_Addr device address on the bus
 * @param Reg_Addr register address
 * @param Buf Data ptr. NO data if NULL
 * @param Nbytes counter of data bytes. NO data if 0
 * @return true if sending ok or false if something is not acknowledged
 */
bool I2c_blocking_write(const uint8_t Device_Addr,const uint8_t Reg_Addr,uint8_t * const Buf,const uint8_t Nbytes);
/**
 * @brief Receives data from @regAddr if device @deviceAddr
 * @param deviceAddr I2c device address without R/W bit set
 * @param regAddr adress of the register to start with
 * @param length length of data to be read
 * @param data place to put data
 * @return true if device was acknowledged
 */
bool I2C_Blocking_read(const uint8_t deviceAddr,const uint8_t regAddr,uint8_t * const data,const uint8_t length);



#endif /* SOURCES_PROJECT_HAL_SRC_I2C_BLOCKING_H_ */
