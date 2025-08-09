#include "Control.h"

extern UART_TxStruct send_gather;
extern QueueHandle_t control_msg_queue;  // 复用已有的消息队列
float tp1, tp2;

/**
 * @brief 通用限幅函数（将值限制在[min, max]范围内）
 * @param value 原始值
 * @param min 最小值
 * @param max 最大值
 * @return 限幅后的值
 */
static inline float LimitValue(float value, float min, float max)
{
  if (value < min) return min;
  if (value > max) return max;
  return value;
}

/**
 * @brief 控制任务
 */
void vControlTask(void *argument)
{
  Encoder_HandleTypeDef tim2_data, tim3_data;
  char msg[64];
  float last_dac_a = 0.0f, last_dac_b = 0.0f;
  const int32_t zero_count = 0;
  
  // 初始化
  Encoder_GetData(ENCODER_TIM2, &tim2_data);
  Encoder_GetData(ENCODER_TIM3, &tim3_data);
  tp1 = LimitValue(tim3_data.total_count * Default_Precision, 0.0f, Limit_Current);
  tp2 = LimitValue(tim2_data.total_count * Default_Precision, 0.0f, Limit_Voltage);
  last_dac_a = LimitValue(TransformCurrent(tp1), 0.0f, Limit_DACA);
  last_dac_b = LimitValue(TransformVoltage(tp2), 0.0f, Limit_DACB);

  for (;;)
    {
      // 获取当前编码器数据
      Encoder_GetData(ENCODER_TIM2, &tim2_data);
      Encoder_GetData(ENCODER_TIM3, &tim3_data);
      
      // 计算原始值
      float raw_tp1 = tim3_data.total_count * Default_Precision;
      float raw_tp2 = tim2_data.total_count * Default_Precision;
      
      tp1 = LimitValue(raw_tp1, 0.0f, Limit_Current);
      tp2 = LimitValue(raw_tp2, 0.0f, Limit_Voltage);

      if (raw_tp1 < 0.0f)
      {
        fr_printf("T3 Under0");
        tim3_data.total_count = zero_count;
        Encoder_SetData(ENCODER_TIM3, &tim3_data);
      }
      if (raw_tp2 < 0.0f)
      {
        fr_printf("T2 Under0");
        tim2_data.total_count = zero_count;
        Encoder_SetData(ENCODER_TIM2, &tim2_data);
      }
      
      // 计算转换后的值
      float temp_a = LimitValue(TransformCurrent(tp1), 0.0f, Limit_DACA);
      float temp_b = LimitValue(TransformVoltage(tp2), 0.0f, Limit_DACB);

      // 使用 set_uart_tx_data 函数更新 send_gather
      UART_TxStruct temp_tx_data;
      // 获取当前数据
      temp_tx_data = get_uart_tx_data();
      // 仅更新需要修改的字段
      temp_tx_data.dac_a = temp_a;
      temp_tx_data.dac_b = temp_b;
      // 使用安全方式设置数据
      set_uart_tx_data(&temp_tx_data);

      // 仅当值变化时发送
      if (fabs(temp_a - last_dac_a) > 1e-6f ||
          fabs(temp_b - last_dac_b) > 1e-6f)
        {
          // 格式化消息
          snprintf(msg, sizeof(msg), "Current=%.2fA, Voltage=%.2fV\n",
                   tp1, tp2);

          // 发送到消息队列
          xQueueSend(control_msg_queue, msg, pdMS_TO_TICKS(1));

          // 更新历史值
          last_dac_a = temp_a;
          last_dac_b = temp_b;
        }

      vTaskDelay(pdMS_TO_TICKS(5));
    }
}

/**
 * @brief NTC热敏电阻温度转换函数
 * @param adc_voltage_mv ADC采集的电压值，单位为mV
 * @return 温度值，单位为摄氏度
 */
float ConvertNTCTemperature(uint16_t adc_voltage_mv)
{
  // NTC参数配置（需要根据实际使用的热敏电阻调整）
  const float REFERENCE_RESISTANCE = 10000.0f;  // 参考电阻值，单位欧姆
  const float NOMINAL_RESISTANCE = 10000.0f;    // NTC标称电阻值(25°C时)，单位欧姆
  const float NOMINAL_TEMPERATURE = 25.0f;      // 标称温度，单位摄氏度
  const float B_CONSTANT = 3950.0f;             // B常数
  const float SUPPLY_VOLTAGE = 3300.0f;         // 供电电压，单位mV
  
  // 防止除零错误
  if (adc_voltage_mv >= SUPPLY_VOLTAGE) {
    return -273.15f; // 返回绝对零度以下表示错误
  }
  
  // 计算NTC电阻值
  float ntc_resistance = (REFERENCE_RESISTANCE * adc_voltage_mv) / (SUPPLY_VOLTAGE - adc_voltage_mv);
  
  // 使用Beta方程计算温度
  // 1/T = 1/T0 + 1/B * ln(R/R0)
  float reciprocal_temperature = 1.0f / (NOMINAL_TEMPERATURE + 273.15f) + 
                                (logf(ntc_resistance / NOMINAL_RESISTANCE)) / B_CONSTANT;
  
  // 转换为摄氏度
  float temperature = 1.0f / reciprocal_temperature - 273.15f;
  
  return temperature;
}

// 将设置电压值转换为DAC数字值
inline float TransformVoltage(float voltage)
{
  return voltage * VOLTAGE_CONVERT_COEF;
}

// 将设置电流值转换为DAC数字值
inline float TransformCurrent(float current)
{
  return (current * CURRENT_CONVERT_COEF) + CURRENT_OFFSET;
}

/**
 * @brief 将秒数转换为时、分、秒
 * @param total_seconds 总秒数（输入）
 * @param hours 小时（输出）
 * @param minutes 分钟（输出）
 * @param seconds 秒（输出）
 * @note 函数会处理负数情况，将其转换为0
 */
void ConvertSecondsToHMS(int32_t total_seconds, uint8_t *hours, uint8_t *minutes, uint8_t *seconds)
{
    // 检查输出指针有效性
    if (hours == NULL || minutes == NULL || seconds == NULL) {
        return;
    }
    
    // 处理负数情况
    if (total_seconds < 0) {
        *hours = 0;
        *minutes = 0;
        *seconds = 0;
        return;
    }
    
    // 计算时、分、秒
    *hours = (uint8_t)(total_seconds / 3600);
    int32_t remaining_seconds = total_seconds % 3600;
    *minutes = (uint8_t)(remaining_seconds / 60);
    *seconds = (uint8_t)(remaining_seconds % 60);
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

