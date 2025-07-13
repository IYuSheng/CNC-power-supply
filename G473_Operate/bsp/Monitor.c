#include "Monitor.h"

#if Monitor_Flag
/* 系统监控任务相关变量 */
static TaskStatus_t prevTaskStatusArray[10] = {0};
static uint32_t prevTotalRuntime = 0;
static UBaseType_t prevNumTasks = 0;

/* 系统监控任务函数 */
void vSystemMonitorTask(void *pvParameters);
#endif

/**
  * @brief  获取TIM4计数器值（用于FreeRTOS运行时间统计）
  */
uint32_t getRuntimeCounterValue(void)
{
  return LL_TIM_GetCounter(TIM4);
}

/**
  * @brief  配置TIM4为FreeRTOS运行时间统计时钟
  */
void configureTimerForRuntimeStats(void)
{
  // 启用TIM4时钟（G473的TIM4属于APB1外设）
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM4);

  // 配置时基（85MHz时钟，预分频84：计数频率1MHz，1us计数1次）
  LL_TIM_InitTypeDef TIM_InitStruct = {0};
  TIM_InitStruct.Prescaler = 84;                  // 85MHz / (84+1) = 1MHz
  TIM_InitStruct.CounterMode = LL_TIM_COUNTERMODE_UP;
  TIM_InitStruct.Autoreload = 0xFFFFFFFF;         // 最大计数范围
  TIM_InitStruct.ClockDivision = LL_TIM_CLOCKDIVISION_DIV1;

  LL_TIM_Init(TIM4, &TIM_InitStruct);
  LL_TIM_DisableARRPreload(TIM4);
  LL_TIM_EnableCounter(TIM4);
}

#if Monitor_Flag
/**
  * @brief  系统监控任务（打印任务状态、CPU占用率）
  */
void vSystemMonitorTask(void *pvParameters)
{

  const UBaseType_t maxTasks = 10;
  TaskStatus_t *taskStatusArray = pvPortMalloc(maxTasks * sizeof(TaskStatus_t));

  if (!taskStatusArray)
    {
      vTaskDelete(NULL);
      return;
    }

  for(;;)
    {
      UBaseType_t numTasks = uxTaskGetSystemState(taskStatusArray, maxTasks, NULL);
      if (numTasks > maxTasks) numTasks = maxTasks;

      // 计算总运行时间
      uint32_t currentTotalRuntime = 0;
      for (UBaseType_t i = 0; i < numTasks; i++)
        {
          currentTotalRuntime += taskStatusArray[i].ulRunTimeCounter;
        }

      /* 状态变化检测与打印 */
      if (prevNumTasks != 0)
        {
          uint32_t deltaTotal = currentTotalRuntime - prevTotalRuntime;
          if (deltaTotal > 0)
            {
              char buffer[128];
              UART_Send_IT(USART3, (uint8_t*)"\r\n", 1);

              /* 遍历所有任务计算CPU占用率 */
              for (UBaseType_t i = 0; i < numTasks; i++)
                {
                  for (UBaseType_t j = 0; j < prevNumTasks; j++)
                    {
                      if (taskStatusArray[i].xHandle == prevTaskStatusArray[j].xHandle)
                        {
                          uint32_t deltaTask = taskStatusArray[i].ulRunTimeCounter - prevTaskStatusArray[j].ulRunTimeCounter;
                          float percent = (100.0f * deltaTask) / deltaTotal;
                          snprintf(buffer, sizeof(buffer),
                                   "%-12s CPU:%5.2f%% Stack:%5u\r\n",
                                   taskStatusArray[i].pcTaskName,
                                   percent,
                                   taskStatusArray[i].usStackHighWaterMark);
                          UART_Send_IT(USART3, (uint8_t*)buffer, strlen(buffer));
                          break;
                        }
                    }
                }
            }
        }

      /* 保存当前状态用于下次对比 */
      memcpy(prevTaskStatusArray, taskStatusArray, numTasks * sizeof(TaskStatus_t));
      prevTotalRuntime = currentTotalRuntime;
      prevNumTasks = numTasks;

      vTaskDelay(pdMS_TO_TICKS(500)); /* 5秒更新一次 */
    }
}
#endif
