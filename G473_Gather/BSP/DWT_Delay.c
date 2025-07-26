#include "DWT_Delay.h"

/**
  * @brief 初始化DWT(Data Watchpoint and Trace)定时器
  * @note 必须在使用任何DWT功能前调用此函数
  */
void DWT_Init(void)
{
  // 启用DWT跟踪
  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;

  // 重置周期计数器
  DWT->CYCCNT = 0;

  // 启用周期计数器
  DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

/**
  * @brief 获取当前DWT计数值
  * @retval 当前CPU时钟周期计数
  */
uint32_t DWT_GetTick(void)
{
  return DWT->CYCCNT;
}

/**
  * @brief 微秒级延迟
  * @param us 要延迟的微秒数
  */
void DWT_Delayus(uint32_t us)
{
  uint32_t start = DWT->CYCCNT;
  // 计算需要等待的时钟周期数
  uint32_t cycles = us * (SystemCoreClock / 1000000);

  // 等待直到经过足够的时钟周期
  while((DWT->CYCCNT - start) < cycles);
}

/**
  * @brief 毫秒级延迟
  * @param ms 要延迟的毫秒数
  */
void DWT_Delayms(uint32_t ms)
{
  uint32_t start = DWT->CYCCNT;
  // 计算需要等待的时钟周期数
  uint32_t cycles = ms * (SystemCoreClock / 1000);

  // 等待直到经过足够的时钟周期
  while((DWT->CYCCNT - start) < cycles);
}
