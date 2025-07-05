/* DWT_Delay.h нд╪Ч */
#ifndef __DWT_DELAY_H
#define __DWT_DELAY_H

#include "main.h"
#include "FreeRTOS.h"
#include "task.h"

void delay_us(uint32_t us);
void delay_ms(uint32_t ms);
void DWT_Init(void);

#endif /* __DWT_DELAY_H */
