#include "Monitor.h"

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

  LL_TIM_InitTypeDef TIM_InitStruct = {0};
  TIM_InitStruct.Prescaler = 8499;   // 20KHz
  TIM_InitStruct.CounterMode = LL_TIM_COUNTERMODE_UP;
  TIM_InitStruct.Autoreload = 0xFFFF;         // 最大计数范围
  TIM_InitStruct.ClockDivision = LL_TIM_CLOCKDIVISION_DIV1;

  LL_TIM_Init(TIM4, &TIM_InitStruct);
  LL_TIM_DisableARRPreload(TIM4);
  LL_TIM_EnableCounter(TIM4);
}

#if Monitor_Flag
void vSystemMonitorTask(void *pvParameters)
{
  #define MAX_TASKS 15  // 最大任务数
  static TaskStatus_t currentTaskStatusArray[MAX_TASKS];
  static TaskStatus_t prevTaskStatusArray[MAX_TASKS];
  static uint32_t prevTotalRuntime = 0;
  static UBaseType_t prevNumTasks = 0;

  for(;;)
    {
      uint32_t currentTotalRuntime;
      UBaseType_t numTasks = uxTaskGetSystemState(currentTaskStatusArray, MAX_TASKS, &currentTotalRuntime);
      
      // 错误处理
      if (numTasks == 0 || numTasks > MAX_TASKS) {
        if (numTasks > MAX_TASKS) numTasks = MAX_TASKS;
        dma_printf("\r\nError: uxTaskGetSystemState() returned %d", numTasks);
        vTaskDelay(pdMS_TO_TICKS(10000)); // 出错时等待更长时间
        continue;
      }

      /* 状态变化检测与打印 */
      if (prevNumTasks != 0)
        {
          // 溢出处理
          uint32_t deltaTotal = currentTotalRuntime - prevTotalRuntime;
          
          // 范围检查
          if (deltaTotal > 20000 && deltaTotal < 60000)
            {
              dma_printf("\r\nTask:         CPU:      Stack:");

              /* 遍历所有任务计算CPU占用率 */
              for (UBaseType_t i = 0; i < numTasks; i++)
                {
                  if (i < prevNumTasks && currentTaskStatusArray[i].xHandle == prevTaskStatusArray[i].xHandle)
                    {
                      uint32_t deltaTask = currentTaskStatusArray[i].ulRunTimeCounter - prevTaskStatusArray[i].ulRunTimeCounter;
                      
                      // CPU使用率计算
                      float percent = (deltaTotal > 0) ? ((100.0f * deltaTask) / deltaTotal) : 0.0f;
                      
                      dma_printf("%-12s %6.2f%% %6u",
                               currentTaskStatusArray[i].pcTaskName,
                               percent,
                               (unsigned int)currentTaskStatusArray[i].usStackHighWaterMark);
                    }
                }
                // 打印系统堆内存使用情况
                dma_printf("Used:        %6.2f%%   %6u",((float)(configTOTAL_HEAP_SIZE - xPortGetFreeHeapSize()) / (float)configTOTAL_HEAP_SIZE) * 100.0f, xPortGetFreeHeapSize());
            }
        }

      /* 保存当前状态用于下次对比 */
      // 内存复制
      UBaseType_t copyTasks = (numTasks < MAX_TASKS) ? numTasks : MAX_TASKS;
      memcpy(prevTaskStatusArray, currentTaskStatusArray, copyTasks * sizeof(TaskStatus_t));
      prevTotalRuntime = currentTotalRuntime;
      prevNumTasks = copyTasks;

      vTaskDelay(pdMS_TO_TICKS(2000)); // 每2秒检查一次
    }
}
#endif
