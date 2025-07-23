#ifndef ENCODER_H
#define ENCODER_H

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "stm32g4xx_ll_gpio.h"
#include "stm32g4xx_ll_exti.h"
#include "stm32g4xx_ll_cortex.h"
#include "stm32g4xx_ll_tim.h"
#include "Uart_Debug.h"

// 编码器数据结构体（保持不变）
typedef struct
{
  int32_t total_count;       // 总计数
  uint8_t last_s3_level;     // PB14当前电平
  uint8_t last_s4_level;     // PB15当前电平
  uint32_t last_cnt;         // 定时器上次计数
} Encoder_HandleTypeDef;

// 编码器ID（调整为TIM2对应S1+S2的编码器模式）
typedef enum
{
  ENCODER_TIM2,  // TIM2编码器（S1=PA0+S2=PA1）
  ENCODER_TIM3,  // TIM3编码器
  ENCODER_SS2,   // S3=PB14+S4=PB15（外部中断）
  ENCODER_MAX
} Encoder_ID;

// 函数声明（保持不变）
void Encoder_Init(void);
void Encoder_GetData(Encoder_ID id, Encoder_HandleTypeDef *data);
void vEncoderTask(void *pvParameters);
uint32_t Encoder_GetRawCount(Encoder_ID id);

#endif
