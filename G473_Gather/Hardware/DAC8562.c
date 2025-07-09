#include "DAC8562.h"

/* 复位 DAC8562 */
static void DAC8562_Reset(void)
{
  LL_GPIO_ResetOutputPin(DAC_CLR_PORT, DAC_CLR_PIN);
  for(volatile uint32_t i = 0; i < 1000; i++);
  LL_GPIO_SetOutputPin(DAC_CLR_PORT, DAC_CLR_PIN);
  for(volatile uint32_t i = 0; i < 1000; i++);
}

/* 发送命令到 DAC8562 */
static void DAC8562_SendCommand(uint8_t* data, uint8_t size)
{
  LL_GPIO_ResetOutputPin(DAC_SYNC_PORT, DAC_SYNC_PIN);
  for(uint8_t i = 0; i < size; i++)
    {
      while(!LL_SPI_IsActiveFlag_TXE(SPI1));
      LL_SPI_TransmitData8(SPI1, data[i]);
    }
  while (LL_SPI_IsActiveFlag_BSY(SPI1));
  LL_GPIO_SetOutputPin(DAC_SYNC_PORT, DAC_SYNC_PIN);
}

/* 设置 DAC 通道输出电压 */
void DAC8562_SetVoltage(uint8_t channel, float voltage)
{
  uint16_t dacValue = (uint16_t)((voltage / 2.5f) * 65535.0f);
  if (dacValue > 65535) dacValue = 65535;

  uint8_t cmd[3];
  if(channel == ADDR_CHANNEL_A)
    {
      cmd[0] = CMD_SETA_UPDATEA;
    }
  else if(channel == ADDR_CHANNEL_B)
    {
      cmd[0] = CMD_SETB_UPDATEB;
    }
  else
    {
      return; // 无效通道
    }
  cmd[1] = (uint8_t)(dacValue >> 8);
  cmd[2] = (uint8_t)(dacValue & 0xFF);
  DAC8562_SendCommand(cmd, 3);
}

/* 设置增益 */
static void DAC8562_WriteGain(uint16_t gain_data)
{
  uint8_t cmd[3] = {CMD_GAIN, (uint8_t)(gain_data >> 8), (uint8_t)(gain_data & 0xFF)};
  DAC8562_SendCommand(cmd, 3);
}

