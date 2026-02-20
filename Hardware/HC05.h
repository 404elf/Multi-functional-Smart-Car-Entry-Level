#ifndef __HC05_H
#define __HC05_H

#include <stdio.h>

uint8_t Serial_GetRxFlag(void);
uint8_t Serial_GetRxData(void);

extern volatile uint8_t g_USART1_FLAG;	

void HC05_Init(void);
void Serial3_Init(void);
uint32_t Serial_timer(void);
void Serial_SendByte(uint8_t Byte);
void Serial_SendArray(uint8_t *Array, uint16_t Length);
void Serial_SendString(char *String);
void Serial_SendNumber(uint32_t Number, uint8_t Length);
void Serial_Printf(char *format, ...);
void UsartPrintf(USART_TypeDef* USARTx, char *format, ...);
#endif
