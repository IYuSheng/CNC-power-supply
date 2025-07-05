/* Init.h 文件 */
#ifndef __INIT_H
#define __INIT_H

#include "main.h"

/* 系统监控相关定义 */
#if Monitor_Flag
void Init_Monitor(void);
void configureTimerForRuntimeStats(void);
uint32_t getRuntimeCounterValue(void);
#endif

void Init_Hardware(void);
void SystemClock_Config(void);
void Init_App(void);

#endif /* __INIT_H */
