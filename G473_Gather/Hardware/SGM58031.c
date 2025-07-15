#include "SGM58031.h"

void I2C1_Init(void)
{
  LL_I2C_InitTypeDef I2C_InitStruct = {0};

  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

  LL_RCC_SetI2CClockSource(LL_RCC_I2C1_CLKSOURCE_PCLK1);

  LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_GPIOA);
  LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_GPIOB);
  /**I2C1 GPIO Configuration
  PA15   ------> I2C1_SCL
  PB7   ------> I2C1_SDA
  */
  GPIO_InitStruct.Pin = LL_GPIO_PIN_15;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_OPENDRAIN;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_UP;
  GPIO_InitStruct.Alternate = LL_GPIO_AF_4;
  LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = LL_GPIO_PIN_7;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_OPENDRAIN;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_UP;
  GPIO_InitStruct.Alternate = LL_GPIO_AF_4;
  LL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* Peripheral clock enable */
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_I2C1);

  I2C_InitStruct.PeripheralMode = LL_I2C_MODE_I2C;
  I2C_InitStruct.Timing = 0x40B285C2;
  I2C_InitStruct.AnalogFilter = LL_I2C_ANALOGFILTER_ENABLE;
  I2C_InitStruct.DigitalFilter = 0;
  I2C_InitStruct.OwnAddress1 = 0;
  I2C_InitStruct.TypeAcknowledge = LL_I2C_ACK;
  I2C_InitStruct.OwnAddrSize = LL_I2C_OWNADDRESS1_7BIT;
  LL_I2C_Init(I2C1, &I2C_InitStruct);
  LL_I2C_EnableAutoEndMode(I2C1);
  LL_I2C_SetOwnAddress2(I2C1, 0, LL_I2C_OWNADDRESS2_NOMASK);
  LL_I2C_DisableOwnAddress2(I2C1);
  LL_I2C_DisableGeneralCall(I2C1);
  LL_I2C_EnableClockStretching(I2C1);

  LL_I2C_ClearFlag_ADDR(I2C1);
  LL_I2C_ClearFlag_STOP(I2C1);
  LL_I2C_ClearFlag_NACK(I2C1);

}

// 增强型I2C写函数（支持16位数据）
ADC_StatusTypeDef I2C_Write16(I2C_TypeDef *I2Cx, uint8_t slave_addr, uint8_t reg_addr, uint16_t reg_data)
{
  uint8_t data[2] = {(uint8_t)(reg_data >> 8), (uint8_t)(reg_data & 0xFF)};
  uint32_t timeout = I2C_TIMEOUT;

  while(LL_I2C_IsActiveFlag_BUSY(I2Cx))
    {
      if(timeout-- == 0)
        {
          return ADC_TIMEOUT;
        }
    }

  if ((I2Cx->CR1 & I2C_CR1_PE) != I2C_CR1_PE)
    {
      /* Enable I2C peripheral */
      LL_I2C_Enable(I2Cx);
    }

  // 使用7位地址（不需要左移）
  LL_I2C_HandleTransfer(I2Cx,
                        slave_addr<<1,  // 7位地址直接使用
                        LL_I2C_ADDRSLAVE_7BIT,
                        3,  // 寄存器地址 + 2字节数据
                        LL_I2C_MODE_AUTOEND,
                        LL_I2C_GENERATE_START_WRITE);

  timeout = I2C_TIMEOUT;
  // 发送寄存器地址
  while(!LL_I2C_IsActiveFlag_TXIS(I2Cx))
    {
      if(LL_I2C_IsActiveFlag_NACK(I2Cx))
        {
          LL_I2C_ClearFlag_NACK(I2Cx);
          return ADC_ERROR;
        }

      if(timeout-- == 0)
        {
          return ADC_TIMEOUT;
        }
    }

  LL_I2C_TransmitData8(I2Cx, reg_addr);
  // 发送高字节
  timeout = I2C_TIMEOUT;
  while(!LL_I2C_IsActiveFlag_TXIS(I2Cx))
    {
      if(LL_I2C_IsActiveFlag_NACK(I2Cx))
        {
          LL_I2C_ClearFlag_NACK(I2Cx);
          return ADC_ERROR;
        }
      if(timeout-- == 0)
        {
          return ADC_TIMEOUT;
        }
    }
  LL_I2C_TransmitData8(I2Cx, data[0]);

  // 发送低字节
  timeout = I2C_TIMEOUT;
  while(!LL_I2C_IsActiveFlag_TXIS(I2Cx))
    {
      if(LL_I2C_IsActiveFlag_NACK(I2Cx))
        {
          LL_I2C_ClearFlag_NACK(I2Cx);
          return ADC_ERROR;
        }
      if(timeout-- == 0) return ADC_TIMEOUT;
    }
  LL_I2C_TransmitData8(I2Cx, data[1]);

  // 等待传输完成
  timeout = I2C_TIMEOUT;
  while(!LL_I2C_IsActiveFlag_STOP(I2Cx))
    {
      if(timeout-- == 0)
        {
          return ADC_TIMEOUT;
        }
    }
  LL_I2C_ClearFlag_STOP(I2Cx);

  return ADC_OK;
}

