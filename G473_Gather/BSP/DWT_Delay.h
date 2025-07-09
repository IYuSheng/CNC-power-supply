#ifndef __DWT_DELAY_H
#define __DWT_DELAY_H

#include "stm32g4xx_ll_system.h"

void DWT_Init(void);
void DWT_Delayus(uint32_t us);
void DWT_Delayms(uint32_t ms);
uint32_t DWT_GetTick(void);

#endif /* __DWT_DELAY_H */
