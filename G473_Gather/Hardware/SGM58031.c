#include "SGM58031.h"

void I2C1_Init(void)
{
  // 初始化I2C1 (PB7=SDA, PA15=SCL)
  LL_GPIO_InitTypeDef gpio_init = {0};

  // 使能时钟
  LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_GPIOB);
  LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_GPIOA);
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_I2C1);

  // 配置PB7 (SDA)
  gpio_init.Pin = LL_GPIO_PIN_7;
  gpio_init.Mode = LL_GPIO_MODE_ALTERNATE;
  gpio_init.Alternate = LL_GPIO_AF_4; // I2C1 AF
  gpio_init.Speed = LL_GPIO_SPEED_FREQ_HIGH;
  gpio_init.OutputType = LL_GPIO_OUTPUT_OPENDRAIN;
  gpio_init.Pull = LL_GPIO_PULL_UP;
  LL_GPIO_Init(GPIOB, &gpio_init);

  // 配置PA15 (SCL)
  gpio_init.Pin = LL_GPIO_PIN_15;
  gpio_init.Alternate = LL_GPIO_AF_5; // I2C1 AF
  LL_GPIO_Init(GPIOA, &gpio_init);

  // 配置I2C
  LL_I2C_DisableOwnAddress2(I2C1);
  LL_I2C_DisableGeneralCall(I2C1);
  LL_I2C_InitTypeDef I2C_InitStruct = {0};
  I2C_InitStruct.PeripheralMode = LL_I2C_MODE_I2C;
  I2C_InitStruct.Timing = 0x10C0ECFF; // 100kHz 标准模式
  I2C_InitStruct.AnalogFilter = LL_I2C_ANALOGFILTER_ENABLE;
  I2C_InitStruct.DigitalFilter = 0;
  I2C_InitStruct.OwnAddress1 = 0;
  I2C_InitStruct.TypeAcknowledge = LL_I2C_ACK;
  I2C_InitStruct.OwnAddrSize = LL_I2C_OWNADDRESS1_7BIT;
  LL_I2C_Init(I2C1, &I2C_InitStruct);
  LL_I2C_Enable(I2C1);

  LL_I2C_EnableClockStretching(I2C1);
  LL_I2C_DisableOwnAddress2(I2C1);
  LL_I2C_DisableGeneralCall(I2C1);

  // 清除所有标志
  LL_I2C_ClearFlag_ADDR(I2C1);
  LL_I2C_ClearFlag_STOP(I2C1);
}

// 增强型I2C写函数（支持16位数据）
ADC_StatusTypeDef I2C_Write16(I2C_TypeDef *I2Cx, uint8_t slave_addr, uint8_t reg_addr, uint16_t reg_data)
{
  uint8_t data[2] = {(uint8_t)(reg_data >> 8), (uint8_t)(reg_data & 0xFF)};

  // 使用7位地址（不需要左移）
  LL_I2C_HandleTransfer(I2Cx,
                        slave_addr,  // 7位地址直接使用
                        LL_I2C_ADDRSLAVE_7BIT,
                        3,  // 寄存器地址 + 2字节数据
                        LL_I2C_MODE_AUTOEND,
                        LL_I2C_GENERATE_START_WRITE);

  uint32_t timeout = I2C_TIMEOUT;

  // 发送寄存器地址
  while(!LL_I2C_IsActiveFlag_TXIS(I2Cx))
    {
      if(LL_I2C_IsActiveFlag_NACK(I2Cx))
        {
          LL_I2C_ClearFlag_NACK(I2Cx);
          return ADC_ERROR;
        }
      if(timeout-- == 0)//这里卡住了
			{
				Debug_printf("1");
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
					Debug_printf("12");
          return ADC_ERROR;
        }
      if(timeout-- == 0)
			{
				Debug_printf("2");
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
				Debug_printf("3");
				return ADC_TIMEOUT;
			}
    }
  LL_I2C_ClearFlag_STOP(I2Cx);

  return ADC_OK;
}

