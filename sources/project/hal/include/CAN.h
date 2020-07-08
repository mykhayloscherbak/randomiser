/**
 * @file CAN.h
 * @brief Contains CAN driver
 * @author Mikl Scherbak
 * @em mikl74@yahoo.com
 * @date 04-05-2016
 */



#ifndef SOURCE_DL_CAN_H_
#define SOURCE_DL_CAN_H_

#include <stdint.h>

typedef struct __attribute__((__packed__)) {
	uint16_t Id;
	uint8_t Dlc;
	uint8_t Data[8];
} Can_Message_t;

void Can_Init(void);
void Can_SendData(Can_Message_t *Msg);
/**
 * @brief Gets CAN data from fifo. Function gets only one message and must be called several times while fifo gets empty.
 * @param Fifo Fifo number (0,1)
 * @param Msg Can message. Data is left unchanged if no message in fifo.
 * @return Zero if no messages are in fifo and nonzero if message is got.
 */
uint8_t CAN_ReadData_Fifo(const uint8_t Fifo,Can_Message_t *Msg);

/**
 * @brief Sets filter array (16-bits is used).
 * @param pFilterArray Array of not-shifted can IDs
 * @param Count Number of CAN ID's
 */
void Can_AddFilter(uint16_t *pFilterArray,uint8_t Count);

#endif /* SOURCE_DL_CAN_H_ */
