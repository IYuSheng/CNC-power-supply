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

  // 示例：读取电压（实际需根据传感器驱动实现）
  sensor_data.voltage = 222;  // 假设返回mV值
  // 示例：读取电流
  sensor_data.current = 333;  // 假设返回mA值
  // 示例：读取温度
  sensor_data.temperature = 27;  // 假设返回℃值
}

/**
  * @brief  任务2处理函数 - 20ms周期
  * @param  None
  * @retval None
  */
void Task2_Handler(void)
{
  /* 数据处理 */
	
}

/**
  * @brief  任务3处理函数 - 100ms周期
  * @param  None
  * @retval None
  */
void Task3_Handler(void)
{
  /* 发送传感器数据到上位机） */
  UART1_Send_Struct(&sensor_data);

  /* 状态显示 */
  Debug_printf("start_flag=%d, mode=%d, reserved=[%d,%d]",
               uart_rx_data.start_flag,
               uart_rx_data.mode,
               uart_rx_data.reserved[0],
               uart_rx_data.reserved[1]);
}
