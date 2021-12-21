#ifndef __USART_H
#define __USART_H

#include <stdint.h>

extern uint8_t USART_RX_BUF[200];

extern uint16_t USART_RX_STA;

void USART_Init(void);

#endif	
