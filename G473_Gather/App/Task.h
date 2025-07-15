/* Task.h нд╪Ч */
#ifndef __TASK_H
#define __TASK_H

#include "stm32g4xx_ll_iwdg.h"
#include "Sys_Timer.h"
#include "Uart_Debug.h"
#include "Uart_comm.h"
#include "DAC8562.h"
#include "SGM58031.h"
#include "Common_ADC.h"

void Task1_Handler(void);
void Task2_Handler(void);
void Task3_Handler(void);

#endif /* __TASK_H */
