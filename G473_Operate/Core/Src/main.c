/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : 低保真(已黑化) 出品：STM32G473数控电源操作端
  ******************************************************************************
  * @attention
  * （注：叫我的网名是请一定要加上(已黑化)，谢谢！）
  ******************************************************************************
  */
/* USER CODE END Header */

#include "main.h"

int main(void)
{
	SystemClock_Config();//系统时钟初始化
	Init_Hardware();//硬件层初始化
	Init_App();//应用层初始化
  /* ------------------------------ 任务创建 -------------------------------------- */
	Watchdog_task_create();//看门狗初始化
	Debug_uart_task_create();//调试串口初始化
	Monitor_task_create();//系统监视器初始化
	Comm_task_create();//通信串口初始化
	TFT_task_create();//屏幕初始化
	Storage_task_create();//EEPROM初始化
	Key_task_create();//按键任务初始化
	Encoder_task_create();//编码器任务初始化
	Control_task_create();//操作控制任务初始化
	Test_task_create();//测试接口任务初始化
  
	S_F = 1;	//启动操作系统标志
  /* ------------------------------ 启动调度器 ---------------------------------- */
	
  vTaskStartScheduler();

	for(;;) {}
}


#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/* FreeRTOS回调函数 */
void vApplicationMallocFailedHook(void)
{
	fr_printf("Application malloc failed");
  for(;;) { /* 内存分配失败时系统挂起 */ }
}

/* FreeRTOS空闲任务 */
void vApplicationIdleHook(void)
{

}

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
  /* 堆栈溢出处理 */
  (void)xTask;
  (void)pcTaskName;
  fr_printf("\r\n!!! Stack Overflow in %s !!!\r\n", pcTaskName);
  while (1);
}

void vApplicationTickHook(void)
{
  /* 时钟节拍处理 ,可放置lvgl的时钟刷新*/

}