/* 初始化 SPI1 和 GPIO */
void DAC8562_Init(void)
{
  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_SPI1);
  LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_GPIOA);

  // SPI引脚配置
  LL_GPIO_SetPinMode(DAC_SCLK_PORT, DAC_SCLK_PIN, LL_GPIO_MODE_ALTERNATE);
  LL_GPIO_SetPinMode(DAC_MOSI_PORT, DAC_MOSI_PIN, LL_GPIO_MODE_ALTERNATE);
  LL_GPIO_SetAFPin_0_7(DAC_SCLK_PORT, DAC_SCLK_PIN, LL_GPIO_AF_5);
  LL_GPIO_SetAFPin_0_7(DAC_MOSI_PORT, DAC_MOSI_PIN, LL_GPIO_AF_5);
  LL_GPIO_SetPinSpeed(DAC_SCLK_PORT, DAC_SCLK_PIN, LL_GPIO_SPEED_FREQ_VERY_HIGH);
  LL_GPIO_SetPinSpeed(DAC_MOSI_PORT, DAC_MOSI_PIN, LL_GPIO_SPEED_FREQ_VERY_HIGH);
  LL_GPIO_SetPinOutputType(DAC_SCLK_PORT, DAC_SCLK_PIN, LL_GPIO_OUTPUT_PUSHPULL);
  LL_GPIO_SetPinOutputType(DAC_MOSI_PORT, DAC_MOSI_PIN, LL_GPIO_OUTPUT_PUSHPULL);
  LL_GPIO_SetPinPull(DAC_SCLK_PORT, DAC_SCLK_PIN, LL_GPIO_PULL_NO);
  LL_GPIO_SetPinPull(DAC_MOSI_PORT, DAC_MOSI_PIN, LL_GPIO_PULL_NO);

  // 控制引脚配置
  LL_GPIO_SetPinMode(DAC_SYNC_PORT, DAC_SYNC_PIN, LL_GPIO_MODE_OUTPUT);
  LL_GPIO_SetPinMode(DAC_LDAC_PORT, DAC_LDAC_PIN, LL_GPIO_MODE_OUTPUT);
  LL_GPIO_SetPinMode(DAC_CLR_PORT, DAC_CLR_PIN, LL_GPIO_MODE_OUTPUT);
  LL_GPIO_SetPinSpeed(DAC_SYNC_PORT, DAC_SYNC_PIN, LL_GPIO_SPEED_FREQ_VERY_HIGH);
  LL_GPIO_SetPinSpeed(DAC_LDAC_PORT, DAC_LDAC_PIN, LL_GPIO_SPEED_FREQ_VERY_HIGH);
  LL_GPIO_SetPinSpeed(DAC_CLR_PORT, DAC_CLR_PIN, LL_GPIO_SPEED_FREQ_VERY_HIGH);
  LL_GPIO_SetPinOutputType(DAC_SYNC_PORT, DAC_SYNC_PIN, LL_GPIO_OUTPUT_PUSHPULL);
  LL_GPIO_SetPinOutputType(DAC_LDAC_PORT, DAC_LDAC_PIN, LL_GPIO_OUTPUT_PUSHPULL);
  LL_GPIO_SetPinOutputType(DAC_CLR_PORT, DAC_CLR_PIN, LL_GPIO_OUTPUT_PUSHPULL);
  LL_GPIO_SetOutputPin(DAC_SYNC_PORT, DAC_SYNC_PIN);
  LL_GPIO_SetOutputPin(DAC_LDAC_PORT, DAC_LDAC_PIN);
  LL_GPIO_SetOutputPin(DAC_CLR_PORT, DAC_CLR_PIN);

  // SPI配置
  LL_SPI_Disable(SPI1);
  LL_SPI_SetTransferDirection(SPI1, LL_SPI_HALF_DUPLEX_TX);
  LL_SPI_SetMode(SPI1, LL_SPI_MODE_MASTER);
  LL_SPI_SetDataWidth(SPI1, LL_SPI_DATAWIDTH_8BIT);
  LL_SPI_SetClockPolarity(SPI1, LL_SPI_POLARITY_LOW);
  LL_SPI_SetClockPhase(SPI1, LL_SPI_PHASE_1EDGE);
  LL_SPI_SetNSSMode(SPI1, LL_SPI_NSS_SOFT);
  LL_SPI_SetBaudRatePrescaler(SPI1, LL_SPI_BAUDRATEPRESCALER_DIV8);
  LL_SPI_SetTransferBitOrder(SPI1, LL_SPI_MSB_FIRST);
  LL_SPI_Enable(SPI1);

  // 复位 DAC
  DAC8562_Reset();

  // 初始化命令流程
  // 1. 上电A、B路
  uint8_t cmd_pwr[3] = {CMD_PWR_UP_A_B, (uint8_t)(DATA_PWR_UP_A_B >> 8), (uint8_t)(DATA_PWR_UP_A_B & 0xFF)};
  DAC8562_SendCommand(cmd_pwr, 3);

  // 2. 所有寄存器复位
  uint8_t cmd_reset[3] = {CMD_RESET_ALL_REG, (uint8_t)(DATA_RESET_ALL_REG >> 8), (uint8_t)(DATA_RESET_ALL_REG & 0xFF)};
  DAC8562_SendCommand(cmd_reset, 3);

  // 3. LDAC脚功能配置
  uint8_t cmd_ldac[3] = {CMD_LDAC_DIS, (uint8_t)(DATA_LDAC_DIS >> 8), (uint8_t)(DATA_LDAC_DIS & 0xFF)};
  DAC8562_SendCommand(cmd_ldac, 3);

  // 4. 使能内部参考+双增益
  uint8_t cmd_ref[3] = {CMD_INTERNAL_REF, (uint8_t)(DATA_INTERNAL_REF_EN >> 8), (uint8_t)(DATA_INTERNAL_REF_EN & 0xFF)};
  DAC8562_SendCommand(cmd_ref, 3);

  // 5. 设置增益（默认两倍增益,此处设1倍）
  DAC8562_WriteGain(DATA_GAIN_B1_A1);
}
