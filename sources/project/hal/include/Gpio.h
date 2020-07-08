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
	GPIO_CAN_CTL,      /**< Can standby/on */
	GPIO_LCD_CS,	   /**< LCD chip select */
	GPIO_LCD_SCK,      /**< LCD clock */
	GPIO_LCD_RS,       /**< LCD Command/data */
	GPIO_LCD_DATA,     /**< LCD data (spi mosi) */
	GPIO_SPEAKER,      /**< Beeper */
	GPIO_GPS_TX,       /**< UART from CPU to GPS */
	GPIO_GPS_RX,       /**< UART from GPS to CPU */
	GPIO_SQW,          /**< Square wave from DS3221 */
	GPIO_S0,		   /**< Sensor 0 */
	GPIO_S1,		   /**< Sensor 1 */
	GPIO_S2,		   /**< Sensor 2 */
	GPIO_S3,		   /**< Sensor 3 */
	GPIO_B0,		   /**< Button 0 */
	GPIO_B1,		   /**< Button 1 */
	GPIO_B2,		   /**< Button 2 */
	GPIO_LCD_PWM,	   /**< Lcd pwm. It's not a gpio */
	GPIO_GPS_PWR,      /**< Gps module power control */
	GPIO_GPS_PPS,      /**< GPS module pps signal */
	GPIO_LCD_RESET,    /**< LCD reset pin */
	GPIO_I2C_SCL,      /**< I2C1 SCL for DS3231M */
	GPIO_I2C_SDA,	   /**< I2C1 SDA for DS3231M */
	GPIO_TOTAL         /**< Total number of GPIOs */
}Gpio_Desc_t;

void Gpio_Init(void);

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

void Gpio_Clear_PortB( void );
void Gpio_Set_PortB_Mask ( uint16_t Mask);
void Gpio_PortB_Data (uint16_t Data);
uint8_t Gpio_Read_Bit ( Gpio_Desc_t Gpio);
/**
 * @brief returns pin and port for selected AF GPIO
 * @param Gpio pin id
 * @param Port Pointer where port pointer will be placed. Must not be NULL
 * @param Pin Pointer where pin number will be placed. Must not be NULL
 */
void Gpio_Get_Alt_PortPin(const Gpio_Desc_t Gpio,GPIO_TypeDef ** const Port,uint8_t * const Pin);

#endif /* SOURCE_DL_GPIO_H_ */
