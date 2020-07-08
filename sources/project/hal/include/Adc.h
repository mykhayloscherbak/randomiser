/**
 * @file Adc.h
 * @brief Contains ADC driver form one channel prototypes
 * @author Mikl Scherbak
 * @em mikl74@yahoo.com
 * @date 02-05-2016
 *
 */

#ifndef SOURCE_DL_ADC_H_
#define SOURCE_DL_ADC_H_

#define ADC_AVG (10u)

void Adc_Init(void);
uint16_t GetAdc_Voltage(void);

#endif /* SOURCE_DL_ADC_H_ */
