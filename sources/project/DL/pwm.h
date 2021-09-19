
#ifndef SOURCES_PROJECT_HAL_INCLUDE_PWM_H_
#define SOURCES_PROJECT_HAL_INCLUDE_PWM_H_

void PWM_Init(void);
void PWM_Deinit(void);
/**
 * @brief Sets backlight to corresponding value
 * @param Percent
 */
void Set_PWM(uint8_t Percent);


#endif /* SOURCES_PROJECT_HAL_INCLUDE_PWM_H_ */
