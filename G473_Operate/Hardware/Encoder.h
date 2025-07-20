#ifndef ENCODER_H
#define ENCODER_H

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "stm32g4xx_ll_tim.h"
#include "stm32g4xx_ll_gpio.h"

// 编码器数据结构体
typedef struct
{
    int32_t total_count;       // 总计数
    int16_t step;              // 单次步长（+1/-1）
    uint8_t dir;               // 方向：0-静止，1-顺时针，2-逆时针
    uint32_t last_cnt;         // 上一次定时器计数
    uint8_t is_overflow;       // 溢出标志
} Encoder_HandleTypeDef;

// 编码器ID
typedef enum
{
    ENCODER_TIM2,
    ENCODER_TIM3,
    ENCODER_TIM15,
    ENCODER_MAX
} Encoder_ID;

// 初始化编码器
void Encoder_Init(void);

// 获取编码器数据（线程安全）
void Encoder_GetData(Encoder_ID id, Encoder_HandleTypeDef *data);

// 编码器处理任务
void vEncoderTask(void *pvParameters);

#endif
