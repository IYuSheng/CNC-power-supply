/* Sys_Timer.h 文件 */
#ifndef __SYS_TIMER_H
#define __SYS_TIMER_H

#include "stm32g4xx_ll_tim.h"
#include "stm32g4xx_ll_rcc.h"
#include "stm32g4xx_ll_bus.h"
#include "Uart_Debug.h"
#include <stdbool.h>

#define SYSTICK_FREQUENCY_HZ    1000    // 系统滴答频率，1000Hz = 1ms
#define TASK_1_FREQUENCY_HZ     1      // 任务1频率，100Hz = 10ms
#define TASK_2_FREQUENCY_HZ     50      // 任务2频率，50Hz = 20ms
#define TASK_3_FREQUENCY_HZ     10      // 任务3频率，10Hz = 100ms
#define TASK_4_FREQUENCY_HZ     1      // 任务4频率，1Hz = 1000ms

void MX_TIM2_Init(void);

#endif /* __SYS_TIMER_H */
