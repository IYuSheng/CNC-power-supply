/* Uart_comm.h 文件 */
#ifndef __UART_COMM_H
#define __UART_COMM_H

#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include "stm32g4xx_ll_usart.h"
#include "stm32g4xx_ll_bus.h"
#include "stm32g4xx_ll_gpio.h"
#include "stm32g4xx_ll_rcc.h"
#include "Uart_Debug.h"

#define UART1_TX_BUF_SIZE	1024
#define UART1_RX_BUF_SIZE	512
#define UART1_TX_QUEUE_SIZE 10  // 队列容量

typedef struct
{
  uint8_t buffer[UART1_TX_BUF_SIZE];  // 单条消息缓冲区
  uint16_t size;                      // 消息长度
  uint8_t in_use;                     // 消息是否有效
} UART1_TX_MESSAGE;

/* 发送结构体：包含电压、电流、温度信息 */
typedef struct
{
  uint16_t voltage;    // 电压（mV）
  uint16_t current;    // 电流（mA）
  uint16_t temperature; // 温度（℃）
	uint16_t adc1;
	uint16_t adc2;
	uint16_t adc3;
	uint16_t adc4;
} UART_TxStruct;

// 定义接收相关结构体（与发送端匹配）
typedef struct
{
  uint8_t start_flag;  // 启动标志：0=停止，1=启动
  uint8_t mode;        // 模式选择：0=默认，1=节能，2=高速
  float adc_a;
	float adc_b;
} UART_RxStruct;  // 发送和接收使用相同结构体（根据实际需求调整）

// 接收缓冲区结构体（环形缓冲区）
typedef struct
{
  uint8_t buffer[UART1_RX_BUF_SIZE];
  volatile uint16_t head;  // 写入指针（中断中更新）
  volatile uint16_t tail;  // 读取指针（主循环中更新）
} UART_RxBufferTypeDef;

// 发送缓冲区相关参数
typedef struct
{
  uint8_t buffer[UART1_TX_BUF_SIZE];  // 发送缓冲区
  uint16_t tx_index;                  // 当前发送索引
  uint16_t tx_size;                   // 总发送长度
  volatile uint8_t tx_busy;           // 发送忙标志（0：空闲，1：发送中）
} UART_TxBufferTypeDef;

// UART设备结构体（包含收发缓冲区及解析相关变量）
typedef struct
{
  UART_RxBufferTypeDef rx_buf;        // 接收缓冲区
  UART_TxBufferTypeDef tx_buf;        // 发送缓冲区
  uint8_t rx_parse_buf[sizeof(UART_RxStruct )];  // 解析缓冲区
  uint16_t rx_parse_len;              // 解析数据长度
  UART_RxStruct  rx_data;              // 解析后的接收数据
} UART_DevTypeDef;

// 全局UART1设备实例（在.c文件中定义，此处声明）
extern UART_DevTypeDef uart1_dev;
// 全局接收数据（供其他模块使用）
extern volatile UART_RxStruct uart_rx_data;
extern UART1_TX_MESSAGE uart1_tx_queue[UART1_TX_QUEUE_SIZE];
extern uint8_t uart1_tx_queue_head;
extern uint8_t uart1_tx_queue_tail;

void UART1_Init(void);
void UART1_Send_IT(USART_TypeDef *USARTx, uint8_t *pData, uint16_t Size);
void UART1_Send_Struct(UART_TxStruct *tx_struct); // 发送结构体
void UART1_Parse_Data(void); // 解析接收数据

#endif /* __UART_COMM_H */
