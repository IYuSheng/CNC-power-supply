#include "Uart_comm.h"

Uart_dev uart1_dev = {0};
UART_RxStruct uart_rx_data = {0}; // 全局接收数据，供外部访问

UART_TxStruct ua = {0};

static SemaphoreHandle_t uart_mutex = NULL;

static void InitHardUart1(void)
{
  // 启用接收中断
  LL_USART_EnableIT_RXNE(USART1);
	LL_USART_EnableIT_TC(USART1);
	
  uint32_t priority_group = NVIC_GetPriorityGrouping(); // 获取系统优先级分组
  NVIC_SetPriority(USART1_IRQn, NVIC_EncodePriority(priority_group, 2, 0)); // 抢占优先级3，子优先级0
  NVIC_EnableIRQ(USART1_IRQn);
	
	// 启用错误中断
  LL_USART_EnableIT_ERROR(USART1);
}

static void Uart1VarInit(void)
{
  memset(&uart1_dev, 0, sizeof(Uart_dev));
  uart_mutex = xSemaphoreCreateMutex();
  if (uart_mutex == NULL)
    {
      // 互斥锁创建失败
			fr_printf("Mutex create failed");
    }
}

void UART1_Send_IT(USART_TypeDef *USARTx, uint8_t *pData, uint16_t Size)
{
  if (xSemaphoreTake(uart_mutex, pdMS_TO_TICKS(10)) != pdTRUE)
    {
	    fr_printf("Mutex get failed");
      return; // 获取锁失败，放弃发送
    }

  // 等待上次发送完成
  uint32_t timeout = 10; // 100ms超时
  while (uart1_dev.tx_busy && timeout-- > 0)
    {
      vTaskDelay(pdMS_TO_TICKS(1));
    }

  // 若超时，强制复位发送状态（避免永久阻塞）
  if (timeout == 0)
    {
			LL_USART_DisableIT_TC(USART1);
      uart1_dev.tx_busy = 0;
    }

  // 复制数据到发送缓冲区（限制最大长度）
  uint16_t copy_size = Size > UART1_TX_BUF_SIZE ? UART1_TX_BUF_SIZE : Size;
  memcpy(uart1_dev.tx_buf, pData, copy_size);

  // 启动发送
  uart1_dev.tx_index = 0;
  uart1_dev.tx_size = copy_size;
  uart1_dev.tx_busy = 1;

  LL_USART_TransmitData8(USART1, uart1_dev.tx_buf[uart1_dev.tx_index++]);
  LL_USART_EnableIT_TC(USART1);

  // 释放互斥锁
  xSemaphoreGive(uart_mutex);
}

// 发送结构体（打包为帧格式）
void UART1_Send_Struct(UART_TxStruct *tx_struct)
{
  if (tx_struct == NULL) return;

  // 计算帧总大小
  const uint8_t header_size = 1;
  const uint8_t footer_size = 1;
  const uint8_t struct_size = sizeof(UART_TxStruct);
  const uint8_t total_size = header_size + struct_size + footer_size;
  
  // 创建发送缓冲区
  uint8_t tx_buf[total_size];
  
  // 构建帧
  tx_buf[0] = 0xAA; // 帧头
  memcpy(&tx_buf[1], tx_struct, struct_size);
  tx_buf[header_size + struct_size] = 0x55; // 帧尾

  // 发送帧数据
  UART1_Send_IT(USART1, tx_buf, total_size);
}

// 解析接收数据
void UART1_Parse_Data(void)
{
  static uint8_t parse_state = 0; // 0: 等待帧头, 1: 接收数据, 2: 等待帧尾
  
  while (uart1_dev.rx_buf.head != uart1_dev.rx_buf.tail)
  {
    taskENTER_CRITICAL();
    uint8_t data = uart1_dev.rx_buf.buffer[uart1_dev.rx_buf.tail];
    uart1_dev.rx_buf.tail = (uart1_dev.rx_buf.tail + 1) % UART1_RX_BUF_SIZE;
    taskEXIT_CRITICAL();

    switch (parse_state)
    {
      case 0: // 等待帧头
        if (data == 0xAA)
        {
          uart1_dev.rx_parse_len = 0;
          parse_state = 1;
        }
        break;
        
      case 1: // 接收数据
        if (uart1_dev.rx_parse_len < sizeof(uart1_dev.rx_parse_buf))
        {
          uart1_dev.rx_parse_buf[uart1_dev.rx_parse_len++] = data;
          
          // 检查是否接收完整结构体
          if (uart1_dev.rx_parse_len == sizeof(UART_RxStruct))
          {
            parse_state = 2;
          }
        }
        else
        {
          // 缓冲区溢出，重置状态
          parse_state = 0;
        }
        break;
        
      case 2: // 等待帧尾
        if (data == 0x55)
        {
          // 解析结构体数据
          memcpy(&uart1_dev.rx_data, uart1_dev.rx_parse_buf, sizeof(UART_RxStruct));
          
          // 复制到全局变量
          taskENTER_CRITICAL();
          uart_rx_data = uart1_dev.rx_data;
          taskEXIT_CRITICAL();
        }
        parse_state = 0; // 重置状态，无论是否成功
        break;
    }
  }
}

void UART1_Init(void)
{
  InitHardUart1();
  Uart1VarInit();
}

// 串口1中断处理函数
void USART1_IRQHandler(void)
{
  // 错误处理（必须放在最前面）
  if (LL_USART_IsActiveFlag_ORE(USART1) || 
      LL_USART_IsActiveFlag_FE(USART1) ||
      LL_USART_IsActiveFlag_NE(USART1))
  {
    // 清除错误标志
    LL_USART_ClearFlag_ORE(USART1);
    LL_USART_ClearFlag_FE(USART1);
    LL_USART_ClearFlag_NE(USART1);
    
    // 读取DR寄存器清除错误
    (void)LL_USART_ReceiveData8(USART1);
  }

  // 接收中断
  if(LL_USART_IsActiveFlag_RXNE(USART1))
  {
    uint8_t data = LL_USART_ReceiveData8(USART1);
    uint16_t next_head = (uart1_dev.rx_buf.head + 1) % UART1_RX_BUF_SIZE;

    if(next_head != uart1_dev.rx_buf.tail)
    {
      uart1_dev.rx_buf.buffer[uart1_dev.rx_buf.head] = data;
      uart1_dev.rx_buf.head = next_head;
    }
  }
  
  // 发送中断处理
  if(LL_USART_IsActiveFlag_TC(USART1) && LL_USART_IsEnabledIT_TC(USART1))
  {
    LL_USART_ClearFlag_TC(USART1);
    if (uart1_dev.tx_index < uart1_dev.tx_size)
    {
      // 发送下一个字节
      LL_USART_TransmitData8(USART1, uart1_dev.tx_buf[uart1_dev.tx_index++]);
    }
    else
    {
      // 所有数据发送完成
      LL_USART_DisableIT_TC(USART1); // 关闭TC中断
      uart1_dev.tx_busy = 0;        // 标记发送完成
    }
  }
}

void vUart1ProcessTask(void *pvParameters)
{
	ua.start_flag = 1;
	ua.mode = 2;
  for (;;)
    {
			UART1_Send_Struct(&ua);
      UART1_Parse_Data(); // 解析接收数据
      // 访问解析后的结果
//      fr_printf("Voltage: %dmV, Current: %dmA, Temp: %d℃\r\n",
//                uart_rx_data.voltage,
//                uart_rx_data.current,
//                uart_rx_data.temperature);
      vTaskDelay(pdMS_TO_TICKS(100));
    }
}
