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

#define ADC_AVG 10
#define MAX_DELTA_ALLOWED 30

void Adc_Init(void);
uint8_t Adc_Get_Pos(void);

#endif /* SOURCE_DL_ADC_H_ */
