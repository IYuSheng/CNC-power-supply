#ifndef __Control_H
#define __Control_H

#include "Debug.h"
#include "Uart_Debug.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "Encoder.h"
#include "Key.h"
#include "math.h"

extern float tp1, tp2;

#define Limit_Voltage	50.0f
#define Limit_Current	22.0f
#define Limit_DACA	2.5f
#define Limit_DACB	2.5f
#define VOLTAGE_CONVERT_COEF 0.047619f  // 电压转换系数
#define CURRENT_CONVERT_COEF 0.0375f    // 电流转换系数
#define CURRENT_OFFSET      1.65f       // 电流偏移量
#define	Default_Precision	0.1f

void vControlTask(void *argument);
extern inline float TransformVoltage(float voltage);
extern inline float TransformCurrent(float current);
float ConvertNTCTemperature(uint16_t adc_voltage_mv);
void ConvertSecondsToHMS(int32_t total_seconds, uint8_t *hours, uint8_t *minutes, uint8_t *seconds);
void HandleSetDAC(const char* param, float* dacValue, const char* dacName);

#endif /* __Control_H */
