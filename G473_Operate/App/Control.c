#include "Control.h"

// 将设置电压值转换为DAC数字值
float TransformVoltage(float voltage)
{
    return voltage * 0.047619f;
}

// 将设置电流值转换为DAC数字值
float TransformCurrent(float current)
{
    return current * 0.047619f;
}

// 函数：处理设置DAC的值
// 参数：param：参数；dacValue：所赋值DAC；dacName：命令参数
void HandleSetDAC(const char* param, float* dacValue, const char* dacName)
{
  if (param == NULL)
  {
    fr_printf("%s: missing parameter", dacName);
    return;
  }
  char* endPtr;
  float value = strtof(param, &endPtr);
  if (endPtr != param && *endPtr == '\0')
  {
    *dacValue = value;
    fr_printf("%s updated to: %.4f", dacName, *dacValue);
  }
  else
  {
    fr_printf("%s: invalid parameter", dacName);
  }
}
