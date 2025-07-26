#ifndef __COMMON_ADC_H
#define __COMMON_ADC_H

#include "stm32g4xx_ll_adc.h"
#include "stm32g4xx_ll_gpio.h"
#include "stm32g4xx_ll_rcc.h"
#include "stm32g4xx_ll_tim.h"
#include "stm32g4xx_ll_bus.h"

/* 通道枚举（对应PB12-PB15） */
typedef enum {
    ADC_tmp1 = 0,  // ADC1_IN11 (PB12)
    ADC_5V_IN = 1,  // ADC2_IN15 (PB15)
    ADC_tmp2 = 2,  // ADC3_IN5  (PB13)
    ADC_12V_IN = 3   // ADC4_IN4  (PB14)
} ADC_Channel_t;

/* 初始化函数 */
void Common_ADC_Init(void);                  // 初始化所有ADC
uint16_t Common_ADC_GetRawValue(ADC_Channel_t ch);  // 获取原始ADC值
float Common_ADC_GetVoltage(ADC_Channel_t ch);      // 获取转换后的电压值
void Common_ADC_ManualSample(void);          // 手动触发一次采样
float Common_ADC_Tran(ADC_Channel_t ch);		//处理转换温度电流结果

#endif /* __COMMON_ADC_H */
