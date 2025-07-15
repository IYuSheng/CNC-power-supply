#include "m24c64.h"

// 超时设置（毫秒）
#define M24C64_TIMEOUT      10      // 一般操作超时
#define M24C64_WRITE_TIMEOUT 5      // 写入操作超时

/**
 * @brief  初始化M24C64（检查设备连接）
 * @retval M24C64_Status_t: 状态码
 */
M24C64_Status_t M24C64_Init(void)
{
  return M24C64_CheckReady();  // 通过检查就绪状态验证连接
}

/**
 * @brief  读取单个字节
 * @param  addr: 存储地址（0~8191）
 * @param  data: 读取数据的指针
 * @retval M24C64_Status_t: 状态码
 */
M24C64_Status_t M24C64_ReadByte(uint16_t addr, uint8_t *data)
{
  return M24C64_Read(addr, data, 1);
}

/**
 * @brief  写入单个字节
 * @param  addr: 存储地址（0~8191）
 * @param  data: 要写入的数据
 * @retval M24C64_Status_t: 状态码
 */
M24C64_Status_t M24C64_WriteByte(uint16_t addr, uint8_t data)
{
  return M24C64_PageWrite(addr, &data, 1);
}

/**
 * @brief  页写入（一次最多写入一个页，32字节）
 * @param  addr: 存储地址（需页对齐，即addr % 32 == 0）
 * @param  data: 要写入的数据缓冲区
 * @param  len: 数据长度（最大32字节，且addr+len不能跨页）
 * @retval M24C64_Status_t: 状态码
 */
M24C64_Status_t M24C64_PageWrite(uint16_t addr, const uint8_t *data, uint16_t len)
{
  // 检查参数合法性
  if (addr >= M24C64_SIZE || len == 0 || len > M24C64_PAGE_SIZE)
    {
      return M24C64_ERROR_PARAM;
    }// 检查是否页对齐（避免跨页写入）
  if ((addr % M24C64_PAGE_SIZE) != 0 || ((addr + len) > ((addr / M24C64_PAGE_SIZE + 1) * M24C64_PAGE_SIZE)))
    {
      return M24C64_ERROR_PARAM;
    }

  // 检查设备是否就绪
  M24C64_Status_t status = M24C64_CheckReady();
  if (status != M24C64_OK)
    {
      return status;
    }

  // 构建写入数据（地址+数据）
  uint8_t buffer[2 + M24C64_PAGE_SIZE];
  buffer[0] = (addr >> 8) & 0xFF;  // 高地址字节
  buffer[1] = addr & 0xFF;         // 低地址字节
  for (uint16_t i = 0; i < len; i++)
    {
      buffer[2 + i] = data[i];
    }
  // I2C发送（带自动结束位）
  uint32_t timeout = M24C64_TIMEOUT;
  while (!LL_I2C_IsActiveFlag_TXE(I2C1) && timeout-- > 0);
  if (timeout == 0) return M24C64_ERROR_TIMEOUT;

  // 发送开始位+设备地址（写）
  LL_I2C_HandleTransfer(I2C1, M24C64_DEVICE_ADDR << 1, LL_I2C_ADDRSLAVE_7BIT,
                        2 + len, LL_I2C_MODE_AUTOEND, LL_I2C_GENERATE_START_WRITE);

  // 发送数据
  for (uint16_t i = 0; i < (2 + len); i++)
    {
      timeout = M24C64_TIMEOUT;
      while (!LL_I2C_IsActiveFlag_TXIS(I2C1) && timeout-- > 0);
      if (timeout == 0) return M24C64_ERROR_TIMEOUT;

      LL_I2C_TransmitData8(I2C1, buffer[i]);
    }// 等待传输完成（自动结束模式会自动发送停止位）
  timeout = M24C64_TIMEOUT;
  while (!LL_I2C_IsActiveFlag_STOP(I2C1) && timeout-- > 0);
  if (timeout == 0) return M24C64_ERROR_TIMEOUT;

  // 清除停止标志
  LL_I2C_ClearFlag_STOP(I2C1);

  // 等待EEPROM内部编程完成（典型5ms）
  vTaskDelay(pdMS_TO_TICKS(5));

  return M24C64_OK;
}

/**
 * @brief  连续读取（可跨页）
 * @param  addr: 存储地址（0~8191）
 * @param  data: 读取数据的缓冲区
 * @param  len: 读取数据长度
 * @retval M24C64_Status_t: 状态码
 */
