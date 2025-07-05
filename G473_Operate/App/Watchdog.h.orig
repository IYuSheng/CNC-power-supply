/* Watchdog.h 文件 */
#ifndef __WATCHDOG_H
#define __WATCHDOG_H

#include "stm32g4xx_ll_iwdg.h"
#include "freertos.h"
#include "task.h"
#include "UART_DEBUG.h"

// 关键任务心跳标志（每个关键任务对应一个，定期更新）
typedef struct {
  uint32_t task1_heartbeat; // 任务1心跳（如监控任务）
  uint32_t task2_heartbeat; // 任务2心跳（如通信任务）
  // ... 其他关键任务
} TaskHeartbeats;

static TaskHeartbeats g_heartbeats = {0};
static const uint32_t HEARTBEAT_TIMEOUT = 3000; // 心跳超时时间（ms）

void IWDG_Init(uint32_t timeout_ms);
void vWatchdogTask(void *pvParameters);

#endif /* __WATCHDOG_H */
