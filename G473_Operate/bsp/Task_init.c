#include "Task_init.h"

BaseType_t xReturn;  /* 保存xTaskCreate返回值 */

extern void Error_Handler(void);

/**
  * @brief  初始化Debug串口调试任务
  */
void Debug_uart_task_create(void)
{
		xReturn = xTaskCreate(vUartProcessTask, "UartProc", 512, 
                          NULL, TASK_PRIO_UART, NULL);
    if (xReturn != pdPASS) {
				fr_printf("Debug task create Failed\r\n");
        Error_Handler();
    }
		else
		{
			fr_printf("Debug task create Success\r\n");
		}
}

/**
  * @brief  初始化Monitor系统监控任务
  */
void Monitor_task_create(void)
{
#if Monitor_Flag
	
	  /* 初始化运行时间统计定时器 */
		configureTimerForRuntimeStats();
    /* 创建系统监控任务 */
    xReturn = xTaskCreate(vSystemMonitorTask, "Monitor", 256, 
                          NULL, TASK_PRIO_MONITOR, NULL);
    if (xReturn != pdPASS) {
				fr_printf("Monitor task create Failed\r\n");
        Error_Handler();
    }
		else
		{
			fr_printf("Monitor task create Success\r\n");
		}
		
#endif
}

/**
  * @brief  初始化看门狗任务
  */
void Watchdog_task_create(void)
{
	IWDG_Init(2000); // 2000ms超时
	
	xReturn = xTaskCreate(vWatchdogTask, "Watchdog", 128, NULL, 3, NULL);
	  if (xReturn != pdPASS) {
				fr_printf("Watchdog task create Failed\r\n");
        Error_Handler();
    }
		else
		{
			fr_printf("Watchdog task create Success\r\n");
		}
}

