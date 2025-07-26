#include "Control.h"

extern UART_TxStruct send_gather;
extern QueueHandle_t control_msg_queue;  // 复用已有的消息队列

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
  SemaphoreHandle_t send_mutex = xSemaphoreCreateMutex();
  if (send_mutex == NULL)
    {
      fr_printf("send_mutex create failed!\n");
      vTaskDelete(NULL);
      return;
    }

  Encoder_HandleTypeDef tim2_data, tim3_data;
  char msg[64];
  float last_dac_a = 0.0f, last_dac_b = 0.0f;
  float tp1, tp2;
	const int32_t zero_count = 0;

  // 初始化：复用循环中的逻辑
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

      // 更新send_gather
      if (xSemaphoreTake(send_mutex, portMAX_DELAY) == pdPASS)
        {
          send_gather.dac_a = temp_a;
          send_gather.dac_b = temp_b;

          xSemaphoreGive(send_mutex);
        }

      // 仅当值变化时发送
      if (fabs(send_gather.dac_a - last_dac_a) > 1e-6f ||
          fabs(send_gather.dac_b - last_dac_b) > 1e-6f)
        {
          // 格式化消息
          snprintf(msg, sizeof(msg), "Current=%.2fA, Voltage=%.2fV\n",
                   tp1, tp2);

          // 发送到消息队列
          xQueueSend(control_msg_queue, msg, pdMS_TO_TICKS(1));

          // 更新历史值
          last_dac_a = send_gather.dac_a;
          last_dac_b = send_gather.dac_b;
        }

      vTaskDelay(pdMS_TO_TICKS(5));
    }
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
