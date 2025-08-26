/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : 低保真(已黑化) 出品：STM32G473数控电源操作端
  ******************************************************************************
  * @attention
  * 注：叫我的网名是请一定要加上(已黑化)，谢谢！
  ******************************************************************************
  */
/* USER CODE END Header */

#include "main.h"

int main(void)
{
  // 系统初始化（保持原逻辑）
  Init_Sys();
  IWDG_Init(1000);
  SystemClock_Config();
  Init_Hardware();
  Init_App();

  while (1)
    {
      // 按优先级遍历任务：先处理最高优先级的就绪任务
      for (uint8_t i = 0; i < TASK_NUM; i++)
        {
          Task_t *task = &tasks[i];
          if (*(task->flag))    // 任务就绪
            {
              *(task->flag) = false;  // 清除标志位
							
              switch (task->id)
                {
                case TASK_ID_ReadADC:
                  Task_ReadADC_Handler();
									break;
								case TASK_ID_Read_Common_ADC:
									Task_Read_Common_ADC_Handler();
                  break;
                case TASK_ID_SetDAC:
                  Task_SetDAC_Handler();
                  break;
                case TASK_ID_Comm_Recv:
                  Task_Comm_Recv_Handler();
                  break;
								case TASK_ID_Comm_Send:
                  Task_Comm_Send_Handler();
                  break;
                case TASK_ID_Debug:
                  Task_Debug_Handler();
                  break;
                case TASK_ID_Stop:
                  Task_Stop_Handler();
                  break;
                case TASK_ID_PID:
                  Task_PID_Handler();
                  break;
                default:
                  break;
                }
            }
        }
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
