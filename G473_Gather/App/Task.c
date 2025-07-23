#include "Task.h"

// 全局变量：存储传感器数据（供发送使用）
static UART_TxStruct sensor_data = {0};
// 创建数组存储四个通道的ADC结果
float voltages[4];

/**
  * @brief  任务1处理函数 - 10ms周期
  * @param  None
  * @retval None
  */
void Task1_Handler(void)
{
  UART1_Parse_Data();

  SGM58031_ReadVoltage(I2C1, 0, &voltages[0]);
  SGM58031_ReadVoltage(I2C1, 1, &voltages[1]);
  SGM58031_ReadVoltage(I2C1, 2, &voltages[2]);
  SGM58031_ReadVoltage(I2C1, 3, &voltages[3]);

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
//        }
//    }

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
  float SetA = uart_rx_data.dac_a;
  float SetB = uart_rx_data.dac_b;

//	float SetA = 2.5f;
//  float SetB = 0.7f;

  /* 设置通道A输出2.5V，通道B输出1.25V */
  DAC8562_SetVoltage(ADDR_CHANNEL_A, SetA);
  DAC8562_SetVoltage(ADDR_CHANNEL_B, SetB);
	
	// 喂狗
  LL_IWDG_ReloadCounter(IWDG);
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
	sensor_data.adc2 = (uint16_t)(Common_ADC_GetVoltage(ADC_CH_PB13) * 1000);
	sensor_data.adc3 = (uint16_t)(Common_ADC_GetVoltage(ADC_CH_PB14) * 1000);
  sensor_data.adc4 = (uint16_t)(Common_ADC_GetVoltage(ADC_CH_PB15) * 1000);
	
  /* -------------------------发送任务--------------------------- */
	
  /* 发送传感器数据到上位机） */
  UART1_Send_Struct(&sensor_data);
	
	// 喂狗
  LL_IWDG_ReloadCounter(IWDG);
}

/**
  * @brief  任务4处理函数 - 1000ms周期
  * @param  None
  * @retval None
  */
void Task4_Handler(void)
{
	/* 报文打印 */
	
	//打印输出模式以及dac输出电压
//	Debug_printf("start_flag=%d, mode=%d, dac_a = %.2f, dac_b = %.2f",
//               uart_rx_data.start_flag,
//               uart_rx_data.mode,
//               uart_rx_data.dac_a,
//               uart_rx_data.dac_b);
	
	Debug_printf("tmp1=%dmv, tmp2=%dmv, 12V_In = %dmv, 5V_In = %dmv",
               sensor_data.adc1,
               sensor_data.adc2,
               sensor_data.adc3,
               sensor_data.adc4);
	
	Debug_printf("adc1= %.2f, adc2= %.2f, adc3 = %.2f, adc4 = %.2f",
               voltages[0],
               voltages[1],
               voltages[2],
               voltages[3]);
	
	// 喂狗
  LL_IWDG_ReloadCounter(IWDG);
}
