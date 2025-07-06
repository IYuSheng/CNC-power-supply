#ifndef __UART_DEBUG_H
#define __UART_DEBUG_H

#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include "stm32g4xx_ll_usart.h"
#include "stm32g4xx_ll_bus.h"
#include "stm32g4xx_ll_gpio.h"
#include "stm32g4xx_ll_rcc.h"

#define UART3_TX_BUF_SIZE	512
#define UART3_RX_BUF_SIZE	1024

/* 串口环形缓冲区结构体 */
typedef struct
{
  uint8_t buffer[UART3_RX_BUF_SIZE];
  volatile uint16_t head;
  volatile uint16_t tail;
} RingBuffer;

/* 串口设备结构体 */
typedef struct
{
  RingBuffer rx_buf;       // 接收环形缓冲区
  uint8_t tx_buf[UART3_TX_BUF_SIZE]; // 发送缓冲区
  volatile uint8_t tx_busy; // 发送忙标志
  volatile uint16_t tx_index;   // 当前发送位置
  volatile uint16_t tx_size;    // 本次需发送的总字节数
} UART_DEV;

extern UART_DEV uart3_dev;//外部声明串口设备结构体句柄

void UART_Init(void);
void UART_Send_IT(USART_TypeDef *USARTx, uint8_t *pData, uint16_t Size);
void Debug_printf(const char *format, ...);

#endif
