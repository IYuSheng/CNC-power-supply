/* Task_init.h 文件 */
#ifndef __TASK_INIT_H
#define __TASK_INIT_H

#include "main.h"
#include "Monitor.h"
#include "Init.h"
#include "UART_DEBUG.h"

#define Monitor_Flag configGENERATE_RUN_TIME_STATS	/* 系统监控启停宏 */

/* USER CODE BEGIN Includes */
#define TASK_PRIO_UART        4
#define TASK_PRIO_MONITOR     1

void Debug_uart_task_create(void);
void Monitor_task_create(void);

#endif /* __TASK_INIT_H */
