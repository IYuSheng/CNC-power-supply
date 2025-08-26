#include "st7789.h"

// 全局变量定义
volatile bool st7789_spi_tx_complete = false;
extern lv_disp_drv_t * current_drv;
extern const lv_area_t * current_area;

/**
 * @brief  初始化DMA用于SPI数据传输
 */
void DMA_Init(void)
{
  // 使能DMA时钟
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_DMAMUX1);
  LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_DMA1);

  // 设置外设地址为SPI数据寄存器
  LL_DMA_SetPeriphAddress(ST7789_DMA, ST7789_DMA_CHANNEL, (uint32_t)&ST7789_SPI->DR);

  // 设置DMA中断
  NVIC_SetPriority(DMA1_Channel3_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 2, 0));
  NVIC_EnableIRQ(DMA1_Channel3_IRQn);
  LL_DMA_EnableIT_TC(ST7789_DMA, ST7789_DMA_CHANNEL);
}

/**
 * @brief  初始化ST7789屏幕
 * @param  无
 * @retval 无
 */
void ST7789_Init(void)
{
  // 2. 硬件复位，确保时序正确并等待时钟稳定
  ST7789_RST_HIGH();
  DWT_Delayms(1);
  ST7789_RST_LOW();
  DWT_Delayms(10);
  ST7789_RST_HIGH();
  DWT_Delayms(200);

  // 3. 初始化屏幕
  ST7789_CS_LOW();

  // 退出睡眠模式
  ST7789_WriteCmd(0x11);
  DWT_Delayms(200);

  // 4. 显示方向设置，使用0x70设置240*320方向
  ST7789_WriteCmd(0x36);
  ST7789_WriteDataByte(0x70);  // 设置240*320方向

  // 5. 像素格式设置，使用16位RGB565
  ST7789_WriteCmd(0x3A);
  ST7789_WriteDataByte(0x55);  // 16位RGB565
  DWT_Delayms(10);

  // 6. 帧率设置，使用默认参数
  ST7789_WriteCmd(0xB2);  // Porch Setting
  ST7789_WriteDataByte(0x0C);
  ST7789_WriteDataByte(0x0C);
  ST7789_WriteDataByte(0x00);
  ST7789_WriteDataByte(0x33);
  ST7789_WriteDataByte(0x33);

  ST7789_WriteCmd(0xB7);  // Gate Control
  ST7789_WriteDataByte(0x75);  // 默认参数

  // 7. 电源设置，使用默认参数
  ST7789_WriteCmd(0xC2);  // VDV and VRH Command Enable
  ST7789_WriteDataByte(0x01);

  ST7789_WriteCmd(0xC3);  // VRH Set
  ST7789_WriteDataByte(0x16);  // 默认参数

  ST7789_WriteCmd(0xC4);  // VDV Set
  ST7789_WriteDataByte(0x20);  // 默认参数

  ST7789_WriteCmd(0xC6);  // Frame Rate Control
  ST7789_WriteDataByte(0x0F);  // 默认参数

  ST7789_WriteCmd(0xD0);  // Power Control 1
  ST7789_WriteDataByte(0xA4);
  ST7789_WriteDataByte(0xA1);

  ST7789_WriteCmd(0xD6);
  ST7789_WriteDataByte(0xA1);

  ST7789_WriteCmd(0xBB);  // VCOM
  ST7789_WriteDataByte(0x3B);  // 默认参数

  // 8. 伽马校正，使用默认参数
  ST7789_WriteCmd(0xE0);  // 正伽马校正
  uint8_t gamma_pos[] = {0xF0,0x0B,0x11,0x0E,0x0D,0x19,0x36,0x33,0x4B,0x07,0x14,0x14,0x2C,0x2E};
  ST7789_WriteData(gamma_pos, 14);

  ST7789_WriteCmd(0xE1);  // 负伽马校正
  uint8_t gamma_neg[] = {0xF0,0x0D,0x12,0x0B,0x09,0x03,0x32,0x44,0x48,0x39,0x16,0x16,0x2D,0x30};
  ST7789_WriteData(gamma_neg, 14);

  ST7789_WriteCmd(0xE4);
  ST7789_WriteDataByte(0x27);
  ST7789_WriteDataByte(0x00);
  ST7789_WriteDataByte(0x00);

  // 9. 开启显示
  ST7789_WriteCmd(0x29);
  ST7789_WriteCmd(0x2C);
  DWT_Delayms(10);

  // 10. 开启背光
  //ST7789_BLK_LOW();

  ST7789_CS_HIGH();
}

