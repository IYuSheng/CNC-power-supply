#include "Task.h"

// 全局变量：存储传感器数据（供发送使用）
static UART_TxStruct sensor_data = {0};

/**
  * @brief  任务1处理函数 - 10ms周期
  * @param  None
  * @retval None
  */
void Task1_Handler(void)
{
  UART1_Parse_Data();
  // 创建数组存储四个通道的ADC结果
  float voltages[4];

  SGM58031_ReadVoltage(I2C1, 0, &voltages[0]);
  //Debug_printf("Channel %d: %.4f V", 0, voltages[0]);
  SGM58031_ReadVoltage(I2C1, 1, &voltages[1]);
  //Debug_printf("Channel %d: %.4f V", 1, voltages[1]);
  SGM58031_ReadVoltage(I2C1, 2, &voltages[2]);
  //Debug_printf("Channel %d: %.4f V", 2, voltages[2]);
  SGM58031_ReadVoltage(I2C1, 3, &voltages[3]);
  //Debug_printf("Channel %d: %.4f V", 3, voltages[3]);

  // 读取所有通道电压
//  for(uint8_t ch = 0; ch < 4; ch++)
//    {
//      if(SGM58031_ReadVoltage(I2C1, ch, &voltages[ch]) == ADC_OK)
//        {
//          Debug_printf("Channel %d: %.4f V", ch, voltages[ch]);
//        }
//      else
//        {
//          Debug_printf("Channel %d: Read Error", ch);
//          //Debug_printf("ADC read error: %d", status);
//        }
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
	/* -------------------------读取任务--------------------------- */
	
	/* 读取CommonADC任务 */
  Common_ADC_ManualSample();  // 触发ADC采样

  // 更新传感器数据（转换为mV便于传输）
  sensor_data.adc1 = (uint16_t)(Common_ADC_GetVoltage(ADC_CH_PB12) * 1000);
  sensor_data.adc2 = (uint16_t)(Common_ADC_GetVoltage(ADC_CH_PB15) * 1000);
  sensor_data.adc3 = (uint16_t)(Common_ADC_GetVoltage(ADC_CH_PB13) * 1000);
  sensor_data.adc4 = (uint16_t)(Common_ADC_GetVoltage(ADC_CH_PB14) * 1000);
	
	/* 调试输出 */
  Debug_printf("ADC Values: %d,%d,%d,%d mV\r\n", 
               sensor_data.adc1, 
               sensor_data.adc2,
               sensor_data.adc3,
               sensor_data.adc4);
	
  /* -------------------------发送任务--------------------------- */
	
  /* 发送传感器数据到上位机） */
  UART1_Send_Struct(&sensor_data);

  /* 状态显示 */
//  Debug_printf("start_flag=%d, mode=%d, reserved=[%d,%d]",
//               uart_rx_data.start_flag,
//               uart_rx_data.mode,
//               uart_rx_data.reserved[0],
//               uart_rx_data.reserved[1]);
  // 喂狗
  LL_IWDG_ReloadCounter(IWDG);
}
