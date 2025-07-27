/* DWT.h нд╪Ч */
#ifndef __DWT_H
#define __DWT_H

#include "stm32g4xx_ll_system.h"

void DWT_Init(void);
uint32_t DWT_GetTick(void);
void DWT_Delayus(uint32_t us);
void DWT_Delayms(uint32_t ms);

#endif /* __DWT_H */
