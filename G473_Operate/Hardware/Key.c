#include "Key.h"

// 按键映射（按枚举顺序对应具体引脚）
static const char* key_names[KEY_MAX] =
{
  "KEY_ENC_1 (PC13)",
  "KEY_ENC_2 (PC14)",
  "KEY_ENC_3 (PC15)",
  "KEY_FUNC1 (PA6)",
  "KEY_FUNC2 (PA8)",
  "KEY_FUNC3 (PB1)"
};

QueueHandle_t control_msg_queue;  // 消息队列句柄

// 按键状态结构体
typedef struct
{
  uint8_t current_state;   // 当前实时电平
  uint8_t last_state;      // 上一次的电平
  uint8_t state_changed;   // 状态变化标志
  uint8_t debounce_cnt;    // 防抖计数
  uint8_t stable_state;    // 防抖后稳定状态
  uint32_t press_time;     // 按下时间戳（用于长按检测）
  uint8_t long_press_detected; // 长按检测标志
} KeyState_t;

static KeyState_t key_states[KEY_MAX];
static const uint8_t DEBOUNCE_THRESHOLD = 10;  // 防抖阈值（增加到10个周期更稳定）
static const uint32_t LONG_PRESS_TIME = 1000; // 长按时间阈值（毫秒）

// 初始化按键GPIO及状态
void Key_Init(void)
{
  // 初始化按键状态并直接读取按键初始电平
  key_states[KEY_ENC_1].stable_state = LL_GPIO_IsInputPinSet(GPIOC, LL_GPIO_PIN_13);
  key_states[KEY_ENC_2].stable_state = LL_GPIO_IsInputPinSet(GPIOC, LL_GPIO_PIN_14);
  key_states[KEY_ENC_3].stable_state = LL_GPIO_IsInputPinSet(GPIOC, LL_GPIO_PIN_15);
  key_states[KEY_FUNC1].stable_state = LL_GPIO_IsInputPinSet(GPIOA, LL_GPIO_PIN_6);
  key_states[KEY_FUNC2].stable_state = LL_GPIO_IsInputPinSet(GPIOA, LL_GPIO_PIN_8);
  key_states[KEY_FUNC3].stable_state = LL_GPIO_IsInputPinSet(GPIOB, LL_GPIO_PIN_1);

  // 初始化按键状态变量
  for (int i = 0; i < KEY_MAX; i++)
    {
      key_states[i].current_state = key_states[i].stable_state;
      key_states[i].last_state = key_states[i].stable_state;
      key_states[i].state_changed = 0;
      key_states[i].debounce_cnt = 0;
      key_states[i].press_time = 0;
      key_states[i].long_press_detected = 0;
    }

  // 创建消息队列
  control_msg_queue = xQueueCreate(10, 64);
  configASSERT(control_msg_queue != NULL);
}

// 读取按键状态的稳定电平
uint8_t Key_Read(Key_TypeDef key)
{
  if (key >= KEY_MAX) return 0;
  return key_states[key].stable_state;
}

// 获取按键状态变化标志
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

// 检查是否为长按
uint8_t Key_IsLongPressed(Key_TypeDef key)
{
  if (key >= KEY_MAX) return 0;
  return key_states[key].long_press_detected;
}

// 重置长按检测标志
void Key_ResetLongPress(Key_TypeDef key)
{
  if (key < KEY_MAX)
    {
      key_states[key].long_press_detected = 0;
    }
}

// 处理单个按键的状态变化，内部调用
static void Key_ProcessOne(Key_TypeDef key, uint8_t current)
{
  uint32_t current_time = xTaskGetTickCount() * portTICK_PERIOD_MS;
  
  // 如果逻辑电平连续10个周期相同则认为稳定
  if (current == key_states[key].current_state)
    {
      key_states[key].debounce_cnt++;
      if (key_states[key].debounce_cnt >= DEBOUNCE_THRESHOLD)
        {
          // 稳定状态变化时处理
          if (key_states[key].stable_state != current)
            {
              key_states[key].stable_state = current;
              key_states[key].state_changed = 1;

              // 按键事件处理
              char msg[64];
              if (current == 0)    // 按下（按键按下时为低电平）
                {
                  key_states[key].press_time = current_time; // 记录按下时间
                  key_states[key].long_press_detected = 0;   // 重置长按标志
                  snprintf(msg, sizeof(msg), "[KEY] %s on\n", key_names[key]);
                }
              else      // 释放
                {
                  // 检查是否为短按（按下时间较短）
                  if ((current_time - key_states[key].press_time) < LONG_PRESS_TIME)
                    {
                      snprintf(msg, sizeof(msg), "[KEY] %s off (short)\n", key_names[key]);
                    }
                  else
                    {
                      // 长按已经在其他地方处理过了
                      snprintf(msg, sizeof(msg), "[KEY] %s off\n", key_names[key]);
                    }
                  key_states[key].press_time = 0; // 清除按下时间
                }
              xQueueSend(control_msg_queue, msg, 0);  // 发送到队列
            }
        }
    }
  else
    {
      // 电平变化，重置防抖计数器
      key_states[key].current_state = current;
      key_states[key].debounce_cnt = 0;
    }
  
  // 长按检测
  if (key_states[key].stable_state == 0 && key_states[key].press_time > 0) 
    {
      // 按键处于按下状态
      if ((current_time - key_states[key].press_time) >= LONG_PRESS_TIME && 
          !key_states[key].long_press_detected)
        {
          key_states[key].long_press_detected = 1;
          char msg[64];
          snprintf(msg, sizeof(msg), "[KEY] %s long\n", key_names[key]);
          xQueueSend(control_msg_queue, msg, 0);
        }
    }
    
  key_states[key].last_state = current;
}

// 按键扫描任务（轮询+状态检测）
void Key_Scan(void)
{
  // 逐个扫描每个按键，直接指定端口和引脚号
  // 1. KEY_ENC_1 (PC13)
  uint8_t current = LL_GPIO_IsInputPinSet(GPIOC, LL_GPIO_PIN_13);
  Key_ProcessOne(KEY_ENC_1, current);

  // 2. KEY_ENC_2 (PC14)
  current = LL_GPIO_IsInputPinSet(GPIOC, LL_GPIO_PIN_14);
  Key_ProcessOne(KEY_ENC_2, current);

  // 3. KEY_ENC_3 (PC15)
  current = LL_GPIO_IsInputPinSet(GPIOC, LL_GPIO_PIN_15);
  Key_ProcessOne(KEY_ENC_3, current);

  // 4. KEY_FUNC1 (PA6)
  current = LL_GPIO_IsInputPinSet(GPIOA, LL_GPIO_PIN_6);
  Key_ProcessOne(KEY_FUNC1, current);

  // 5. KEY_FUNC2 (PA8)
  current = LL_GPIO_IsInputPinSet(GPIOA, LL_GPIO_PIN_8);
  Key_ProcessOne(KEY_FUNC2, current);

  // 6. KEY_FUNC3 (PB1)
  current = LL_GPIO_IsInputPinSet(GPIOB, LL_GPIO_PIN_1);
  Key_ProcessOne(KEY_FUNC3, current);
}

// 按键扫描任务
void vKeyScanTask(void *pvParameters)
{
  for (;;)
    {
      Key_Scan();
      vTaskDelay(pdMS_TO_TICKS(20));  // 20ms扫描一次（适当增加扫描间隔）
    }
}