// 增强型I2C读函数（支持16位数据）
ADC_StatusTypeDef I2C_Read16(I2C_TypeDef *I2Cx, uint8_t slave_addr, uint8_t reg_addr, uint16_t *data)
{
  // 1. 发送寄存器地址（写操作）
  LL_I2C_HandleTransfer(I2Cx,
                        slave_addr,
                        LL_I2C_ADDRSLAVE_7BIT,
                        1,  // 只发送寄存器地址
                        LL_I2C_MODE_SOFTEND,
                        LL_I2C_GENERATE_RESTART_7BIT_READ);

  uint32_t timeout = I2C_TIMEOUT;

  // 等待发送寄存器地址
  while(!LL_I2C_IsActiveFlag_TXIS(I2Cx))
    {
      if(LL_I2C_IsActiveFlag_NACK(I2Cx))
        {
          LL_I2C_ClearFlag_NACK(I2Cx);
          LL_I2C_GenerateStopCondition(I2Cx);
          return ADC_ERROR;
        }
      if(timeout-- == 0)
        {
					Debug_printf("4");
          LL_I2C_GenerateStopCondition(I2Cx);
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
          LL_I2C_GenerateStopCondition(I2Cx);
          return ADC_ERROR;
        }
      if(timeout-- == 0)
        {
					Debug_printf("5");
          LL_I2C_GenerateStopCondition(I2Cx);
          return ADC_TIMEOUT;
        }
    }

  // 2. 读取数据（带重复起始条件）
  LL_I2C_HandleTransfer(I2Cx,
                        slave_addr,
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
				Debug_printf("6");
				return ADC_TIMEOUT;
			}
    }
  msb = LL_I2C_ReceiveData8(I2Cx);

  // 读取低字节前配置NACK
  LL_I2C_AcknowledgeNextData(I2Cx, LL_I2C_NACK);

  // 读取低字节
  timeout = I2C_TIMEOUT;
  while(!LL_I2C_IsActiveFlag_RXNE(I2Cx))
    {
      if(LL_I2C_IsActiveFlag_NACK(I2Cx))
        {
          LL_I2C_ClearFlag_NACK(I2Cx);
          return ADC_ERROR;
        }
      if(timeout-- == 0) return ADC_TIMEOUT;
    }
  lsb = LL_I2C_ReceiveData8(I2Cx);

  *data = (msb << 8) | lsb;

  // 等待传输完成
  timeout = I2C_TIMEOUT;
  while(!LL_I2C_IsActiveFlag_STOP(I2Cx))
    {
      if(timeout-- == 0)
			{
				Debug_printf("7");
				return ADC_TIMEOUT;
			}
    }
  LL_I2C_ClearFlag_STOP(I2Cx);

  // 恢复ACK设置
  LL_I2C_AcknowledgeNextData(I2Cx, LL_I2C_ACK);

  return ADC_OK;
}

// 初始化ADC
void SGM58031_Init(I2C_TypeDef *I2Cx)
{
//  // 配置连续转换模式
  // uint16_t config = SGM58031_CONFIG_OS_SINGLE |
  //                  SGM58031_CONFIG_MUX_CH0 |
  //                  SGM58031_CONFIG_PGA_4096 |
  //                  SGM58031_CONFIG_DR_100 |
  //                  0x0080; // 其他必要位
	    // 完整配置（使用通道0）
    uint16_t config = SGM58031_CONFIG_OS_SINGLE |
                     SGM58031_CONFIG_MUX_CH0 |    // AIN0-GND
                     SGM58031_CONFIG_PGA_4096 |
                     SGM58031_CONFIG_MODE_SINGLE | // 单次模式
                     SGM58031_CONFIG_DR_100 |
                     SGM58031_CONFIG_COMP_DISABLE;

//  uint16_t config = 0x8583;

  I2C_Write16(I2Cx, SGM58031_I2C_ADDR, SGM58031_REG_CONFIG, config);
}

// 只读取单个通道的函数
ADC_StatusTypeDef SGM58031_ReadChannel(I2C_TypeDef *I2Cx, uint8_t channel, int16_t *result)
{
  // 确保通道号在0-3范围内
  if(channel > 3)
    {
      return ADC_ERROR;
    }

  // 配置通道并启动转换
  uint16_t channel_config = 0x8583 & ~(0x7000);  // 清除MUX位
  channel_config |= (channel << 12);             // 设置通道选择位
  channel_config |= 0x8000;                      // 设置OS位启动转换

  ADC_StatusTypeDef status = I2C_Write16(I2Cx, SGM58031_I2C_ADDR,
                                         SGM58031_REG_CONFIG, channel_config);
  if(status != ADC_OK)
    {
			Debug_printf("77");
      return status;
    }

  // 等待转换完成 - 通过轮询状态寄存器
  uint32_t start_time = DWT_GetTick();
  uint16_t config_value;

  do
    {
      // 读取配置寄存器检查OS位
      status = I2C_Read16(I2Cx, SGM58031_I2C_ADDR,
                          SGM58031_REG_CONFIG, &config_value);
      if(status != ADC_OK)
        {
          return status;
        }

      // 检查超时（50ms）
      if(DWT_GetTick() - start_time > 50)
        {
          return ADC_TIMEOUT;
        }

    }
  while(!(config_value & 0x8000));    // 等待OS位变高（转换完成）

  // 读取转换结果
  uint16_t raw_data;
  status = I2C_Read16(I2Cx, SGM58031_I2C_ADDR,
                      SGM58031_REG_CONV, &raw_data);
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
    return I2C_Read16(I2Cx, SGM58031_I2C_ADDR, SGM58031_REG_CONFIG, config);
}
