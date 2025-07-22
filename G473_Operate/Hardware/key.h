#ifndef __KEY_H
#define __KEY_H

#include "stm32g4xx_ll_gpio.h"
#include "Uart_Debug.h"
#include <stdint.h>
#include "freertos.h"
#include "task.h"
#include "queue.h"

// 消息队列句柄（全局可见）
extern QueueHandle_t key_msg_queue;

// 按键枚举（与key_pin_map严格对应）
typedef enum {
    KEY_ENC_1 = 0,    // PC13
    KEY_ENC_2 = 1,    // PC14
    KEY_ENC_3 = 2,    // PC15
    KEY_FUNC1 = 3,    // PA6
    KEY_FUNC2 = 4,    // PA8
    KEY_FUNC3 = 5,    // PB1
    KEY_MAX           // 总数量
} Key_TypeDef;


// 函数声明
void Key_Init(void);
uint8_t Key_Read(Key_TypeDef key);
uint8_t Key_GetStateChange(Key_TypeDef key);
void Key_ResetStateChange(Key_TypeDef key);
void vKeyScanTask(void *pvParameters);

#endif /* __KEY_H */
