#include "Uart_Debug.h"

UART_DEV uart3_dev = {0};
extern UART_TxStruct send_gather;
static SemaphoreHandle_t uart_tx_mutex = NULL;

// 添加DMA相关变量
static uint8_t dma_tx_buffer[UART3_TX_BUF_SIZE];
static SemaphoreHandle_t dma_tx_sem = NULL;

void Init_UartDma(void);

static void InitHardUart(void)
{
  // 启用接收中断
  LL_USART_EnableIT_RXNE(USART3);

  NVIC_SetPriority(USART3_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 4, 0)); // 抢占优先级4，子优先级0
  NVIC_EnableIRQ(USART3_IRQn);

  Init_UartDma();
}

static void UartVarInit(void)
{
  memset(&uart3_dev, 0, sizeof(UART_DEV));
  uart3_dev.rx_buf.head = 0;
  uart3_dev.rx_buf.tail = 0;

  // 创建互斥锁保护发送资源
  uart_tx_mutex = xSemaphoreCreateMutex();
  configASSERT(uart_tx_mutex != NULL);

  // 创建二值信号量用于DMA传输同步
  dma_tx_sem = xSemaphoreCreateBinary();
  configASSERT(dma_tx_sem != NULL);
  // 初始化时给予一个令牌，表示DMA可用
  xSemaphoreGive(dma_tx_sem);
}

void Init_UartDma(void)
{
  // 使能DMA时钟
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_DMA1);
  
  // 配置DMA通道
  LL_DMA_SetPeriphRequest(DMA1, LL_DMA_CHANNEL_2, LL_DMAMUX_REQ_USART3_TX);
  LL_DMA_SetDataTransferDirection(DMA1, LL_DMA_CHANNEL_2, LL_DMA_DIRECTION_MEMORY_TO_PERIPH);
  LL_DMA_SetChannelPriorityLevel(DMA1, LL_DMA_CHANNEL_2, LL_DMA_PRIORITY_HIGH);
  LL_DMA_SetMode(DMA1, LL_DMA_CHANNEL_2, LL_DMA_MODE_NORMAL);
  LL_DMA_SetPeriphIncMode(DMA1, LL_DMA_CHANNEL_2, LL_DMA_PERIPH_NOINCREMENT);
  LL_DMA_SetMemoryIncMode(DMA1, LL_DMA_CHANNEL_2, LL_DMA_MEMORY_INCREMENT);
  LL_DMA_SetPeriphSize(DMA1, LL_DMA_CHANNEL_2, LL_DMA_PDATAALIGN_BYTE);
  LL_DMA_SetMemorySize(DMA1, LL_DMA_CHANNEL_2, LL_DMA_MDATAALIGN_BYTE);
  
  // 设置外设地址
  LL_DMA_SetPeriphAddress(DMA1, LL_DMA_CHANNEL_2, LL_USART_DMA_GetRegAddr(USART3, LL_USART_DMA_REG_DATA_TRANSMIT));
  
  // 使能DMA传输完成中断
  LL_DMA_EnableIT_TC(DMA1, LL_DMA_CHANNEL_2);
  
  // 配置DMA中断优先级
  NVIC_SetPriority(DMA1_Channel2_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 8, 0));
  NVIC_EnableIRQ(DMA1_Channel2_IRQn);
  
  // 使能USART的DMA发送
  LL_USART_EnableDMAReq_TX(USART3);
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
      uint32_t timeout = 100; // 100ms超时
      while (uart3_dev.tx_busy && timeout-- > 0)
        {
          vTaskDelay(pdMS_TO_TICKS(1));
        }

    }
  else	//系统启动前
    {
      // 等待上次发送完成
      uint32_t timeout = 100000;
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

// DMA发送函数
void UART_Send_DMA(USART_TypeDef *USARTx, uint8_t *pData, uint16_t Size)
{
  if (Size > UART3_TX_BUF_SIZE) Size = UART3_TX_BUF_SIZE;
  
  // 等待上次DMA传输完成，使用信号量等待，超时100ms
  if (xSemaphoreTake(dma_tx_sem, pdMS_TO_TICKS(100)) != pdTRUE)
  {
    // 如果超时，强制停止DMA传输
    LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_2);
  }
  
  // 设置DMA内存地址和传输数据长度
  LL_DMA_SetMemoryAddress(DMA1, LL_DMA_CHANNEL_2, (uint32_t)dma_tx_buffer);
  LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_2, Size);

  // 复制数据到DMA缓冲区
  memcpy(dma_tx_buffer, pData, Size);
  
  // 使能DMA通道开始传输
  LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_2);
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

void dma_printf(const char *format, ...)
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

  UART_Send_DMA(USART3, (uint8_t *)buffer, len);
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
    static uint8_t cmdIndex = 0;              /* 命令缓冲区索引 */
    static char cmdBuffer[64] = {0};          /* 命令缓冲区 */

    for(;;)
    {
        if (uart3_dev.rx_buf.head != uart3_dev.rx_buf.tail)
        {
            /* 1. 读取接收数据 */
            taskENTER_CRITICAL();
            data = uart3_dev.rx_buf.buffer[uart3_dev.rx_buf.tail];
            uart3_dev.rx_buf.tail = (uart3_dev.rx_buf.tail + 1) % UART3_RX_BUF_SIZE;
            taskEXIT_CRITICAL();

            /* 2. 数据回显 暂不需要，调试用 */
            //UART_Send_IT(USART1, &data, 1);

            /* 3. 命令缓冲区管理与处理（基本命令） */
            if ((data == '\r') || (data == '\n'))  /* 检测行结束符 */
            {
                cmdBuffer[cmdIndex] = '\0';  // 确保字符串以null结尾

                /* 处理基本命令 */
                TriggerCommandProcessing(cmdBuffer);

                cmdIndex = 0;  // 重置缓冲区
                memset(cmdBuffer, 0, sizeof(cmdBuffer));
            }
            else if (cmdIndex < (sizeof(cmdBuffer) - 1U))  /* 防止缓冲区溢出 */
            {
                cmdBuffer[cmdIndex++] = data;
            }
        }

        vTaskDelay(pdMS_TO_TICKS(10));
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

// DMA发送中断处理函数
void DMA1_Channel2_IRQHandler(void)
{
  // 检查是否传输完成中断
  if (LL_DMA_IsActiveFlag_TC2(DMA1))
  {
    // 清除中断标志
    LL_DMA_ClearFlag_TC2(DMA1);
    
    // 禁用DMA通道
    LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_2);
    
    // 给信号量发送一个令牌，表示DMA传输完成
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xSemaphoreGiveFromISR(dma_tx_sem, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
  }
}

