/**
 * @file Gpio.h
 * @brief Contains definitions for Gpio.c
 * @date 12-04-2016
 * @author Mikl Scherbak
 * @em mikl74@yahoo.com
 * @version 1.0
 */

#ifndef SOURCE_DL_GPIO_H_
#define SOURCE_DL_GPIO_H_
#include "stm32f1xx.h"
/**
 * @brief Descriptions of GPIO pins
 */
typedef enum
{
	GPIO_HEARTBEAT = 0,/**< HEARTBEAT led */
//	GPIO_NSS,		   /**< SPI not slave select */
//	GPIO_BUTTON,       /**< Button in */
	GPIO_RESET,		   /**< Reset for LCD */
//	GPIO_DC,		   /**< D/Command */
//	GPIO_BEEPER,       /**< Shit beeper */
	GPIO_SCL,          /**< LCD SCL */
	GPIO_SDA,         /**< LCD SDA */
	GPIO_TOTAL         /**< Total number of GPIOs */
}Gpio_Desc_t;

void Gpio_Init(void);
void Gpio_DeInit(void);

/**
 * @brief Sets the corresponding GPIO pin
 * @param Gpio pin
 */
void Gpio_Set_Bit(Gpio_Desc_t Gpio);
/**
 * @brief Reads the corresponding GPIO pin
 * @param Gpio pin
 */
void Gpio_Clear_Bit(Gpio_Desc_t Gpio);
uint8_t Gpio_Read_Bit ( Gpio_Desc_t Gpio);
uint8_t Gpio_Get_Pin(const Gpio_Desc_t gpio);
GPIO_TypeDef  * Gpio_Get_Port(const Gpio_Desc_t gpio);

#endif /* SOURCE_DL_GPIO_H_ */
