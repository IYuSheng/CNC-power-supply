#include "Task_init.h"

BaseType_t xReturn;  /* 保存xTaskCreate返回值 */

extern void Error_Handler(void);

/**
  * @brief  初始化Debug串口调试任务
  */
void Debug_uart_task_create(void)
{
  xReturn = xTaskCreate(vUartProcessTask, "UartProc", 1024,
                        NULL, TASK_PRIO_UART, NULL);
  if (xReturn != pdPASS)
    {
      fr_printf("Debug task create Failed");
      Error_Handler();
    }
  else
    {
      fr_printf("Debug task create Success");
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
  if (xReturn != pdPASS)
    {
      fr_printf("Monitor task create Failed");
      Error_Handler();
    }
  else
    {
      fr_printf("Monitor task create Success");
    }

#endif
}

/**
  * @brief  初始化通信任务
  */
void Comm_task_create(void)
{
  xReturn = xTaskCreate(vUart1ProcessTask, "Comm", 1024,
												NULL, TASK_PRIO_Comm, NULL);
  if (xReturn != pdPASS)
    {
      fr_printf("Comm task create Failed");
      Error_Handler();
    }
  else
    {
      fr_printf("Comm task create Success");
    }
}

/**
  * @brief  初始化看门狗任务
  */
void Watchdog_task_create(void)
{
  IWDG_Init(3000); // 3000ms超时

  xReturn = xTaskCreate(vWatchdogTask, "Watchdog", 64, 
												NULL, TASK_PRIO_WATCHDOG, NULL);
  if (xReturn != pdPASS)
    {
      fr_printf("Watchdog task create Failed");
      Error_Handler();
    }
  else
    {
      fr_printf("Watchdog task create Success");
    }
}

/**
  * @brief  初始化按键任务
  */
void Key_task_create(void)
{
  xReturn = xTaskCreate(vKeyScanTask, "Key", 128,
												NULL, TASK_PRIO_KEY, NULL);
  if (xReturn != pdPASS)
    {
      fr_printf("Key task create Failed");
      Error_Handler();
    }
  else
    {
      fr_printf("Key task create Success");
    }
}

/**
  * @brief  初始化屏幕任务
  */
void TFT_task_create(void)
{
  xReturn = xTaskCreate(vTFTTask, "TFT", 1024,
												NULL,TASK_PRIO_TFT, NULL);
  if (xReturn != pdPASS)
    {
      fr_printf("TFT task create Failed");
      Error_Handler();
    }
  else
    {
      fr_printf("TFT task create Success");
    }
}

/**
  * @brief  初始化存储任务
  */
void Storage_task_create(void)
{
  xReturn = xTaskCreate(vStorageTask, "Storage", 128,
												NULL, TASK_PRIO_STORAGE, NULL);
  if (xReturn != pdPASS)
    {
      fr_printf("Storage task create Failed");
      Error_Handler();
    }
  else
    {
      fr_printf("Storage task create Success");
    }
}
