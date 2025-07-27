#include "m24c64.h"

// 超时设置（毫秒）
#define M24C64_TIMEOUT      1000      // 一般操作超时
#define M24C64_WRITE_TIMEOUT 10      // 写入操作超时

// 内部函数：等待I2C总线就绪
static M24C64_Status_t i2c_wait_for_ready(uint32_t timeout_ms)
{
  uint32_t timeout = timeout_ms * 100; // 转换为10us单位

  while (timeout-- > 0)
    {
      // 发送开始位+设备地址（写），不发送数据
      LL_I2C_HandleTransfer(I2C1, M24C64_DEVICE_ADDR << 1,
                            LL_I2C_ADDRSLAVE_7BIT, 0,
                            LL_I2C_MODE_SOFTEND, LL_I2C_GENERATE_START_WRITE);

      // 等待应答
      if (LL_I2C_IsActiveFlag_NACK(I2C1))
        {
          // 收到NACK，清除标志并继续轮询
          LL_I2C_ClearFlag_NACK(I2C1);
          LL_I2C_GenerateStopCondition(I2C1);
          continue;
        }

      LL_I2C_GenerateStopCondition(I2C1);
      return M24C64_OK;
    }

  return M24C64_ERROR_TIMEOUT;
}

// 内部函数：发送I2C数据
static M24C64_Status_t i2c_send_data(const uint8_t *data, uint16_t len, uint8_t auto_end)
{
  uint32_t timeout;

  // 等待TXE标志
  timeout = M24C64_TIMEOUT;
  while (!LL_I2C_IsActiveFlag_TXE(I2C1) && timeout-- > 0);
  if (timeout == 0) return M24C64_ERROR_TIMEOUT;

  // 发送数据
  LL_I2C_HandleTransfer(I2C1, M24C64_DEVICE_ADDR << 1,
                        LL_I2C_ADDRSLAVE_7BIT, len,
                        auto_end ? LL_I2C_MODE_AUTOEND : LL_I2C_MODE_SOFTEND,
                        LL_I2C_GENERATE_START_WRITE);

  for (uint16_t i = 0; i < len; i++)
    {
      timeout = M24C64_TIMEOUT;
      while (!LL_I2C_IsActiveFlag_TXIS(I2C1) && timeout-- > 0);
      if (timeout == 0) return M24C64_ERROR_TIMEOUT;

      LL_I2C_TransmitData8(I2C1, data[i]);

      // 检查NACK
      if (LL_I2C_IsActiveFlag_NACK(I2C1))
        {
          LL_I2C_ClearFlag_NACK(I2C1);
          LL_I2C_GenerateStopCondition(I2C1);
          return M24C64_ERROR_I2C;
        }
    }

  // 等待传输完成
  if (!auto_end)
    {
      timeout = M24C64_TIMEOUT;
      while (!LL_I2C_IsActiveFlag_TC(I2C1) && timeout-- > 0);
      if (timeout == 0) return M24C64_ERROR_TIMEOUT;
    }
  else
    {
      timeout = M24C64_TIMEOUT;
      while (!LL_I2C_IsActiveFlag_STOP(I2C1) && timeout-- > 0);
      if (timeout == 0) return M24C64_ERROR_TIMEOUT;
      LL_I2C_ClearFlag_STOP(I2C1);
    }

  return M24C64_OK;
}

// 内部函数：接收I2C数据
static M24C64_Status_t i2c_receive_data(uint8_t *data, uint16_t len)
{
  uint32_t timeout;

  // 等待TXE标志
  timeout = M24C64_TIMEOUT;
  while (!LL_I2C_IsActiveFlag_TXE(I2C1) && timeout-- > 0);
  if (timeout == 0) return M24C64_ERROR_TIMEOUT;

  // 发起读取
  LL_I2C_HandleTransfer(I2C1, M24C64_DEVICE_ADDR << 1,
                        LL_I2C_ADDRSLAVE_7BIT, len,
                        LL_I2C_MODE_AUTOEND, LL_I2C_GENERATE_START_READ);

  // 接收数据
  for (uint16_t i = 0; i < len; i++)
    {
      timeout = M24C64_TIMEOUT;
      while (!LL_I2C_IsActiveFlag_RXNE(I2C1) && timeout-- > 0);
      if (timeout == 0) return M24C64_ERROR_TIMEOUT;

      data[i] = LL_I2C_ReceiveData8(I2C1);
    }

  // 等待停止位
  timeout = M24C64_TIMEOUT;
  while (!LL_I2C_IsActiveFlag_STOP(I2C1) && timeout-- > 0);
  if (timeout == 0) return M24C64_ERROR_TIMEOUT;

  LL_I2C_ClearFlag_STOP(I2C1);
  return M24C64_OK;
}

/**
 * @brief  初始化M24C64（检查设备连接）
 * @retval M24C64_Status_t: 状态码
 */
M24C64_Status_t M24C64_Init(void)
{
  return i2c_wait_for_ready(M24C64_TIMEOUT);
}

/**
 * @brief  等待EEPROM内部写操作完成
 * @retval M24C64_Status_t: 状态码
 */