// 增强型I2C读函数（支持16位数据）
ADC_StatusTypeDef I2C_Read16(I2C_TypeDef *I2Cx, uint8_t slave_addr, uint8_t reg_addr, uint16_t *data)
{
  uint32_t timeout = I2C_TIMEOUT;
  *data = 0;
  while(LL_I2C_IsActiveFlag_BUSY(I2Cx))

    {
      if(timeout-- == 0)
        {
          return ADC_TIMEOUT;
        }
    }

  // 1. 发送寄存器地址（写操作）
  LL_I2C_HandleTransfer(I2Cx,
                        slave_addr<<1,
                        LL_I2C_ADDRSLAVE_7BIT,
                        1,  // 只发送寄存器地址
                        LL_I2C_MODE_SOFTEND,
                        LL_I2C_GENERATE_START_WRITE);

  timeout = I2C_TIMEOUT;
  // 等待发送寄存器地址
  while(!LL_I2C_IsActiveFlag_TXIS(I2Cx))
    {
      if(LL_I2C_IsActiveFlag_NACK(I2Cx))
        {
          LL_I2C_ClearFlag_NACK(I2Cx);
          return ADC_ERROR;
        }
      if(timeout-- == 0)
        {
          return ADC_TIMEOUT;
        }
    }
  LL_I2C_TransmitData8(I2Cx, reg_addr);

  // 等待地址发送完成
  timeout = I2C_TIMEOUT;
  while(!LL_I2C_IsActiveFlag_TC(I2Cx))
    {
      if(LL_I2C_IsActiveFlag_NACK(I2Cx))
        {
          LL_I2C_ClearFlag_NACK(I2Cx);
          return ADC_ERROR;
        }
      if(timeout-- == 0)
        {
          return ADC_TIMEOUT;
        }
    }

  // 2. 读取数据（带重复起始条件）
  LL_I2C_HandleTransfer(I2Cx,
                        slave_addr<<1,
                        LL_I2C_ADDRSLAVE_7BIT,
                        2,  // 读取2字节
                        LL_I2C_MODE_AUTOEND,
                        LL_I2C_GENERATE_START_READ);

  uint8_t msb = 0, lsb = 0;

  // 读取高字节
  timeout = I2C_TIMEOUT;
  while(!LL_I2C_IsActiveFlag_RXNE(I2Cx))
    {
      if(LL_I2C_IsActiveFlag_NACK(I2Cx))
        {
          LL_I2C_ClearFlag_NACK(I2Cx);
          return ADC_ERROR;
        }
      if(timeout-- == 0)
        {
          return ADC_TIMEOUT;
        }
    }

  msb = LL_I2C_ReceiveData8(I2Cx);

  // 读取低字节
  timeout = I2C_TIMEOUT;
  while(!LL_I2C_IsActiveFlag_RXNE(I2Cx))
    {
      if(LL_I2C_IsActiveFlag_NACK(I2Cx))
        {
          LL_I2C_ClearFlag_NACK(I2Cx);
          return ADC_ERROR;
        }
      if(timeout-- == 0)
        {
          return ADC_TIMEOUT;
        }
    }

  lsb = LL_I2C_ReceiveData8(I2Cx);

  // 等待传输完成
  timeout = I2C_TIMEOUT;
  while(!LL_I2C_IsActiveFlag_STOP(I2Cx))
    {
      if(timeout-- == 0)
        {
          return ADC_TIMEOUT;
        }
    }
  LL_I2C_ClearFlag_STOP(I2Cx);

  *data = (msb << 8) | lsb;

  return ADC_OK;
}

// 初始化ADC
void SGM58031_Init(I2C_TypeDef *I2Cx)
{
  // 配置寄存器
  uint16_t config = SGM58031_CONFIG_OS_SINGLE |      // 0x8000 - 启动单次转换
                    SGM58031_CONFIG_MUX_CH0 |         // 0x4000 - AIN0-GND单端
                    SGM58031_CONFIG_PGA_4096 |        // 0x0200 - ±4.096V量程
                    SGM58031_CONFIG_MODE_SINGLE |     // 0x0100 - 单次模式
                    SGM58031_CONFIG_DR_100 |          // 0x00C0 - 400SPS
                    SGM58031_CONFIG_COMP_DISABLE;     // 0x0003 - 禁用比较器

  I2C_Write16(I2Cx, SGM58031_I2C_ADDR, SGM58031_REG_CONFIG, config);

  // 配置config1寄存器(待完善)
  //uint16_t config1 = SGM58031_CONFIG1_DR_SEL;

}

