#include "Uart_comm.h"

Uart_dev uart1_dev = {0};
UART_RxStruct uart_rx_data = {0}; // 全局接收数据，供外部访问

UART_TxStruct send_gather = {0};

static SemaphoreHandle_t uart_mutex = NULL; //  串口发送互斥锁
SemaphoreHandle_t uart_data_mutex = NULL; // 串口访问接收数据互斥锁
SemaphoreHandle_t uart_send_mutex = NULL; // 串口发送数据互斥锁

// 添加DMA相关变量
static uint8_t dma_rx_buffer[UART1_RX_BUF_SIZE]; // DMA接收缓冲区
static uint16_t dma_last_pos = 0;

void InitUartDma(void);

static void InitHardUart1(void)
{
  LL_USART_EnableIT_TC(USART1);

  uint32_t priority_group = NVIC_GetPriorityGrouping(); // 获取系统优先级分组
  NVIC_SetPriority(USART1_IRQn, NVIC_EncodePriority(priority_group, 3, 0)); // 抢占优先级3，子优先级0
  NVIC_EnableIRQ(USART1_IRQn);

  // 初始化DMA
  InitUartDma();
}

static void Uart1VarInit(void)
{
  memset(&uart1_dev, 0, sizeof(Uart_dev));
  uart_mutex = xSemaphoreCreateMutex();
  if (uart_mutex == NULL)
    {
      fr_printf("Mutex create failed");
    }
  
  // 创建数据访问互斥锁
  uart_data_mutex = xSemaphoreCreateMutex();
  if (uart_data_mutex == NULL)
    {
      fr_printf("Data mutex create failed");
    }
  
  // 创建发送数据互斥锁
  uart_send_mutex = xSemaphoreCreateMutex();
  if (uart_send_mutex == NULL)
    {
      fr_printf("Send data mutex create failed");
    }
}

