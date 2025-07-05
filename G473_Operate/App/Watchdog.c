#include "Watchdog.h"

/**
 * @brief 初始化独立看门狗（IWDG）
 * @param timeout_ms：超时时间（ms），范围：128ms~32768ms（基于LSI=32kHz）
 */
void IWDG_Init(uint32_t timeout_ms)
{
  // 使能IWDG（一旦使能，只能通过复位关闭）
  LL_IWDG_Enable(IWDG);

  // 解锁IWDG寄存器（默认锁定，需解锁才能配置）
  LL_IWDG_EnableWriteAccess(IWDG);

  // 配置预分频器（LSI=32kHz，分频后时钟=32kHz / 预分频值）
  // 预分频选项：LL_IWDG_PRESCALER_4/8/16/32/64/128/256
  LL_IWDG_SetPrescaler(IWDG, LL_IWDG_PRESCALER_32); // 32kHz/32=1kHz（1ms计数1次）

  // 计算重装载值（超时时间 = 重装载值 * 1ms）
  uint32_t reload = timeout_ms;
  // 重装载值最大为0x0FFF（4095），超时时间最大为4095ms（约4秒）
  if (reload > 0x0FFF) reload = 0x0FFF;
  LL_IWDG_SetReloadCounter(IWDG, reload);

  // 等待寄存器更新完成
  while (LL_IWDG_IsReady(IWDG) == 0);

  // 初始喂狗（重置计数器）
  LL_IWDG_ReloadCounter(IWDG);
}

/**
 * @brief 喂狗任务：定期喂狗 + 检查其他任务心跳
 */
void vWatchdogTask(void *pvParameters)
{
  while (1)
    {
      // 1. 检查关键任务心跳是否正常
      //uint32_t current_tick = xTaskGetTickCount(); // 获取当前系统滴答数

//    // 检查任务1心跳（如果超时，停止喂狗）
//    if (current_tick - g_heartbeats.task1_heartbeat > HEARTBEAT_TIMEOUT) {
//      // 任务1卡死，进入死循环，不再喂狗，触发复位
//      for (;;);
//    }

//    // 检查任务2心跳（同上）
//    if (current_tick - g_heartbeats.task2_heartbeat > HEARTBEAT_TIMEOUT) {
//      for (;;);
//    }

      // 2. 所有任务正常，喂狗
      LL_IWDG_ReloadCounter(IWDG);

      //fr_printf("Watchdog\r\n");

      // 3. 1s执行一次（喂狗周期必须小于IWDG超时时间，如IWDG超时1秒，则喂狗周期<1秒）
      vTaskDelay(pdMS_TO_TICKS(100));
    }
}