M24C64_Status_t M24C64_WaitForWriteComplete(void)
{
  return i2c_wait_for_ready(M24C64_WRITE_TIMEOUT);
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
  uint8_t buffer[3] =
  {
    (uint8_t)(addr >> 8),  // 高地址字节
    (uint8_t)(addr & 0xFF), // 低地址字节
    data                   // 数据字节
  };

  // 检查参数合法性
  if (addr >= M24C64_SIZE)
    return M24C64_ERROR_PARAM;

  // 检查设备就绪
  M24C64_Status_t status = i2c_wait_for_ready(M24C64_TIMEOUT);
  if (status != M24C64_OK)
    return status;

  // 发送数据
  status = i2c_send_data(buffer, 3, 1); // 自动结束模式
  if (status != M24C64_OK)
    return status;

  // 等待内部写操作完成
  return M24C64_WaitForWriteComplete();
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
    return M24C64_ERROR_PARAM;

  // 检查是否页对齐
  if ((addr % M24C64_PAGE_SIZE) != 0 || ((addr + len) > ((addr / M24C64_PAGE_SIZE + 1) * M24C64_PAGE_SIZE)))
    return M24C64_ERROR_PARAM;

  // 构建写入数据（地址+数据）
  uint8_t buffer[2 + M24C64_PAGE_SIZE];
  buffer[0] = (uint8_t)(addr >> 8);  // 高地址字节
  buffer[1] = (uint8_t)(addr & 0xFF); // 低地址字节

  // 复制数据
  for (uint16_t i = 0; i < len; i++)
    buffer[2 + i] = data[i];

  // 检查设备就绪
  M24C64_Status_t status = i2c_wait_for_ready(M24C64_TIMEOUT);
  if (status != M24C64_OK)
    return status;

  // 发送数据
  status = i2c_send_data(buffer, 2 + len, 1); // 自动结束模式
  if (status != M24C64_OK)
    return status;

  // 等待内部写操作完成
  return M24C64_WaitForWriteComplete();
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
  uint8_t addr_buffer[2] =
  {
    (uint8_t)(addr >> 8),  // 高地址字节
    (uint8_t)(addr & 0xFF)  // 低地址字节
  };

  // 检查参数合法性
  if (addr >= M24C64_SIZE || len == 0 || (addr + len) > M24C64_SIZE)
    return M24C64_ERROR_PARAM;

  // 检查设备就绪
  M24C64_Status_t status = i2c_wait_for_ready(M24C64_TIMEOUT);
  if (status != M24C64_OK)
    return status;

  // 步骤1：发送起始地址（伪写操作）
  status = i2c_send_data(addr_buffer, 2, 0); // 软结束模式
  if (status != M24C64_OK)
    return status;

  // 步骤2：读取数据
  return i2c_receive_data(data, len);
}

M24C64_Status_t M24C64_WriteByPage(uint16_t addr, const uint8_t *data, uint16_t len)
{
  uint16_t remaining = len;
  uint16_t current_addr = addr;
  const uint8_t *current_data = data;
  M24C64_Status_t status = M24C64_OK;

  while (remaining > 0 && status == M24C64_OK)
    {
      uint16_t page_start = current_addr & ~(M24C64_PAGE_SIZE - 1);
      uint16_t page_remaining = M24C64_PAGE_SIZE - (current_addr - page_start);
      uint16_t write_len = (remaining < page_remaining) ? remaining : page_remaining;

      status = M24C64_PageWrite(page_start, current_data, write_len);
      if (status != M24C64_OK) break;

      remaining -= write_len;
      current_addr += write_len;
      current_data += write_len;
    }
  return status;
}

/**
 * @brief  测试任务
 */
void vStorageTask(void *pvParameters)
{
//  M24C64_Status_t ret;

//  // 初始化
//  ret = M24C64_Init();
//  if (ret != M24C64_OK)
//    {
//      fr_printf("M24C64初始化失败！状态码：%d\n", ret);
//      vTaskDelete(NULL);
//    }

//  // 测试数据（跨页写入）
//  uint8_t test_data[64];
//  uint8_t read_data[64];

//  // 填充测试数据
//  for (uint16_t i = 0; i < 64; i++)
//    test_data[i] = i + 1;

//  // 写入测试（跨2页）
//  ret = M24C64_WriteByPage(0, test_data, 64);
//  if (ret != M24C64_OK)
//    {
//      fr_printf("页写入失败！状态码：%d\n", ret);
//      vTaskDelete(NULL);
//    }


//  // 读取测试
//  ret = M24C64_Read(0, read_data, 64);
//  if (ret != M24C64_OK)
//    {
//      fr_printf("读取失败！状态码：%d\n", ret);
//      vTaskDelete(NULL);
//    }

//  // 验证数据
//  uint8_t verify_ok = 1;
//  for (uint16_t i = 0; i < 64; i++)
//    {
//      if (read_data[i] != test_data[i])
//        {
//          fr_printf("数据不匹配！地址0x%04X：写入0x%02X，读出0x%02X\n", i, test_data[i], read_data[i]);
//          verify_ok = 0;
//          break;
//        }
//    }

//  if (verify_ok)
//    fr_printf("所有数据验证通过！\n");

  // 循环待命
  for (;;)
    {
      vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
