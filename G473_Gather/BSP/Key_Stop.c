#include "Key_Stop.h"

/**
 * @brief 获取PA8按键当前状态
 * @retval 按键状态: KEY_STOP_PRESSED(按下) 或 KEY_STOP_RELEASED(释放)
 */
Key_Stop_StateTypeDef Key_Stop_GetState(void)
{
  // 读取PA8引脚电平
  // 假设按键按下时为低电平，释放时为高电平(上拉输入)
  if (LL_GPIO_ReadInputPort(GPIOA) & (1 << 8))
    {
      return KEY_STOP_RELEASED;
    }
  else
    {
      return KEY_STOP_PRESSED;
    }
}

/**
 * @brief 检查PA8按键是否按下
 * @retval 1: 按键按下, 0: 按键未按下
 */
uint8_t Key_Stop_IsPressed(void)
{
  return (Key_Stop_GetState() == KEY_STOP_PRESSED) ? 1 : 0;
}

/**
 * @brief 检查PA8按键是否释放
 * @retval 1: 按键释放, 0: 按键未释放
 */
uint8_t Key_Stop_IsReleased(void)
{
  return (Key_Stop_GetState() == KEY_STOP_RELEASED) ? 1 : 0;
}

uint8_t Get_Mode(void)
{
  // 读取GPIOC的13号引脚状态
  // 13号引脚低电平表示电压环，高电平表示电流环
  return LL_GPIO_ReadInputPort(GPIOC) & (1 << 13) ?  Voltage_LOOP:Current_LOOP ;
}
