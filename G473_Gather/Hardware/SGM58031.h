#ifndef __SGM58031_H
#define __SGM58031_H

#include "stm32g4xx_ll_gpio.h"
#include "stm32g4xx_ll_i2c.h"
#include "stm32g4xx_ll_bus.h"
#include "stm32g4xx_ll_system.h"
#include "Uart_Debug.h"
#include "DWT_Delay.h"

// 超时设置
#define I2C_TIMEOUT 100000

typedef struct
{
  int16_t ch0_value;  // 通道0的ADC值
  int16_t ch1_value;  // 通道1的ADC值
  int16_t ch2_value;  // 通道2的ADC值
  int16_t ch3_value;  // 通道3的ADC值
} ADC_Results;

// ADC转换状态类型
typedef enum
{
  ADC_OK,
  ADC_ERROR,
  ADC_TIMEOUT
} ADC_StatusTypeDef;

// I2C 地址配置 (ADDR引脚决定)
#define SGM58031_I2C_ADDR    0x48  // 默认地址 (ADDR接地)

// ========== 寄存器地址定义 ==========
#define SGM58031_REG_CONV        0x00  // 转换结果寄存器
#define SGM58031_REG_CONFIG      0x01  // 配置寄存器
#define SGM58031_REG_LOW_THRESH  0x02  // 低阈值寄存器
#define SGM58031_REG_HIGH_THRESH 0x03  // 高阈值寄存器
#define SGM58031_REG_CONFIG1     0x04  // 配置寄存器1
#define SGM58031_REG_CHIP_ID     0x05  // 芯片ID寄存器
#define SGM58031_REG_GN_TRIM1    0x06  // 外部参考增益修正寄存器

// ========== 配置寄存器设置 ==========
#define SGM58031_CONFIG_OS_SINGLE    0x8000  // 单次转换模式

#define SGM58031_CONFIG_MUX_CH0      0x4000  // 通道0 (AINP=AIN0, AINN=GND)
#define SGM58031_CONFIG_MUX_CH1      0x5000  // 通道1 (AINP=AIN1, AINN=GND)
#define SGM58031_CONFIG_MUX_CH2      0x6000  // 通道2 (AINP=AIN2, AINN=GND)
#define SGM58031_CONFIG_MUX_CH3      0x7000  // 通道3 (AINP=AIN3, AINN=GND)

#define SGM58031_CONFIG_PGA_6144    0x0000  // ±6.144V (增益=2/3)
#define SGM58031_CONFIG_PGA_4096    0x0200  // ±4.096V (增益=1)
#define SGM58031_CONFIG_PGA_2048    0x0400  // ±2.048V (增益=2)
#define SGM58031_CONFIG_PGA_1024    0x0600  // ±1.024V (增益=4)
#define SGM58031_CONFIG_PGA_0512    0x0800  // ±0.512V (增益=8)
#define SGM58031_CONFIG_PGA_0256    0x0A00  // ±0.256V (增益=16)

#define SGM58031_CONFIG_MODE_CONT    0x0000  // 连续转换模式
#define SGM58031_CONFIG_MODE_SINGLE   0x0100  // 单次转换模式
#define SGM58031_CONFIG_COMP_DISABLE  0x0003  // 禁用比较器
#define SGM58031_CONFIG_DEFAULT       0x8583  // 默认配置

// ========== 数据速率配置 (DR[2:0] bits at D[7:5]) ==========
// DR_SEL=0时的标准速率
#define SGM58031_CONFIG_DR_6_25      0x0000  // 6.25SPS   (DR[2:0]=000)
#define SGM58031_CONFIG_DR_12_5      0x0020  // 12.5SPS   (DR[2:0]=001)
#define SGM58031_CONFIG_DR_25        0x0040  // 25SPS     (DR[2:0]=010)
#define SGM58031_CONFIG_DR_50        0x0060  // 50SPS     (DR[2:0]=011)
#define SGM58031_CONFIG_DR_100       0x0080  // 100SPS    (DR[2:0]=100)
#define SGM58031_CONFIG_DR_200       0x00A0  // 200SPS    (DR[2:0]=101)
#define SGM58031_CONFIG_DR_400       0x00C0  // 400SPS    (DR[2:0]=110)
#define SGM58031_CONFIG_DR_800       0x00E0  // 800SPS    (DR[2:0]=111)

// DR_SEL=1时的扩展速率 (需要在Config1寄存器中设置DR_SEL=1)
#define SGM58031_CONFIG_DR_7_5       0x0000  // 7.5SPS    (DR[2:0]=000, DR_SEL=1)
#define SGM58031_CONFIG_DR_15        0x0020  // 15SPS     (DR[2:0]=001, DR_SEL=1)
#define SGM58031_CONFIG_DR_30        0x0040  // 30SPS     (DR[2:0]=010, DR_SEL=1)
#define SGM58031_CONFIG_DR_60        0x0060  // 60SPS     (DR[2:0]=011, DR_SEL=1)
#define SGM58031_CONFIG_DR_120       0x0080  // 120SPS    (DR[2:0]=100, DR_SEL=1)
#define SGM58031_CONFIG_DR_240       0x00A0  // 240SPS    (DR[2:0]=101, DR_SEL=1)
#define SGM58031_CONFIG_DR_480       0x00C0  // 480SPS    (DR[2:0]=110, DR_SEL=1)
#define SGM58031_CONFIG_DR_960       0x00E0  // 960SPS    (DR[2:0]=111, DR_SEL=1)

// Config1寄存器中的DR_SEL位，此处需要配置config1寄存器
#define SGM58031_CONFIG1_DR_SEL      0x0080  // D[7] 扩展速率选择

// ========== 函数声明 ==========
void I2C1_Init(void);
void SGM58031_Init(I2C_TypeDef *I2Cx);
ADC_StatusTypeDef I2C_Write16(I2C_TypeDef *I2Cx, uint8_t slave_addr, uint8_t reg_addr, uint16_t reg_data);
ADC_StatusTypeDef I2C_Read16(I2C_TypeDef *I2Cx, uint8_t slave_addr, uint8_t reg_addr, uint16_t *data);
ADC_StatusTypeDef SGM58031_ReadAllChannels(I2C_TypeDef *I2Cx, int16_t results[4]);
ADC_StatusTypeDef SGM58031_ReadChannel(I2C_TypeDef *I2Cx, uint8_t channel, int16_t *result);
ADC_StatusTypeDef SGM58031_ReadConfig(I2C_TypeDef *I2Cx, uint16_t *config);
float SGM58031_ConvertToVoltage(int16_t adc_raw, uint16_t pga_config);
ADC_StatusTypeDef SGM58031_ReadVoltage(I2C_TypeDef *I2Cx, uint8_t channel, float *voltage);

#endif /* __SGM58031_H */