ADC_StatusTypeDef SGM58031_ReadChannel(I2C_TypeDef *I2Cx, uint8_t channel, int16_t *result)
{
  if(channel > 4)
    {
      *result = 0x8000; // 无效通道标记
      return ADC_ERROR;
    }

  // 配置为单端模式，明确设置PGA为±4.096V
  uint16_t channel_config = SGM58031_CONFIG_OS_SINGLE |      // 启动单次转换
                            SGM58031_CONFIG_MODE_SINGLE |     // 单次模式
                            SGM58031_CONFIG_PGA_4096 |        // ±4.096V量程
                            SGM58031_CONFIG_DR_800 |          // 采样率
                            ((channel + 4) << 12);            // 设置通道（单端）

  // 写入配置寄存器
  ADC_StatusTypeDef status = I2C_Write16(I2Cx, SGM58031_I2C_ADDR,
                                         SGM58031_REG_CONFIG, channel_config);
  if(status != ADC_OK)
    {
      *result = 0x8000; // 写失败：标记无效
      return status;
    }

  // 增加通道切换延时
  DWT_Delayms(10);  // 等待10ms确保通道切换完成

  // 读取转换结果
  uint16_t raw_data;
  status = I2C_Read16(I2Cx, SGM58031_I2C_ADDR, SGM58031_REG_CONV, &raw_data);

  if(status != ADC_OK)
    {
      return status;
    }

  *result = (int16_t)raw_data;

  return ADC_OK;
}

// 优化读取所有通道的函数，使用单通道读取
ADC_StatusTypeDef SGM58031_ReadAllChannels(I2C_TypeDef *I2Cx, int16_t results[4])
{
  for(uint8_t ch = 0; ch < 4; ch++)
    {
      ADC_StatusTypeDef status = SGM58031_ReadChannel(I2Cx, ch, &results[ch]);
      if(status != ADC_OK)
        {
          return status;
        }
    }
  return ADC_OK;
}

ADC_StatusTypeDef SGM58031_ReadConfig(I2C_TypeDef *I2Cx, uint16_t *config)
{
  return I2C_Read16(I2Cx, SGM58031_I2C_ADDR<<1, SGM58031_REG_CONFIG, config);
}

// 根据PGA设置转换ADC值为电压
float SGM58031_ConvertToVoltage(int16_t adc_raw, uint16_t pga_config)
{
  // 校验数据有效性（无效数据返回NaN，便于上层识别）
  if(adc_raw < -32768 || adc_raw > 32767)
    {
      return NAN; // 非数字，表示无效值
    }

  float fsr; // Full Scale Range

  switch(pga_config)
    {
    case SGM58031_CONFIG_PGA_6144:
      fsr = 6.144f;
      break;
    case SGM58031_CONFIG_PGA_4096:
      fsr = 4.096f;
      break;
    case SGM58031_CONFIG_PGA_2048:
      fsr = 2.048f;
      break;

    case SGM58031_CONFIG_PGA_1024:
      fsr = 1.024f;
      break;
    case SGM58031_CONFIG_PGA_0512:
      fsr = 0.512f;
      break;
    case SGM58031_CONFIG_PGA_0256:
      fsr = 0.256f;
      break;
    default:
      fsr = 4.096f;
      break;
    }

  return (float)adc_raw * fsr / 32768.0f;
}

// 简化版本（针对您当前的4.096V设置）
float SGM58031_ConvertToVoltage_4096V(int16_t adc_raw)
{
  return (float)adc_raw * 4.096f / 32768.0f;
}

// 读取ADC值并转换为电压
ADC_StatusTypeDef SGM58031_ReadVoltage(I2C_TypeDef *I2Cx, uint8_t channel, float *voltage)
{
  int16_t adc_raw;
  ADC_StatusTypeDef status = SGM58031_ReadChannel(I2Cx, channel, &adc_raw);

  if(status != ADC_OK)
    {
      *voltage = NAN;
      //Debug_printf("Channel %d: ADC通信失败（无设备响应）", channel);
      return status;
    }

  // 转换为电压值（使用4.096V满量程）
  *voltage = SGM58031_ConvertToVoltage_4096V(adc_raw);

  if(isnan(*voltage))
    {
      Debug_printf("Channel %d: ADC数据无效（原始值超出范围）", channel);
    }
  else
    {
      Debug_printf("Channel %d: Raw=0x%04X, Voltage=%.4fV", channel, adc_raw, *voltage);
    }

  return ADC_OK;
}
