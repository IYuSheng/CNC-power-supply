#include "Sys_Timer.h"

// 系统节拍计数器
static volatile uint32_t system_tick = 0;

// 任务计数器
static volatile uint32_t task_RADC_counter = 0;
static volatile uint32_t task_RCommon_ADC_counter = 0;
static volatile uint32_t task_State_counter = 0;
static volatile uint32_t task_Comm_counter = 0;
static volatile uint32_t task_Debug_counter = 0;

// 任务执行周期(以系统滴答为单位)
static const uint32_t task_RADC_period = SYSTICK_FREQUENCY_HZ / TASK_RADC_FREQUENCY_HZ;
static const uint32_t task_RCommon_ADC_period = SYSTICK_FREQUENCY_HZ / TASK_RCommon_ADC_FREQUENCY_HZ;
static const uint32_t task_State_period = SYSTICK_FREQUENCY_HZ / TASK_State_FREQUENCY_HZ;
static const uint32_t task_Comm_period = SYSTICK_FREQUENCY_HZ / TASK_Comm_Recv_FREQUENCY_HZ;
static const uint32_t task_Debug_period = SYSTICK_FREQUENCY_HZ / TASK_Debug_FREQUENCY_HZ;

/**
  * @brief  定时器2初始化函数
  * @param  None
  * @retval None
  */
void MX_TIM2_Init(void)
{
  /* 使能定时器2时钟 */
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM2);

  /* 定时器基本配置 */
  LL_TIM_SetPrescaler(TIM2, (SystemCoreClock / 1000000) - 1);
  LL_TIM_SetCounterMode(TIM2, LL_TIM_COUNTERMODE_UP);
  LL_TIM_SetAutoReload(TIM2, 1000-1);
  LL_TIM_DisableARRPreload(TIM2);

  /* 使能定时器中断 */
  LL_TIM_EnableIT_UPDATE(TIM2);

  /* 配置NVIC */
  NVIC_SetPriority(TIM2_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 2, 0));
  NVIC_EnableIRQ(TIM2_IRQn);

  /* 使能定时器 */
  LL_TIM_EnableCounter(TIM2);
}

/**
  * @brief  定时器2中断服务函数
  * @param  None
  * @retval None
  */
void TIM2_IRQHandler(void)
{
  /* 检查更新中断标志 */
  if (LL_TIM_IsActiveFlag_UPDATE(TIM2) == 1)
    {
      /* 清除更新中断标志 */
      LL_TIM_ClearFlag_UPDATE(TIM2);

      /* 增加系统滴答计数器 */
      system_tick++;

      /* 更新任务计数器并设置任务标志 */
      if (++task_RADC_counter >= task_RADC_period)
        {
          task_RADC_counter = 0;
          task_RADC_flag = true;
        }

      if (++task_RCommon_ADC_counter >= task_RCommon_ADC_period)
        {
          task_RCommon_ADC_counter = 0;
          task_RCommonADC_flag = true;
        }
      if (++task_State_counter >= task_State_period)
        {
          task_State_counter = 0;
          task_Stop_flag = true;
        }
      if (++task_Comm_counter >= task_Comm_period)
        {
          task_Comm_counter = 0;
          task_Comm_Recv_flag = true;
        }
			if (++task_Debug_counter >= task_Debug_period)
        {
          task_Debug_counter = 0;
          task_Debug_flag = true;
        }
    }
}
