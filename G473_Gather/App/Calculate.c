#include "Calculate.h"

/**
 * @brief  内联函数：限制值在指定范围内
 * @param  value: 要限制的值
 * @param  min: 最小值
 * @param  max: 最大值
 * @retval 限制后的值
 */
static inline float min_max(float value, float min, float max)
{
  if (value > max)
    return max;
  else if (value < min)
    return min;
  else
    return value;
}

/**
 * @brief  初始化增量式PI控制器参数
 * @param  pi: PI控制器结构体指针
 * @param  Kp: 比例系数
 * @param  Ki: 积分系数
 * @param  output_min: 输出最小值限制
 * @param  output_max: 输出最大值限制
 * @param  integral_min: 积分项最小值限制（可选，用于限制积分增量）
 * @param  integral_max: 积分项最大值限制（可选）
 * @retval None
 */
void PI_Init(PI_HandleTypeDef *pi, float Kp, float Ki,
             float output_min, float output_max,
             float integral_min, float integral_max)
{
  pi->Kp = Kp;
  pi->Ki = Ki;

  pi->output_min = output_min;
  pi->output_max = output_max;
  pi->integral_min = integral_min;
  pi->integral_max = integral_max;

  // 初始化历史数据
  pi->last_error = 0.0f;
  pi->last_last_error = 0.0f;
  pi->last_output = 0.0f;  // 初始输出
}

/**
 * @brief  执行增量式PI计算
 * @param  pi: PI控制器结构体指针
 * @param  process_value: 过程值（当前值）
 * @param  target_value: 目标值（希望达到的值）
 * @retval 新的输出值（上一次输出 + 增量）
 */
float PI_Calculate(PI_HandleTypeDef *pi, float process_value, float target_value)
{
  // 1. 计算当前误差
  float error = target_value - process_value;

  // 2. 计算增量式PI的输出变化量Δu
  // 公式：Δu = Kp*(e - e_prev) + Ki*e （简化版，忽略微分项）
  float delta_u = pi->Kp * (error - pi->last_error) + pi->Ki * error * 0.005f;
  // （注：0.01f是采样周期，需与Task1的5ms周期一致）

  // 3. 限制增量Δu的范围（可选，避免单次变化过大）
  delta_u = min_max(delta_u, pi->integral_min, pi->integral_max);

  // 4. 计算新输出（上一次输出 + 增量）
  float new_output = pi->last_output + delta_u;

  // 5. 限制输出在允许范围内
  new_output = min_max(new_output, pi->output_min, pi->output_max);

  // 6. 更新历史数据（供下次计算）
  pi->last_last_error = pi->last_error;  // 上上次误差 = 上一次误差
  pi->last_error = error;                // 上一次误差 = 当前误差
  pi->last_output = new_output;          // 上一次输出 = 本次输出

  return new_output;
}

/**
 * @brief  重置增量式PI控制器内部状态
 * @param  pi: PI控制器结构体指针
 * @retval None
 */
void PI_Reset(PI_HandleTypeDef *pi)
{
  pi->last_error = 0.0f;
  pi->last_last_error = 0.0f;
  pi->last_output = 0.0f;  // 重置输出（或根据需求设为目标值）
}
