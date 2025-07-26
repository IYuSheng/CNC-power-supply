/* Sys_Timer.h 文件 */
#ifndef __SYS_TIMER_H
#define __SYS_TIMER_H

#include "stm32g4xx_ll_tim.h"
#include "stm32g4xx_ll_rcc.h"
#include "stm32g4xx_ll_bus.h"
#include <stdbool.h>

// 任务标志
extern volatile bool task_RADC_flag;
extern volatile bool task_RCommonADC_flag;
extern volatile bool task_SDAC_flag;
extern volatile bool task_Comm_Recv_flag;
extern volatile bool task_Debug_flag;
extern volatile bool task_Stop_flag;

#define SYSTICK_FREQUENCY_HZ    1000      // 系统滴答频率，1000Hz = 1ms
#define TASK_RADC_FREQUENCY_HZ     200       // 任务读取ADC频率，200Hz = 5ms
#define TASK_RCommon_ADC_FREQUENCY_HZ     5       // 任务读取CommonADC频率，5Hz = 200ms
#define TASK_State_FREQUENCY_HZ     50        // 任务检测状态频率，50Hz = 20ms
#define TASK_Comm_Recv_FREQUENCY_HZ     100         // 任务通信接收频率，100Hz = 10ms
#define TASK_Debug_FREQUENCY_HZ     1        // 任务调试打印频率，1Hz = 1000ms

void MX_TIM2_Init(void);

#endif /* __SYS_TIMER_H */