/**
 * @brief  通过DMA发送数据
 * @param  data: 数据缓冲区
 * @param  size: 数据大小（字节）
 */
void ST7789_SPI_Transmit_DMA(uint8_t *data, uint32_t size)
{
  // 等待SPI空闲
  while(LL_SPI_IsActiveFlag_BSY(ST7789_SPI));

  // 禁用DMA通道
  LL_DMA_DisableChannel(ST7789_DMA, ST7789_DMA_CHANNEL);
  while(LL_DMA_IsEnabledChannel(ST7789_DMA, ST7789_DMA_CHANNEL));

  // 设置DMA参数
  LL_DMA_SetMemoryAddress(ST7789_DMA, ST7789_DMA_CHANNEL, (uint32_t)data);
  LL_DMA_SetDataLength(ST7789_DMA, ST7789_DMA_CHANNEL, size);

  // 清除传输完成标志
  LL_DMA_ClearFlag_TC3(ST7789_DMA);

  // 启动传输
  st7789_spi_tx_complete = false;
  LL_DMA_EnableChannel(ST7789_DMA, ST7789_DMA_CHANNEL);
}

// 单字节SPI写入函数
static void ST7789_SPI_WriteByte(uint8_t data)
{
  // 等待发送缓冲区为空
  while (!LL_SPI_IsActiveFlag_TXE(SPI1));

  // 发送数据
  LL_SPI_TransmitData8(SPI1, data);

  // 等待传输完成
  while (LL_SPI_IsActiveFlag_BSY(SPI1));
}

void ST7789_WriteCmd(uint8_t cmd)
{
  ST7789_CS_LOW();    // 片选使能
  ST7789_DC_LOW();    // 命令模式
  ST7789_SPI_WriteByte(cmd);
  ST7789_CS_HIGH();   // 片选释放
}

void ST7789_WriteDataByte(uint8_t data)
{
  ST7789_CS_LOW();    // 片选使能
  ST7789_DC_HIGH();   // 数据模式
  ST7789_SPI_WriteByte(data);
  ST7789_CS_HIGH();   // 片选释放
}

// 优化版本支持多字节数据传输
void ST7789_WriteData(uint8_t *data, uint32_t len)
{
  ST7789_CS_LOW();
  ST7789_DC_HIGH();

  // 等待SPI空闲
  while (!LL_SPI_IsActiveFlag_TXE(SPI1));

  // 发送数据数组
  for (uint32_t i = 0; i < len; i++)
    {
      LL_SPI_TransmitData8(SPI1, data[i]);
      // 额外等待以防发送缓冲区满时等待
      if (i < len - 1 && !LL_SPI_IsActiveFlag_TXE(SPI1))
        {
          while (!LL_SPI_IsActiveFlag_TXE(SPI1));
        }
    }

  // 等待所有数据发送完成
  while (LL_SPI_IsActiveFlag_BSY(SPI1));
  ST7789_CS_HIGH();
}

/**
 * @brief  设置显示窗口
 * @param  x0,y0: 左上角坐标
 * @param  x1,y1: 右下角坐标
 * @param  dir_mode: 显示方向模式
 */
