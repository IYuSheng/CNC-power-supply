#ifndef __Control_H
#define __Control_H

#include "Debug.h"
#include "Uart_Debug.h"

float TransformVoltage(float voltage);
float TransformCurrent(float current);
void HandleSetDAC(const char* param, float* dacValue, const char* dacName);

#endif /* __Control_H */
