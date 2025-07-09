#include "Uart_Debug.h"

UART_DEV uart3_dev = {0};
extern UART_TxStruct ua;
static SemaphoreHandle_t uart_tx_mutex = NULL;

static void InitHardUart(void)
{
  // 启用接收中断
  LL_USART_EnableIT_RXNE(USART3);

  uint32_t priority_group = NVIC_GetPriorityGrouping(); // 获取系统优先级分组
  NVIC_SetPriority(USART3_IRQn, NVIC_EncodePriority(priority_group, 3, 1)); // 抢占优先级3，子优先级0
  NVIC_EnableIRQ(USART3_IRQn);
}

static void UartVarInit(void)
{
  memset(&uart3_dev, 0, sizeof(UART_DEV));
  uart3_dev.rx_buf.head = 0;
  uart3_dev.rx_buf.tail = 0;

  // 创建互斥锁保护发送资源
  uart_tx_mutex = xSemaphoreCreateMutex();
  configASSERT(uart_tx_mutex != NULL);
}

void UART_Send_IT(USART_TypeDef *USARTx, uint8_t *pData, uint16_t Size)
{
  if(S_F == 1)	// 系统启动后
    {
      if (xSemaphoreTake(uart_tx_mutex, pdMS_TO_TICKS(10)) != pdTRUE)
        {
          return; // 获取锁失败，放弃发送
        }

      // 等待上次发送完成
      uint32_t timeout = 10; // 100ms超时
      while (uart3_dev.tx_busy && timeout-- > 0)
        {
          vTaskDelay(pdMS_TO_TICKS(1));
        }

    }
  else	//系统启动前
    {
      // 等待上次发送完成
      uint32_t timeout = 10000;
      while (uart3_dev.tx_busy && timeout-- > 0)
        {
          __NOP();
        }
    }

  if (uart3_dev.tx_busy)
    {
      // 强制终止前一次发送
      LL_USART_DisableIT_TC(USART3);
      uart3_dev.tx_busy = 0;
    }

  // 复制数据到发送缓冲区（限制最大长度）
  uint16_t copy_size = Size > UART3_TX_BUF_SIZE ? UART3_TX_BUF_SIZE : Size;
  memcpy(uart3_dev.tx_buf, pData, copy_size);

  // 启动发送
  uart3_dev.tx_index = 0;
  uart3_dev.tx_size = copy_size;
  uart3_dev.tx_busy = 1;

  LL_USART_TransmitData8(USART3, uart3_dev.tx_buf[uart3_dev.tx_index++]);
  LL_USART_EnableIT_TC(USART3);
  if(S_F == 1)
    {
      // 释放互斥锁
      xSemaphoreGive(uart_tx_mutex);
    }
}

void fr_printf(const char *format, ...)
{
  char buffer[UART3_TX_BUF_SIZE];
  va_list args;
  va_start(args, format);
  vsnprintf(buffer, sizeof(buffer), format, args);
  va_end(args);

  // 添加\r\n确保换行
  size_t len = strlen(buffer);
  if (len < sizeof(buffer) - 2)
    {
      buffer[len++] = '\r';
      buffer[len++] = '\n';
      buffer[len] = '\0';
    }

  UART_Send_IT(USART3, (uint8_t *)buffer, len);
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
  uint8_t data;

#if Monitor_Flag
  static uint8_t cmdIndex = 0;              /* 命令缓冲区索引 */
  static char cmdBuffer[64] = {0};          /* 命令缓冲区（Rule 8.12：静态存储期） */
#endif

  for(;;)
    {
      if (uart3_dev.rx_buf.head != uart3_dev.rx_buf.tail) /* 检测接收缓冲数据 */
        {
          taskENTER_CRITICAL();                 /* 进入临界区（Rule 20.7：保护共享资源） */
          data = uart3_dev.rx_buf.buffer[uart3_dev.rx_buf.tail];
          uart3_dev.rx_buf.tail = (uart3_dev.rx_buf.tail + 1) % UART3_TX_BUF_SIZE;
          taskEXIT_CRITICAL();

          UART_Send_IT(USART1, &data, 1);       /* 回显接收数据 */

          /* 命令解析逻辑 */
          if ((data == '\r') || (data == '\n'))  /* 检测行结束符 */
            {
              cmdBuffer[cmdIndex] = '\0';  // 确保字符串以null结尾

              // 解析SetA命令
              if (strncmp(cmdBuffer, "SetA:", 5) == 0)
                {
                  // 将冒号后的字符串转换为整数
                  char* endPtr;
                  long value = strtol(cmdBuffer + 5, &endPtr, 10);

                  // 验证转换是否成功
                  if (endPtr != cmdBuffer + 5 && *endPtr == '\0')
                    {
                      ua.reserved[0] = (uint8_t)value;
                      fr_printf("SetA updated to: %d", ua.reserved[0]);
                    }
                }
              // 解析SetB命令
              else if (strncmp(cmdBuffer, "SetB:", 5) == 0)
                {
                  // 将冒号后的字符串转换为整数
                  char* endPtr;
                  long value = strtol(cmdBuffer + 5, &endPtr, 10);

                  // 验证转换是否成功
                  if (endPtr != cmdBuffer + 5 && *endPtr == '\0')
                    {
                      ua.reserved[1] = (uint8_t)value;
                      fr_printf("SetB updated to: %d", ua.reserved[1]);
                    }
                }
							}

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
          vTaskDelay(pdMS_TO_TICKS(1));    /* 100Hz检测频率 */
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