void ST7789_SetWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
  // 开始SPI事务（保持CS低电平直到完成所有窗口设置）
  ST7789_CS_LOW();

  // 发送列地址设置命令（CASET, 0x2A）
  ST7789_DC_LOW();  // 命令模式
  while (!LL_SPI_IsActiveFlag_TXE(SPI1));
  LL_SPI_TransmitData8(SPI1, 0x2A);
  while (LL_SPI_IsActiveFlag_BSY(SPI1));

  // 发送列地址数据（x0~x1）
  ST7789_DC_HIGH();  // 数据模式
  uint8_t col_data[4] =
  {
    ((x0) >> 8),  // x0高8位
    (x0),         // x0低8位
    ((x1) >> 8),  // x1高8位
    (x1)           // x1低8位
  };

  for (uint32_t i = 0; i < 4; i++)
    {
      while (!LL_SPI_IsActiveFlag_TXE(SPI1));
      LL_SPI_TransmitData8(SPI1, col_data[i]);
    }
  while (LL_SPI_IsActiveFlag_BSY(SPI1));

  // 发送行地址设置命令（RASET, 0x2B）
  ST7789_DC_LOW();
  while (!LL_SPI_IsActiveFlag_TXE(SPI1));
  LL_SPI_TransmitData8(SPI1, 0x2B);
  while (LL_SPI_IsActiveFlag_BSY(SPI1));

  // 发送行地址数据（y0~y1）
  ST7789_DC_HIGH();
  uint8_t row_data[4] =
  {
    (y0 >> 8),  // y0高8位
    y0,         // y0低8位
    (y1 >> 8),  // y1高8位
    y1         // y1低8位
  };

  for (uint32_t i = 0; i < 4; i++)
    {
      while (!LL_SPI_IsActiveFlag_TXE(SPI1));
      LL_SPI_TransmitData8(SPI1, row_data[i]);
    }
  while (LL_SPI_IsActiveFlag_BSY(SPI1));

  // 发送写GRAM命令（RAMWR, 0x2C）
  ST7789_DC_LOW();
  while (!LL_SPI_IsActiveFlag_TXE(SPI1));
  LL_SPI_TransmitData8(SPI1, 0x2C);
  while (LL_SPI_IsActiveFlag_BSY(SPI1));

  // 结束SPI事务
  ST7789_CS_HIGH();
}

/**
 * @brief  填充指定区域颜色（使用DMA加速）
 * @param  x0,y0: 左上角坐标
 * @param  x1,y1: 右下角坐标
 * @param  color: 填充颜色
 * @param  dir_mode: 显示方向模式
 */
void ST7789_Fill(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color)
{
  // 计算像素总数
  uint32_t pixel_num = (x1 - x0 + 1) * (y1 - y0 + 1);
  // 颜色为高低字节（RGB565格式，前高后低，循环发送）
  uint8_t color_h = color >> 8;
  uint8_t color_l = color & 0xFF;

  // 1. 设置窗口
  ST7789_SetWindow(x0, y0, x1, y1);

  // 2. 准备颜色数据，直接使用SPI发送（ST7789_WriteData的函数可用来发送）
  ST7789_CS_LOW();
  ST7789_DC_HIGH();  // 数据模式

  // 等待SPI空闲
  while (!LL_SPI_IsActiveFlag_TXE(SPI1));

  // 循环发送颜色数据（每发送2字节颜色数据就等待SPI发送完成）
  for (uint32_t i = 0; i < pixel_num; i++)
    {
      // 发送高字节
      LL_SPI_TransmitData8(SPI1, color_h);
      while (i < pixel_num - 1 && !LL_SPI_IsActiveFlag_TXE(SPI1));

      // 发送低字节
      LL_SPI_TransmitData8(SPI1, color_l);
      while (i < pixel_num - 1 && !LL_SPI_IsActiveFlag_TXE(SPI1));
    }

  // 等待最后一个数据发送完成
  while (LL_SPI_IsActiveFlag_BSY(SPI1));
  ST7789_CS_HIGH();
}

/**
 * @brief  屏幕刷新线程
 * @param  无
 * @retval 无
 */
void vTFTTask(void *pvParameters)
{
  
  Timer_Init(); // 初始化定时器
  
  for (;;)
  {
    // 如果当前屏幕已加载，处理屏幕事件数据
    if(main_screen_loaded)
    {
      Gui_Event_Data();
    }

    lv_timer_handler();
    vTaskDelay(pdMS_TO_TICKS(LVGL_TICK_PERIOD_MS));
  }
}

/**
 * @brief  DMA传输完成中断处理函数
 */
void DMA1_Channel3_IRQHandler(void)
{
  if(LL_DMA_IsActiveFlag_TC3(ST7789_DMA))
  {
    LL_DMA_ClearFlag_TC3(ST7789_DMA);
    LL_DMA_DisableChannel(ST7789_DMA, ST7789_DMA_CHANNEL);

    while(LL_SPI_IsActiveFlag_BSY(ST7789_SPI));
    // 释放片选
    ST7789_CS_HIGH();
    
    // 通知LVGL刷新完成
    if(current_drv != NULL)
    {
      lv_disp_flush_ready(current_drv);
      current_drv = NULL;
      current_area = NULL;
    }
  }
}
