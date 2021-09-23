/**
 * @file BLL.h
 * @brief Contains business logic implementation
 * @author Mikl Scherbak
 * @em mikl74@yahoo.com
 * @date 18-04-2017
 *
 */

#ifndef SOURCE_BLL_BLL_H_
#define SOURCE_BLL_BLL_H_

#define DECIMAL_POINT_0 (1<<7)
#define DECIMAL_POINT_1 (1<<15)
#define DECIMAL_POINT_NONE 0
#define IDLE_TIMEOUT (5ul * 60ul * 100ul) /* 5 minutes */
#define NO_REPLY_TIMEOUT (20ul * 100ul) /* 20 seconds */
#define UNIT_ID_2DIGIT 1
#define BLINK_ON 2
#define BLINK_OFF 3

/**
 * @brief Initializes business logic structures
 */
void BLL_Init(void);
void BLL_Timer_OC_Hit(void);
void BLL_Timer_Overflow(void);
uint8_t MainLoop_Iteration(void);


#endif /* SOURCE_BLL_BLL_H_ */
