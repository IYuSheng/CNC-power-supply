#ifndef CALCULATE_H
#define CALCULATE_H

// 增量式PI控制器结构体（需添加历史误差和输出）
typedef struct
{
  float Kp;               // 比例系数
  float Ki;               // 积分系数
  float output_min;       // 输出最小值限制
  float output_max;       // 输出最大值限制
  float integral_min;     // 积分项最小值限制（可选）
  float integral_max;     // 积分项最大值限制（可选）

  // 历史数据（增量式需要）
  float last_error;       // 上一次误差
  float last_last_error;  // 上上次误差（用于计算误差变化量）
  float last_output;      // 上一次输出（用于叠加增量）
} PI_HandleTypeDef;

// 函数声明
void PI_Init(PI_HandleTypeDef *pi, float Kp, float Ki,
             float output_min, float output_max,
             float integral_min, float integral_max);
float PI_Calculate(PI_HandleTypeDef *pi, float process_value, float target_value);
void PI_Reset(PI_HandleTypeDef *pi);

#endif // CALCULATE_H
