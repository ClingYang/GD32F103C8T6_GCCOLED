#ifndef __USART_H
#define __USART_H
#include "gd32f10x.h"
#include <stdio.h>
#include <stdarg.h>

#define USART2_RDATA_ADDRESS      ((uint32_t)&USART_DATA(USART2))
#define USARTx_RX_BY_IDLE_DMA	0		//1:使用接收空闲中断+DMA   0:仅使用接收中断
extern uint8_t rxbuffer[256];
extern uint8_t rx_count;
extern uint8_t tx_count;
extern __IO uint8_t receive_flag;

void dma_config(void);
void usart_config(void);
void usart_config(void);
void printf2(const char *fmt, ...);

#endif /* __USART_H */