M24C64_Status_t M24C64_Read(uint16_t addr, uint8_t *data, uint16_t len)
{
  // 检查参数合法性
  if (addr >= M24C64_SIZE || len == 0 || (addr + len) > M24C64_SIZE)
    {
      return M24C64_ERROR_PARAM;
    }

  // 检查设备是否就绪
  M24C64_Status_t status = M24C64_CheckReady();
  if (status != M24C64_OK)
    {
      return status;
    }

  uint32_t timeout;
  // 步骤1：发送起始地址（伪写操作）
  timeout = M24C64_TIMEOUT;
  while (!LL_I2C_IsActiveFlag_TXE(I2C1) && timeout-- > 0);
  if (timeout == 0) return M24C64_ERROR_TIMEOUT;

  // 发送开始位+设备地址（写）
  LL_I2C_HandleTransfer(I2C1, M24C64_DEVICE_ADDR << 1, LL_I2C_ADDRSLAVE_7BIT,
                        2, LL_I2C_MODE_SOFTEND, LL_I2C_GENERATE_START_WRITE);

  // 发送高地址字节
  timeout = M24C64_TIMEOUT;
  while (!LL_I2C_IsActiveFlag_TXIS(I2C1) && timeout-- > 0);
  if (timeout == 0) return M24C64_ERROR_TIMEOUT;
  LL_I2C_TransmitData8(I2C1, (addr >> 8) & 0xFF);

  // 发送低地址字节
  timeout = M24C64_TIMEOUT;
  while (!LL_I2C_IsActiveFlag_TXIS(I2C1) && timeout-- > 0);
  if (timeout == 0) return M24C64_ERROR_TIMEOUT;
  LL_I2C_TransmitData8(I2C1, addr & 0xFF);// 等待发送完成
  timeout = M24C64_TIMEOUT;
  while (!LL_I2C_IsActiveFlag_TC(I2C1) && timeout-- > 0);
  if (timeout == 0) return M24C64_ERROR_TIMEOUT;

  // 步骤2：读取数据
  LL_I2C_HandleTransfer(I2C1, M24C64_DEVICE_ADDR << 1, LL_I2C_ADDRSLAVE_7BIT,
                        len, LL_I2C_MODE_AUTOEND, LL_I2C_GENERATE_START_READ);

  // 接收数据
  for (uint16_t i = 0; i < len; i++)
    {
      timeout = M24C64_TIMEOUT;
      while (!LL_I2C_IsActiveFlag_RXNE(I2C1) && timeout-- > 0);
      if (timeout == 0) return M24C64_ERROR_TIMEOUT;

      data[i] = LL_I2C_ReceiveData8(I2C1);
    }// 等待传输完成（自动结束模式会自动发送停止位）
  timeout = M24C64_TIMEOUT;
  while (!LL_I2C_IsActiveFlag_STOP(I2C1) && timeout-- > 0);
  if (timeout == 0) return M24C64_ERROR_TIMEOUT;

  // 清除停止标志
  LL_I2C_ClearFlag_STOP(I2C1);

  return M24C64_OK;
}/**
 * @brief  检查设备是否就绪（轮询ACK）
 * @retval M24C64_Status_t: M24C64_OK表示就绪，其他表示错误
 */
M24C64_Status_t M24C64_CheckReady(void)
{
  uint32_t timeout = M24C64_WRITE_TIMEOUT * 100;  // 扩大超时时间（毫秒转10us）

  // 轮询设备ACK（EEPROM在写入期间不会响应）
  while (timeout-- > 0)
    {
      // 发送开始位+设备地址（写），不发送数据
      LL_I2C_HandleTransfer(I2C1, M24C64_DEVICE_ADDR << 1, LL_I2C_ADDRSLAVE_7BIT,
                            0, LL_I2C_MODE_SOFTEND, LL_I2C_GENERATE_START_WRITE);

      // 等待应答或超时
      if (LL_I2C_IsActiveFlag_NACK(I2C1))
        {
          // 收到NACK，设备忙
          LL_I2C_ClearFlag_NACK(I2C1);
          LL_I2C_GenerateStopCondition(I2C1);
          vTaskDelay(pdMS_TO_TICKS(1));  // 短暂延时
          continue;
        }// 收到ACK，设备就绪
      LL_I2C_GenerateStopCondition(I2C1);
      return M24C64_OK;
    }

  return M24C64_ERROR_TIMEOUT;  // 超时
}

void vStorageTask(void *pvParameters)
{
    M24C64_Init();
    
    uint8_t test_array[4] = {1, 2, 3, 4};
    uint8_t read_array[4] = {0};
    
    for (;;)
    {
        // 方法1：使用单字节写入（简单但较慢）
        for (int i = 0; i < 4; i++)
        {
            M24C64_WriteByte(i, test_array[i]);
            vTaskDelay(pdMS_TO_TICKS(10)); // 写入间隔
        }
        
        // 读取验证
        M24C64_Read(0, read_array, 4);

        fr_printf("Read Data: %d %d %d %d", read_array[0], read_array[1], read_array[2], read_array[3]);
        
        // 方法2：使用页对齐地址写入（推荐）
        // 使用地址0（页对齐）
        // M24C64_PageWrite(0, test_array, 4);
        // M24C64_Read(0, read_array, 4);
        
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
