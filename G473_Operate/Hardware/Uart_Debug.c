#include "UART_DEBUG.h"

UART_DEV uart3_dev = {0};

static void InitHardUart(void)
{
  // 启用接收中断
  LL_USART_EnableIT_RXNE(USART3);
	uint32_t priority_group = NVIC_GetPriorityGrouping(); // 获取系统优先级分组
  NVIC_SetPriority(USART3_IRQn, NVIC_EncodePriority(priority_group, 5, 0)); // 抢占优先级5，子优先级0
  NVIC_EnableIRQ(USART3_IRQn);
}

static void UartVarInit(void)
{
  memset(&uart3_dev, 0, sizeof(UART_DEV));
}

void UART_Send_IT(USART_TypeDef *USARTx, uint8_t *pData, uint16_t Size)
{
  // 等待上次发送完成
  while(uart3_dev.tx_busy);

  // 复制数据到发送缓冲区
  uint16_t copy_size = Size > UART3_TX_BUF_SIZE ? UART3_TX_BUF_SIZE : Size;
  memcpy(uart3_dev.tx_buf, pData, copy_size);

  // 启用发送中断
  uart3_dev.tx_index = 0;
  uart3_dev.tx_size = copy_size;
  uart3_dev.tx_busy = 1;

  // 修改为USART3
  LL_USART_TransmitData8(USART3, uart3_dev.tx_buf[uart3_dev.tx_index++]);
  LL_USART_EnableIT_TC(USART3);
}

void fr_printf(const char *format, ...)
{
  char buffer[UART3_TX_BUF_SIZE];
  va_list args;
  va_start(args, format);
  vsnprintf(buffer, sizeof(buffer), format, args);
  va_end(args);
  // 修改为USART3
  UART_Send_IT(USART3, (uint8_t *)buffer, strlen(buffer));
}

void UART_Init(void)
{
  MX_USART3_UART_Init();
  InitHardUart();
  UartVarInit();
}

/* 串口命令处理任务 */
void vUartProcessTask(void *pvParameters)
{
  (void)pvParameters;

#if Monitor_Flag
  static uint8_t cmdIndex = 0;              /* 命令缓冲区索引 */
  static char cmdBuffer[64] = {0};          /* 命令缓冲区（Rule 8.12：静态存储期） */
#endif

  for(;;)
    {
      if (uart3_dev.rx_buf.head != uart3_dev.rx_buf.tail) /* 检测接收缓冲数据 */
        {
          uint8_t data;
          taskENTER_CRITICAL();                 /* 进入临界区（Rule 20.7：保护共享资源） */
          data = uart3_dev.rx_buf.buffer[uart3_dev.rx_buf.tail];
          uart3_dev.rx_buf.tail = (uart3_dev.rx_buf.tail + 1) % UART3_RX_BUF_SIZE;
          taskEXIT_CRITICAL();

          UART_Send_IT(USART1, &data, 1);       /* 回显接收数据 */

#if Monitor_Flag
          /* 命令解析逻辑 */
          if ((data == '\r') || (data == '\n'))  /* 检测行结束符 */
            {
              cmdBuffer[cmdIndex] = '\0';
              if (strcmp(cmdBuffer, "status") == 0)
                {
                  vSystemMonitorTask(NULL);         /* 触发系统状态查询 */
                }
              cmdIndex = 0;                       /* 重置缓冲区索引 */
            }
          else if (cmdIndex < (sizeof(cmdBuffer)-1U)) /* 防止缓冲区溢出（Rule 18.1） */
            {
              cmdBuffer[cmdIndex++] = data;       /* 存储有效字符 */
            }
#endif
        }

      vTaskDelay(20 / portTICK_PERIOD_MS);    /* 50Hz检测频率 */
    }
}

// 串口3中断处理函数
void USART3_IRQHandler(void)
{
  if(LL_USART_IsActiveFlag_RXNE(USART3))
  {
    uint8_t data = LL_USART_ReceiveData8(USART3);
    uint16_t next_head = (uart3_dev.rx_buf.head + 1) % UART3_RX_BUF_SIZE;

    if(next_head != uart3_dev.rx_buf.tail)
    {
      uart3_dev.rx_buf.buffer[uart3_dev.rx_buf.head] = data;
      uart3_dev.rx_buf.head = next_head;
    }
    // 重新启用接收中断 (通常不需要，因为RXNE标志会自动清除)
    LL_USART_EnableIT_RXNE(USART3);
  }

  // 发送中断处理
  if(LL_USART_IsActiveFlag_TC(USART3) && LL_USART_IsEnabledIT_TC(USART3))
  {
    LL_USART_ClearFlag_TC(USART3);
    if (uart3_dev.tx_index < uart3_dev.tx_size)
    {
      // 发送下一个字节
      LL_USART_TransmitData8(USART3, uart3_dev.tx_buf[uart3_dev.tx_index++]);
    }
    else
    {
      // 所有数据发送完成
      LL_USART_DisableIT_TC(USART3); // 关闭TC中断
      uart3_dev.tx_busy = 0;        // 标记发送完成
    }
  }
}
