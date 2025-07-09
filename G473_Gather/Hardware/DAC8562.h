/* DAC8562.h 文件 */
#ifndef __DAC8562_H
#define __DAC8562_H

#include "stm32g4xx_ll_spi.h"
#include "stm32g4xx_ll_gpio.h"
#include "stm32g4xx_ll_rcc.h"
#include "stm32g4xx_ll_system.h"
#include "stm32g4xx_ll_bus.h"

/* 引脚定义 */
#define DAC_SCLK_PIN        LL_GPIO_PIN_5
#define DAC_SCLK_PORT       GPIOA
#define DAC_MOSI_PIN        LL_GPIO_PIN_7
#define DAC_MOSI_PORT       GPIOA
#define DAC_SYNC_PIN        LL_GPIO_PIN_3
#define DAC_SYNC_PORT       GPIOA
#define DAC_LDAC_PIN        LL_GPIO_PIN_4
#define DAC_LDAC_PORT       GPIOA
#define DAC_CLR_PIN         LL_GPIO_PIN_6
#define DAC_CLR_PORT        GPIOA

/* DAC8562命令码 */
#define CMD_SETA_UPDATEA       0x18  // A通道命令+16位A路数据
#define CMD_SETB_UPDATEB       0x19  // B通道命令+16位B路数据
#define CMD_UPDATE_ALL_DACS    0x0F  // 更新两路寄存器命令
#define CMD_GAIN               0x02  // 内部放大倍数命令
#define CMD_PWR_UP_A_B         0x20  // 上电A、B路命令 
#define CMD_RESET_ALL_REG      0x28  // 所有寄存器复位命令
#define CMD_LDAC_DIS           0x30  // LDAC脚功能命令
#define CMD_INTERNAL_REF       0x38  // 内部参考使能/禁用命令 

/* 数据参数 */
#define DATA_GAIN_B2_A2     0x0000 // B路2倍，A路1倍
#define DATA_GAIN_B2_A1     0x0001 // B路1倍，A路2倍
#define DATA_GAIN_B1_A2     0x0002 // B路2倍，A路2倍
#define DATA_GAIN_B1_A1     0x0003 // B路1倍，A路1倍  
#define DATA_PWR_UP_A_B     0x0003 // Power up DAC-A and DAC-B	
#define DATA_RESET_ALL_REG  0x0001 // 所有寄存器复位、清空寄存器
#define DATA_LDAC_DIS       0x0003 // LDAC脚不起作用 
#define DATA_INTERNAL_REF_EN 0x0001 // 使能内部参考+双增益
#define DATA_INTERNAL_REF_DIS 0x0000 // 禁用内部参考 

/* DAC 通道地址 */
#define ADDR_CHANNEL_A 0x00 // 通道 A 
#define ADDR_CHANNEL_B 0x01 // 通道 B 
#define ADDR_BOTH_CHANNELS 0x07 // 双通道

void DAC8562_Init(void);
void DAC8562_SetVoltage(uint8_t channel, float voltage);

#endif /* __DAC8562_H */
