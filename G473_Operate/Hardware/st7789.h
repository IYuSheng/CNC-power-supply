/* st7789.h 文件 */
#ifndef __ST7789_H
#define __ST7789_H

#include "main.h"
#include <stdint.h>
#include "stm32g4xx_ll_spi.h"

// 屏幕分辨率定义
#define ST7789_WIDTH  320
#define ST7789_HEIGHT 240

// GPIO控制宏定义
#define ST7789_CS_LOW()    LL_GPIO_ResetOutputPin(GPIOA, LL_GPIO_PIN_4)    // PA4: 片选信号(低电平有效)
#define ST7789_CS_HIGH()   LL_GPIO_SetOutputPin(GPIOA, LL_GPIO_PIN_4)      // PA4: 片选信号(高电平无效)
#define ST7789_DC_LOW()    LL_GPIO_ResetOutputPin(GPIOA, LL_GPIO_PIN_3)    // PA3: 数据/命令选择(0=命令,1=数据)
#define ST7789_DC_HIGH()   LL_GPIO_SetOutputPin(GPIOA, LL_GPIO_PIN_3)      // PA3: 数据/命令选择(0=命令,1=数据)
#define ST7789_RST_HIGH()  LL_GPIO_SetOutputPin(GPIOA, LL_GPIO_PIN_2)      // PA2: 复位信号(高电平正常)
#define ST7789_RST_LOW()   LL_GPIO_ResetOutputPin(GPIOA, LL_GPIO_PIN_2)    // PA2: 复位信号(低电平关闭)
#define ST7789_BLK_LOW()   LL_GPIO_ResetOutputPin(GPIOB, LL_GPIO_PIN_0)    // PB0: 背光控制(低电平开启)
#define ST7789_BLK_HIGH()  LL_GPIO_SetOutputPin(GPIOB, LL_GPIO_PIN_0)      // PB0: 背光控制(高电平关闭)

// 确保使用正确的颜色格式 (RGB565)
#define RGB565(r, g, b) (((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3))

// 常用颜色定义
#define RED     RGB565(255, 0, 0)
#define GREEN   RGB565(0, 255, 0)
#define BLUE    RGB565(0, 0, 255)
#define YELLOW  RGB565(255, 255, 0)
#define CYAN    RGB565(0, 255, 255)
#define MAGENTA RGB565(255, 0, 255)
#define WHITE   RGB565(255, 255, 255)
#define BLACK   RGB565(0, 0, 0)

// 屏幕初始化函数
void ST7789_Init(void);

// 底层通信函数
void ST7789_WriteCmd(uint8_t cmd);                                               // 写命令
void ST7789_WriteData(uint8_t *data, uint32_t len);                        // 写多字节数据
void ST7789_WriteDataByte(uint8_t data);                                   // 写单字节数据

void ST7789_DMA_Init(void);
void ST7789_WriteDataDMA(uint16_t *data, uint32_t len);

void vTFTTask(void *pvParameters);

#endif /* __ST7789_H */
