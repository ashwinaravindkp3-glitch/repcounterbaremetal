#ifndef UART_BM_H
#define UART_BM_H

#include <stdint.h>

void UART2_Init(uint32_t baudrate);
void UART2_SendChar(char c);
void UART2_SendString(const char *str);
char UART2_ReadChar(void);
uint8_t UART2_IsRxReady(void);

#endif /* UART_BM_H */