// DMA串口接收初始化函数
void InitUartDma(void)
{
  // 使能DMA时钟
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_DMA1);
  
  // 配置DMA通道
  LL_DMA_SetPeriphRequest(DMA1, LL_DMA_CHANNEL_1, LL_DMAMUX_REQ_USART1_RX);
  LL_DMA_SetDataTransferDirection(DMA1, LL_DMA_CHANNEL_1, LL_DMA_DIRECTION_PERIPH_TO_MEMORY);
  LL_DMA_SetChannelPriorityLevel(DMA1, LL_DMA_CHANNEL_1, LL_DMA_PRIORITY_HIGH);
  LL_DMA_SetMode(DMA1, LL_DMA_CHANNEL_1, LL_DMA_MODE_CIRCULAR);
  LL_DMA_SetPeriphIncMode(DMA1, LL_DMA_CHANNEL_1, LL_DMA_PERIPH_NOINCREMENT);
  LL_DMA_SetMemoryIncMode(DMA1, LL_DMA_CHANNEL_1, LL_DMA_MEMORY_INCREMENT);
  LL_DMA_SetPeriphSize(DMA1, LL_DMA_CHANNEL_1, LL_DMA_PDATAALIGN_BYTE);
  LL_DMA_SetMemorySize(DMA1, LL_DMA_CHANNEL_1, LL_DMA_MDATAALIGN_BYTE);
  
  // 设置外设地址
  LL_DMA_SetPeriphAddress(DMA1, LL_DMA_CHANNEL_1, LL_USART_DMA_GetRegAddr(USART1, LL_USART_DMA_REG_DATA_RECEIVE));
  // 设置内存地址
  LL_DMA_SetMemoryAddress(DMA1, LL_DMA_CHANNEL_1, (uint32_t)dma_rx_buffer);
  // 设置数据长度
  LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_1, UART1_RX_BUF_SIZE);
  
  // 使能DMA传输完成中断
  LL_DMA_EnableIT_TC(DMA1, LL_DMA_CHANNEL_1);
  
  // 配置DMA中断优先级
  NVIC_SetPriority(DMA1_Channel1_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 1, 0));
  NVIC_EnableIRQ(DMA1_Channel1_IRQn);
  
  // 使能DMA通道
  LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_1);
  
  // 使能USART的DMA接收
  LL_USART_EnableDMAReq_RX(USART1);
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

  const uint16_t struct_size = sizeof(UART_RxStruct);
  
  // 获取当前DMA写入位置
  uint16_t dma_current_pos = UART1_RX_BUF_SIZE - LL_DMA_GetDataLength(DMA1, LL_DMA_CHANNEL_1);
  
  // 处理缓冲区中的数据
  while (dma_last_pos != dma_current_pos)
  {
    uint8_t data = dma_rx_buffer[dma_last_pos];
    dma_last_pos = (dma_last_pos + 1) % UART1_RX_BUF_SIZE;

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
        if (uart1_dev.rx_parse_len < struct_size)
        {
          uart1_dev.rx_parse_buf[uart1_dev.rx_parse_len++] = data;

          // 检查是否接收完整结构体
          if (uart1_dev.rx_parse_len == struct_size)
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
          memcpy(&uart1_dev.rx_data, uart1_dev.rx_parse_buf, struct_size);

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
  MX_USART1_UART_Init();
  InitHardUart1();
  Uart1VarInit();
}

// 串口1中断处理发送函数
void USART1_IRQHandler(void)
{
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

void DMA1_Channel1_IRQHandler(void)
{
  // 检查是否传输完成中断
  if (LL_DMA_IsActiveFlag_TC1(DMA1))
  {
    // 清除中断标志
    LL_DMA_ClearFlag_TC1(DMA1);
  }
}

// 获取最新的UART接收数据（线程安全）
UART_RxStruct get_uart_rx_data(void)
{
    UART_RxStruct data = {0};
    
    // 获取互斥锁
    if (xSemaphoreTake(uart_data_mutex, pdMS_TO_TICKS(5)) == pdTRUE)
    {
        // 复制数据
        memcpy(&data, &uart_rx_data, sizeof(UART_RxStruct));
        // 释放互斥锁
        xSemaphoreGive(uart_data_mutex);
    }
    
    return data;
}

// 获取发送数据的副本（线程安全）
UART_TxStruct get_uart_tx_data(void)
{
    UART_TxStruct data = {0};
    
    // 获取互斥锁
    if (uart_send_mutex != NULL && xSemaphoreTake(uart_send_mutex, pdMS_TO_TICKS(5)) == pdTRUE)
    {
        // 复制数据
        memcpy(&data, &send_gather, sizeof(UART_TxStruct));
        // 释放互斥锁
        xSemaphoreGive(uart_send_mutex);
    }
    
    return data;
}

// 设置发送数据（线程安全）
void set_uart_tx_data(UART_TxStruct *data)
{
    if (data == NULL) return;
    
    // 获取互斥锁
    if (uart_send_mutex != NULL && xSemaphoreTake(uart_send_mutex, pdMS_TO_TICKS(5)) == pdTRUE)
    {
        // 复制数据
        memcpy(&send_gather, data, sizeof(UART_TxStruct));
        // 释放互斥锁
        xSemaphoreGive(uart_send_mutex);
    }
}

void vUart1ProcessTask(void *pvParameters)
{
  send_gather.start_flag = 1;
  send_gather.mode = 2;
  for (;;)
    {
      UART1_Send_Struct(&send_gather);
      UART1_Parse_Data(); // 解析接收数据
      // 发送给上位机
      dma_printf("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",
                 uart_rx_data.voltage_out,
                 uart_rx_data.current_out,
                 uart_rx_data.voltage_in,
                 uart_rx_data.current_in,
                 uart_rx_data.adc_tmp1,
                 uart_rx_data.adc_tmp2,
                 uart_rx_data.voltage_12V_in,
                 uart_rx_data.voltage_5V_in,
                 uart_rx_data.mode_stop,
                 uart_rx_data.mode_flag);
      vTaskDelay(pdMS_TO_TICKS(1));
    }
}
