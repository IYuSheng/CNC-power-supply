/* Monitor.h нд╪Ч */
#ifndef __MONITOR_H
#define __MONITOR_H

#include "main.h"

uint32_t getRuntimeCounterValue(void);
void configureTimerForRuntimeStats(void);
void vSystemMonitorTask(void *pvParameters);

#endif /* __MONITOR_H */
