#include "Task.h"

// 全局变量：存储传感器数据（供发送使用）
static UART_TxStruct sensor_data = {0};

// 全局变量：存储读取到的四通道ADC（供解析使用）
ADC_Results adc_rd = {0};

/**
  * @brief  任务1处理函数 - 10ms周期
  * @param  None
  * @retval None
  */
void Task1_Handler(void)
{
  // 创建数组存储四个通道的ADC结果
  int16_t adc_results[4];

  UART1_Parse_Data();
  // 调用SGM58031读取所有通道
//  ADC_StatusTypeDef status = SGM58031_ReadAllChannels(I2C1, adc_results);

//    if(status == ADC_OK)
//    {
//        // 将ADC结果存储到全局结构体
//        adc_rd.ch0_value = adc_results[0];
//        adc_rd.ch1_value = adc_results[1];
//        adc_rd.ch2_value = adc_results[2];
//        adc_rd.ch3_value = adc_results[3];
//
//        Debug_printf("ADC read1: %d, ADC read2: %d, ADC read3: %d, ADC read4: %d",
//                    adc_rd.ch0_value, adc_rd.ch1_value, adc_rd.ch2_value, adc_rd.ch3_value);
//    }
//    else
//    {
//        // 处理错误：设置错误标志或使用默认值
//        Debug_printf("ADC read error: %d", status);
//    }

		ADC_StatusTypeDef status = SGM58031_ReadChannel(I2C1, 0, &adc_results[0]);
		if(status == ADC_OK)
    {
			adc_rd.ch1_value = adc_results[0];
			Debug_printf("ADC read0: %d",adc_rd.ch1_value);
		}
		else
    {
        // 处理错误：设置错误标志或使用默认值
        Debug_printf("ADC read error: %d", status);
    }

// 初始化后检查配置寄存器
//  uint16_t adc_config;
//  if (SGM58031_ReadConfig(I2C1, &adc_config) == ADC_OK)
//    {
//      Debug_printf("ADC Config: 0x%04X\n", adc_config);
//      Debug_printf(" - OS: %s\n", (adc_config & 0x8000) ? "Active" : "Inactive");
//      Debug_printf(" - MUX: CH%d\n", (adc_config >> 12) & 0x7);
//      Debug_printf(" - PGA: %d\n", 4096 >> ((adc_config >> 9) & 0x7));
//      Debug_printf(" - MODE: %s\n", (adc_config & 0x0100) ? "Single" : "Continuous");
//      Debug_printf(" - DR: %d SPS\n", 8 << ((adc_config >> 5) & 0x7)); // 简化计算
//    }

  // // 示例：读取电压（实际需根据传感器驱动实现）
  // sensor_data.voltage = 222;  // 假设返回mV值
  // // 示例：读取电流
  // sensor_data.current = 333;  // 假设返回mA值
  // // 示例：读取温度
  // sensor_data.temperature = 27;  // 假设返回℃值
}

/**
  * @brief  任务2处理函数 - 20ms周期
  * @param  None
  * @retval None
  */
void Task2_Handler(void)
{
  /* 数据处理 */

  /* 临时测试 */
  uint8_t SetA = uart_rx_data.reserved[0];
  uint8_t SetB = uart_rx_data.reserved[1];

  /* 设置通道A输出2.5V，通道B输出1.25V */
  DAC8562_SetVoltage(ADDR_CHANNEL_A, SetA);
  DAC8562_SetVoltage(ADDR_CHANNEL_B, SetB);

}

/**
  * @brief  任务3处理函数 - 100ms周期
  * @param  None
  * @retval None
  */
void Task3_Handler(void)
{
  /* 发送任务 */

  /* 发送传感器数据到上位机） */
  UART1_Send_Struct(&sensor_data);

  /* 状态显示 */
//  Debug_Debug_printf("start_flag=%d, mode=%d, reserved=[%d,%d]",
//               uart_rx_data.start_flag,
//               uart_rx_data.mode,
//               uart_rx_data.reserved[0],
//               uart_rx_data.reserved[1]);
}
