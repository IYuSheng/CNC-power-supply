#include "st7789.h"


/**
 * @brief  初始化ST7789屏幕（整合正确初始化序列）
 * @param  无
 * @retval 无
 */
void ST7789_Init(void)
{
  // 2. 硬件复位（按正确时序调整延时）
  ST7789_RST_HIGH();
  vTaskDelay(pdMS_TO_TICKS(1));
  ST7789_RST_LOW();
  vTaskDelay(pdMS_TO_TICKS(10));
  ST7789_RST_HIGH();
  vTaskDelay(pdMS_TO_TICKS(120));

  // 3. 初始化序列
  ST7789_CS_LOW();

  // 退出睡眠模式
  ST7789_WriteCmd(0x11);
  vTaskDelay(pdMS_TO_TICKS(120));

  // 4. 显示方向设置（使用0x70适配240×320竖屏）
  ST7789_WriteCmd(0x36);
  ST7789_WriteDataByte(0x70);  // 适配240×320竖屏

  // 5. 像素格式设置（保持16位RGB565）
  ST7789_WriteCmd(0x3A);
  ST7789_WriteDataByte(0x55);  // 16位RGB565
  vTaskDelay(pdMS_TO_TICKS(10));

  // 6. 帧率设置（添加对方的参数）
  ST7789_WriteCmd(0xB2);  // Porch Setting
  ST7789_WriteDataByte(0x0C);
  ST7789_WriteDataByte(0x0C);
  ST7789_WriteDataByte(0x00);
  ST7789_WriteDataByte(0x33);
  ST7789_WriteDataByte(0x33);

  ST7789_WriteCmd(0xB7);  // Gate Control
  ST7789_WriteDataByte(0x75);  // 对方使用的参数

  // 7. 电源参数设置（使用对方的参数）
  ST7789_WriteCmd(0xC2);  // VDV and VRH Command Enable
  ST7789_WriteDataByte(0x01);

  ST7789_WriteCmd(0xC3);  // VRH Set
  ST7789_WriteDataByte(0x16);  // 对方使用的参数

  ST7789_WriteCmd(0xC4);  // VDV Set
  ST7789_WriteDataByte(0x20);  // 对方使用的参数

  ST7789_WriteCmd(0xC6);  // Frame Rate Control
  ST7789_WriteDataByte(0x0F);  // 对方使用的参数

  ST7789_WriteCmd(0xD0);  // Power Control 1
  ST7789_WriteDataByte(0xA4);
  ST7789_WriteDataByte(0xA1);

  ST7789_WriteCmd(0xD6);
  ST7789_WriteDataByte(0xA1);

  ST7789_WriteCmd(0xBB);  // VCOM
  ST7789_WriteDataByte(0x3B);  // 对方使用的参数

  // 8. 伽马校正（使用对方的参数）
  ST7789_WriteCmd(0xE0);  // 正伽马校正
  uint8_t gamma_pos[] = {0xF0,0x0B,0x11,0x0E,0x0D,0x19,0x36,0x33,0x4B,0x07,0x14,0x14,0x2C,0x2E};
  ST7789_WriteData(gamma_pos, 14);

  ST7789_WriteCmd(0xE1);  // 负伽马校正
  uint8_t gamma_neg[] = {0xF0,0x0D,0x12,0x0B,0x09,0x03,0x32,0x44,0x48,0x39,0x16,0x16,0x2D,0x30};
  ST7789_WriteData(gamma_neg, 14);

  ST7789_WriteCmd(0xE4);
  ST7789_WriteDataByte(0x25);
  ST7789_WriteDataByte(0x00);
  ST7789_WriteDataByte(0x00);

  // 9. 开启显示
  ST7789_WriteCmd(0x29);
  ST7789_WriteCmd(0x2C);
  vTaskDelay(pdMS_TO_TICKS(10));

  // 10. 开启背光
  ST7789_BLK_LOW();  // 根据硬件确认是否需要HIGH

  ST7789_CS_HIGH();
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
  ST7789_CS_HIGH();   // 片选禁用
}

void ST7789_WriteDataByte(uint8_t data)
{
  ST7789_CS_LOW();    // 片选使能
  ST7789_DC_HIGH();   // 数据模式
  ST7789_SPI_WriteByte(data);
  ST7789_CS_HIGH();   // 片选禁用
}

//void ST7789_WriteData(uint8_t *data, uint32_t len)
//{
//  ST7789_CS_LOW();    // 片选使能
//  ST7789_DC_HIGH();   // 数据模式
//  for (uint32_t i = 0; i < len; i++)
//    {
//      ST7789_SPI_WriteByte(data[i]);
//    }
//  ST7789_CS_HIGH();   // 片选禁用
//}

// 优化版本（批量发送）
void ST7789_WriteData(uint8_t *data, uint32_t len)
{
  ST7789_CS_LOW();
  ST7789_DC_HIGH();

  // 等待SPI就绪
  while (!LL_SPI_IsActiveFlag_TXE(SPI1));

  // 批量发送数据
  for (uint32_t i = 0; i < len; i++)
    {
      LL_SPI_TransmitData8(SPI1, data[i]);
      // 减少等待，仅在缓冲区满时等待
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
 * @brief  设置显示窗口（绘图区域）
 * @param  x0,y0: 左上角坐标
 * @param  x1,y1: 右下角坐标
 * @param  dir_mode: 显示方向模式 (0-3)
 * @retval 无
 */
void ST7789_SetWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint8_t dir_mode)
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

void ST7789_Fill(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color, uint8_t dir_mode)
{
  // 计算总像素数（假设x0<=x1且y0<=y1，无检查）
  uint32_t pixel_num = (x1 - x0 + 1) * (y1 - y0 + 1);
  // 拆分颜色为高低字节（提前计算，避免循环内运算）
  uint8_t color_h = color >> 8;
  uint8_t color_l = color & 0xFF;

  // 1. 设置窗口（复用上面的极速窗口函数）
  ST7789_SetWindow(x0, y0, x1, y1, dir_mode);

  // 2. 连续发送颜色数据（直接操作SPI，避免ST7789_WriteData的函数调用开销）
  ST7789_CS_LOW();
  ST7789_DC_HIGH();  // 数据模式

  // 等待SPI就绪
  while (!LL_SPI_IsActiveFlag_TXE(SPI1));

  // 循环发送所有像素（每次发送2字节颜色，利用SPI缓冲区减少等待）
  for (uint32_t i = 0; i < pixel_num; i++)
    {
      // 发送高字节
      LL_SPI_TransmitData8(SPI1, color_h);
      while (i < pixel_num - 1 && !LL_SPI_IsActiveFlag_TXE(SPI1));

      // 发送低字节
      LL_SPI_TransmitData8(SPI1, color_l);
      while (i < pixel_num - 1 && !LL_SPI_IsActiveFlag_TXE(SPI1));
    }

  // 等待最后一笔数据发送完成
  while (LL_SPI_IsActiveFlag_BSY(SPI1));
  ST7789_CS_HIGH();
}

/**
 * @brief  清屏（填充单一颜色）
 * @param  color: 16位RGB565颜色值
 * @retval 无
 */
void ST7789_Clear(uint16_t color)
{
  ST7789_Fill(0, 0, ST7789_WIDTH - 1, ST7789_HEIGHT - 1, color, 2);
}



void vTFTTask(void *pvParameters)
{
  ST7789_Init();
  ST7789_Clear(RED);

  for (;;)
    {
      ST7789_Clear(GREEN);

      ST7789_Clear(BLUE);

      ST7789_Clear(YELLOW);
      vTaskDelay(pdMS_TO_TICKS(100));
    }
}
