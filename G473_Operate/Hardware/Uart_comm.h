/* Uart_comm.h 文件 */
#ifndef __UART_COMM_H
#define __UART_COMM_H

#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"
#include "stm32g4xx_ll_usart.h"
#include "usart.h"
#include "Uart_Debug.h"

#define UART1_TX_BUF_SIZE	256
#define UART1_RX_BUF_SIZE	512

/* 发送结构体：包含启停指令及附加参数（均为uint8_t） */
typedef struct
{
  uint8_t start_flag;  // 启动标志：0=停止，1=启动
  uint8_t mode;        // 模式选择：0=默认，1=节能，2=高速
  //后续可想办法在此添加两个uint8_t确保对齐
  float dac_a;
	float dac_b;
} UART_TxStruct;

/* 接收结构体：包含电压、电流、温度信息 */
#pragma pack(1) // 强制1字节对齐，确保结构体大小在两端一致
typedef struct
{
  uint16_t voltage_out;    // 对应发送端的voltage_out
  uint16_t current_out;    // 对应发送端的current_out
  uint16_t voltage_in;     // 对应发送端的voltage_in
  uint16_t current_in;     // 对应发送端的current_in
  uint16_t adc_tmp1;       // 对应发送端的adc_tmp1
  uint16_t adc_tmp2;       // 对应发送端的adc_tmp2
  uint16_t voltage_12V_in; // 对应发送端的voltage_12V_in
  uint16_t voltage_5V_in;  // 对应发送端的voltage_5V_in
  uint8_t mode_stop;       // 对应发送端的mode_stop
  uint8_t mode_flag;       // 对应发送端的mode_flag
} UART_RxStruct;
#pragma pack() // 恢复默认对齐

/* 串口环形缓冲区结构体 */
typedef struct
{
  uint8_t buffer[UART1_RX_BUF_SIZE];
  volatile uint16_t head;
  volatile uint16_t tail;
} Ringbuffer;

/* 串口设备结构体 */
typedef struct
{
  Ringbuffer rx_buf;       // 接收环形缓冲区
  uint8_t tx_buf[UART1_TX_BUF_SIZE]; // 发送缓冲区
  volatile uint8_t tx_busy; // 发送忙标志
  volatile uint16_t tx_index;   // 当前发送位置
  volatile uint16_t tx_size;    // 本次需发送的总字节数

  uint8_t rx_parse_buf[UART1_RX_BUF_SIZE]; // 解析缓冲区
  uint8_t rx_parse_len;     // 已接收解析字节数
  UART_RxStruct rx_data;    // 解析后的接收数据
} Uart_dev;

extern Uart_dev uart1_dev;//外部声明串口设备结构体句柄
extern UART_RxStruct uart_rx_data; // 供外部访问的接收数据
extern UART_TxStruct send_gather;

extern SemaphoreHandle_t uart_data_mutex;
extern SemaphoreHandle_t uart_send_mutex;

void UART1_Init(void);
void UART1_Send_IT(USART_TypeDef *USARTx, uint8_t *pData, uint16_t Size);
void UART1_Send_Struct(UART_TxStruct *tx_struct); // 发送结构体
void UART1_Parse_Data(void); // 解析接收数据
UART_RxStruct get_uart_rx_data(void);
UART_TxStruct get_uart_tx_data(void);
void set_uart_tx_data(UART_TxStruct *data);

void vUart1ProcessTask(void *pvParameters);

#endif /* __UART_COMM_H */
