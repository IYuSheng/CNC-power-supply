#include "main.h"

int main(void)
{
	//系统初始化
	Init_Sys();
	//系统看门狗初始化
	IWDG_Init(1000);
	//系统时钟初始化
  SystemClock_Config();
	//硬件初始化
	Init_Hardware();
	//App接口函数初始化
	Init_App();
	
  while (1)
  {
	/* --------------------前台任务-------------------- */
		/* 指令接收及发送数据任务 */
    if (task1_flag)
    {
      task1_flag = false;
      Task1_Handler();
    }
    
    /* 检查并处理任务2 */
    if (task2_flag)
    {
      task2_flag = false;
      Task2_Handler();
    }
    
    /* 检查并处理任务3 */
    if (task3_flag)
    {
      task3_flag = false;
      Task3_Handler();
    }
    
    /* --------------------后台任务-------------------- */

		
  }
}


/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
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
