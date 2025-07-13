#include "Key.h"
#include "stm32g4xx_ll_gpio.h"

// 按键引脚映射表
static const struct
{
  GPIO_TypeDef* port;
  uint16_t pin;
}

key_pin_map[KEY_MAX] =
{
  {GPIOC, GPIO_PIN_13},  // KEY_ENC_1 (PC13)
  {GPIOC, GPIO_PIN_14},  // KEY_ENC_2 (PC14)
  {GPIOC, GPIO_PIN_15},  // KEY_ENC_3 (PC15)
  {GPIOA, GPIO_PIN_6},   // KEY_FUNC1 (PA6)
  {GPIOA, GPIO_PIN_8},   // KEY_FUNC2 (PA8)
  {GPIOB, GPIO_PIN_1}    // KEY_FUNC3 (PB1)
};

// 按键状态（内部维护）
typedef struct
{
  uint8_t current_state;   // 当前状态（消抖后）
  uint8_t last_state;      // 上一次状态
  uint8_t state_changed;   // 状态变化标志
  uint8_t debounce_cnt;    // 消抖计数器
  uint8_t stable_state;    // 稳定状态
} KeyState_t;

static KeyState_t key_states[KEY_MAX];
static const uint8_t DEBOUNCE_THRESHOLD = 5;  // 消抖阈值（5次连续相同电平）

// 初始化按键
void Key_Init(void)
{
  for (int i = 0; i < KEY_MAX; i++)
    {
      key_states[i].current_state = LL_GPIO_IsInputPinSet(key_pin_map[i].port, key_pin_map[i].pin);
      key_states[i].last_state = key_states[i].current_state;
      key_states[i].state_changed = 0;
      key_states[i].debounce_cnt = 0;
      key_states[i].stable_state = key_states[i].current_state;
    }
}// 读取按键电平（带消抖）
uint8_t Key_Read(Key_TypeDef key)
{
  if (key >= KEY_MAX) return 0;
  return key_states[key].stable_state;
}

// 获取按键状态变化（上升沿或下降沿）
uint8_t Key_GetStateChange(Key_TypeDef key)
{
  if (key >= KEY_MAX) return 0;
  return key_states[key].state_changed;
}

// 重置状态变化标志
void Key_ResetStateChange(Key_TypeDef key)
{
  if (key < KEY_MAX)
    {
      key_states[key].state_changed = 0;
    }
}

// 按键扫描（需在FreeRTOS任务中定期调用，如10ms一次）
void Key_Scan(void)
{
  for (int i = 0; i < KEY_MAX; i++)
    {
      uint8_t current = LL_GPIO_IsInputPinSet(key_pin_map[i].port, key_pin_map[i].pin);

      // 消抖逻辑
      if (current == key_states[i].current_state)
        {
          key_states[i].debounce_cnt++;
          if (key_states[i].debounce_cnt >= DEBOUNCE_THRESHOLD)
            {
              // 状态稳定
              if (key_states[i].stable_state != current)
                {
                  key_states[i].stable_state = current;
                  key_states[i].state_changed = 1;  // 标记状态变化
                }
            }
        }
      else
        {
          // 状态变化，重置计数器
          key_states[i].current_state = current;
          key_states[i].debounce_cnt = 0;
        }// 更新上一次状态
      key_states[i].last_state = current;
    }
}

// 按键扫描任务（10ms周期）
void vKeyScanTask(void *pvParameters)
{

  for (;;)
    {
      Key_Scan();  // 扫描所有按键状态
      vTaskDelay(pdMS_TO_TICKS(1));  // 10ms延时
    }
}
