#ifndef __KEY_STOP_H
#define __KEY_STOP_H

#include "stm32g4xx_ll_gpio.h"
#include "Task.h"

// 按键状态枚举
typedef enum
{
  KEY_STOP_RELEASED = 0,  // 按键释放
  KEY_STOP_PRESSED        // 按键按下
} Key_Stop_StateTypeDef;

// 函数声明
Key_Stop_StateTypeDef Key_Stop_GetState(void);
uint8_t Key_Stop_IsPressed(void);
uint8_t Key_Stop_IsReleased(void);
uint8_t Get_Mode(void);

#endif /* __KEY_STOP_H */
