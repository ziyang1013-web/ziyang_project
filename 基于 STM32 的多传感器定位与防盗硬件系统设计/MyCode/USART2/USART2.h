
#ifndef __SERIAL_H
#define __SERIAL_H

#include <stdio.h>
extern uint8_t Wstr[100];
extern volatile uint8_t Wi;
extern uint8_t Wstate;
void Serial_Init(void);
void Serial_SendByte(uint8_t Byte);
void Serial_SendArray(uint8_t *Array, uint16_t Length);
void Serial_SendString(char *String);
void Serial_SendNumber(uint32_t Number, uint8_t Length);
void Serial_Printf(char *format, ...);
void USART2_sendChar(uint8_t c);
void USART2_sendString(uint8_t *p);
uint8_t Serial_GetRxFlag(void);
uint8_t Serial_GetRxData(void);

#endif
