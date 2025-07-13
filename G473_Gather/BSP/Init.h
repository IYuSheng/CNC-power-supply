/* Init.h нд╪Ч */
#ifndef __INIT_H
#define __INIT_H

#include "main.h"
#include "stm32g4xx_ll_iwdg.h"

void Init_Sys(void);
void Init_Hardware(void);
void SystemClock_Config(void);
void Init_App(void);
void IWDG_Init(uint32_t timeout_ms);

#endif /* __INIT_H */
