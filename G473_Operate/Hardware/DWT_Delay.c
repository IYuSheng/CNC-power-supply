#include "DWT_Delay.h"

// 微秒级延时（基于DWT硬件计数器，需在系统初始化后使用）
//void delay_us(uint32_t us)
//{
//  uint32_t start = DWT->CYCCNT;
//  uint32_t cycles = us * (SystemCoreClock / 1000000);  // 计算所需时钟周期数
//  while (DWT->CYCCNT - start < cycles);
//}
// 临时软件延时（仅用于测试）
void delay_us(uint32_t us)
{
  for (uint32_t i = 0; i < us * (170000000 / 1000000 / 10); i++);
}

// 毫秒级延时（基于微秒延时）
void delay_ms(uint32_t ms)
{
  delay_us(ms * 1000);
}

// 启用DWT计数器（需在系统初始化后调用一次）
void DWT_Init(void)
{
  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;  // 使能DWT
  DWT->CYCCNT = 0;                                 // 清零计数器
  DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;            // 启用周期计数
}
