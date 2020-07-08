/**
 * @file CAN.c
 * @brief Contains CAN driver
 * @author Mikl Scherbak
 * @em mikl74@yahoo.com
 * @date 02-05-2016
 *
 */

#include <stm32f1xx.h>
#include "CAN.h"

void Can_Init(void)
{
	RCC->APB1ENR |= RCC_APB1ENR_CAN1EN;
	RCC->APB2ENR |= RCC_APB2ENR_AFIOEN;

	/* Set GPIO = PA11 CAN_RX Input Pull-Up
	 * PA12=CAN_TX = AF Push-Pull
	 */
	GPIOA->CRH |= GPIO_CRH_CNF11_1 | GPIO_CRH_CNF12_1 | GPIO_CRH_MODE12;
	GPIOA->CRH &= ~(GPIO_CRH_CNF11_0 | GPIO_CRH_CNF12_0 | GPIO_CRH_MODE11);

	CAN1->MCR |=  CAN_MCR_ABOM;
	CAN1->MCR &= ~(CAN_MCR_DBF);
	CAN1->MCR &= ~(CAN_MCR_SLEEP);
	CAN1->MCR |= CAN_MCR_INRQ;

	while ( (CAN1->MSR & CAN_MSR_INAK) == 0)
	{
		//TODO: Add check for timeout
	}
	CAN1->MSR = CAN_MCR_ABOM | CAN_MCR_AWUM | CAN_MCR_RFLM | CAN_MCR_TXFP | CAN_MCR_INRQ;
	/* Prescaller = 18, BS1=10 bs2 = 4 */
	CAN1->BTR = CAN_BTR_TS2_0 | CAN_BTR_TS2_1 | CAN_BTR_TS1_3 | CAN_BTR_TS1_1 | 17 ;

	CAN1->FMR &= ~(CAN_FMR_FINIT);
	CAN1->MCR &= ~(CAN_MCR_INRQ);
	while ( (CAN1->MSR & CAN_MSR_INAK) == 0 )
	{

	}
}

void Can_AddFilter(uint16_t *pFilterArray,uint8_t Count)
{
	CAN1->FMR |= CAN_FMR_FINIT;
	CAN1->FM1R = CAN_FM1R_FBM; /* All in filter mode */
	CAN1->FS1R = 0; /* 16bit filter mode */
	CAN1->FFA1R = 0; /* All filters to fifo 0 */
	for (uint8_t i = 0; i < Count; i++)
	{
		uint16_t FilterValue = (uint16_t) (pFilterArray[i] << 5);
		switch (i & 0x3)
		{
		case 0:
			CAN1->sFilterRegister[i >> 2].FR1 = FilterValue;
			break;
		case 1:
			CAN1->sFilterRegister[i >> 2].FR1|= (uint16_t)(FilterValue << 16);
			break;
		case 2:
			CAN1->sFilterRegister[i >> 2].FR2 = FilterValue;
			break;
		case 3:
			CAN1->sFilterRegister[i >> 2].FR2|= (uint16_t)(FilterValue << 16);
			break;
		}
		CAN1->FA1R |= 1u << ( i >> 2);
	}
	CAN1->FMR &= ~(CAN_FMR_FINIT);
}

/**
 * @brief Sends package to inited CAN bus
 * @param Msg message
 */
void Can_SendData(Can_Message_t *Msg)
{
	uint8_t CurrentMailBox = (CAN1->TSR >> 24) & 0x3;
	if (CurrentMailBox < 3)
	{
		CAN1->sTxMailBox[CurrentMailBox].TDTR = Msg->Dlc;
		CAN1->sTxMailBox[CurrentMailBox].TIR = (uint32_t)(Msg->Id)<<21;
		CAN1->sTxMailBox[CurrentMailBox].TDLR = (uint32_t)(Msg->Data[0] | ( Msg->Data[1] << 8) | (Msg->Data[2] << 16) | (Msg->Data[3] << 24));
		CAN1->sTxMailBox[CurrentMailBox].TDHR = (uint32_t)(Msg->Data[4] | ( Msg->Data[5] << 8) | (Msg->Data[6] << 16) | (Msg->Data[7] << 24));
		CAN1->sTxMailBox[CurrentMailBox].TIR |= CAN_TI0R_TXRQ; /* We can use TI0 for all mailboxes, it's the same bit */
	}
}

uint8_t CAN_ReadData_Fifo(const uint8_t Fifo,Can_Message_t *Msg)
{
	uint8_t RetVal = 0;
	if ((CAN1->RF0R & CAN_RF0R_FMP0) != 0) /* Some messages in fifo. We get only one. Upper function must cycle */
	{
		Msg->Id = ((CAN1->sFIFOMailBox[Fifo].RIR) >> 21) & 0x7ff; /* Std ID */
		Msg->Dlc = (CAN1->sFIFOMailBox[Fifo].RDTR) & 0xF; /* DLC */
		for (uint8_t i=0; i < Msg->Dlc; i++)
		{
			Msg->Data[i] = ( ((i<4) ? (CAN1->sFIFOMailBox[Fifo].RDLR) : (CAN1->sFIFOMailBox[Fifo].RDHR)) >>  ((i & 0x3 ) << 3)) & 0xFF;
		}
		CAN1->RF0R |= CAN_RF0R_RFOM0;
		RetVal = 1;
	}
	return RetVal;
}
