#include "Uart_Debug.h"

UART_DEV uart3_dev = {0};

static void MX_USART3_UART_Init(void)
{

  /* USER CODE BEGIN USART3_Init 0 */

  /* USER CODE END USART3_Init 0 */

  LL_USART_InitTypeDef USART_InitStruct = {0};

  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

  LL_RCC_SetUSARTClockSource(LL_RCC_USART3_CLKSOURCE_PCLK1);

  /* Peripheral clock enable */
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_USART3);

  LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_GPIOB);
  /**USART3 GPIO Configuration
  PB10   ------> USART3_TX
  PB11   ------> USART3_RX
  */
  GPIO_InitStruct.Pin = LL_GPIO_PIN_10;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  GPIO_InitStruct.Alternate = LL_GPIO_AF_7;
  LL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = LL_GPIO_PIN_11;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  GPIO_InitStruct.Alternate = LL_GPIO_AF_7;
  LL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* USER CODE BEGIN USART3_Init 1 */

  /* USER CODE END USART3_Init 1 */
  USART_InitStruct.PrescalerValue = LL_USART_PRESCALER_DIV1;
  USART_InitStruct.BaudRate = 115200;
  USART_InitStruct.DataWidth = LL_USART_DATAWIDTH_8B;
  USART_InitStruct.StopBits = LL_USART_STOPBITS_1;
  USART_InitStruct.Parity = LL_USART_PARITY_NONE;
  USART_InitStruct.TransferDirection = LL_USART_DIRECTION_TX_RX;
  USART_InitStruct.HardwareFlowControl = LL_USART_HWCONTROL_NONE;
  USART_InitStruct.OverSampling = LL_USART_OVERSAMPLING_16;
  LL_USART_Init(USART3, &USART_InitStruct);
  LL_USART_SetTXFIFOThreshold(USART3, LL_USART_FIFOTHRESHOLD_1_8);
  LL_USART_SetRXFIFOThreshold(USART3, LL_USART_FIFOTHRESHOLD_1_8);
  LL_USART_DisableFIFO(USART3);
  LL_USART_ConfigAsyncMode(USART3);

  /* USER CODE BEGIN WKUPType USART3 */

  /* USER CODE END WKUPType USART3 */

  LL_USART_Enable(USART3);

  /* Polling USART3 initialisation */
  while((!(LL_USART_IsActiveFlag_TEACK(USART3))) || (!(LL_USART_IsActiveFlag_REACK(USART3))))
    {
    }
  /* USER CODE BEGIN USART3_Init 2 */

  /* USER CODE END USART3_Init 2 */

}

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
}

void UART_Send_IT(USART_TypeDef *USARTx, uint8_t *pData, uint16_t Size)
{
  // 等待上次发送完成
  uint32_t timeout = 10000;
  while (uart3_dev.tx_busy && timeout-- > 0)
    {
      __NOP();
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
}

void Debug_printf(const char *format, ...)
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
