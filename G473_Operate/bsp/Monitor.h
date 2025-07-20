/* Monitor.h 文件 */
#ifndef __MONITOR_H
#define __MONITOR_H

#include "stm32g4xx_ll_tim.h"
#include "stm32g4xx_ll_bus.h"
#include "Uart_Debug.h"
#include "FreeRTOS.h"
#include "task.h"
#include <string.h>
#include <stdio.h>

#define Monitor_Flag configGENERATE_RUN_TIME_STATS	/* 系统监控启停宏 */

uint32_t getRuntimeCounterValue(void);
void configureTimerForRuntimeStats(void);
void vSystemMonitorTask(void *pvParameters);

#endif /* __MONITOR_H */
