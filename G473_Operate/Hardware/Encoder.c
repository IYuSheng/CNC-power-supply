#include "Encoder.h"

static Encoder_HandleTypeDef encoders[ENCODER_MAX] = {0};
static SemaphoreHandle_t enc_mutex;

// 引脚定义
#define S3_PIN     	 LL_GPIO_PIN_14
#define S4_PIN       LL_GPIO_PIN_15
#define PB_PORT      GPIOB

// 外部中断编码器添加专用变量，避免在中断中访问数组
static volatile uint32_t ss2_total_count = 0;
static volatile uint8_t last_s3_level = 0;
static volatile uint8_t last_s4_level = 0;

/**
 * @brief 初始化所有外部中断引脚
 */
static void Encoder_Exti_Init(void)
{
  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
  LL_EXTI_InitTypeDef EXTI_InitStruct = {0};

  // 1. 使能时钟
  LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_GPIOB);  // PB14/PB15
  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_SYSCFG); // SYSCFG

  GPIO_InitStruct.Mode = LL_GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_UP;
  GPIO_InitStruct.Pin = S3_PIN | S4_PIN;
  LL_GPIO_Init(PB_PORT, &GPIO_InitStruct);

  // 4. 配置EXTI映射
  // PB14 -> EXTI14
  SYSCFG->EXTICR[3] &= ~SYSCFG_EXTICR4_EXTI14_Msk;  // EXTI14位于第4个EXTICR寄存器
  SYSCFG->EXTICR[3] |= SYSCFG_EXTICR4_EXTI14_PB;     // 映射到GPIOB
  // PB15 -> EXTI15
  SYSCFG->EXTICR[3] &= ~SYSCFG_EXTICR4_EXTI15_Msk;
  SYSCFG->EXTICR[3] |= SYSCFG_EXTICR4_EXTI15_PB;

  // 5. 配置EXTI中断
  EXTI_InitStruct.Line_0_31 = LL_EXTI_LINE_14 | LL_EXTI_LINE_15;
  EXTI_InitStruct.Mode = LL_EXTI_MODE_IT;                // 中断模式
  EXTI_InitStruct.Trigger = LL_EXTI_TRIGGER_FALLING;     // 下降沿触发
  EXTI_InitStruct.LineCommand = ENABLE;
  LL_EXTI_Init(&EXTI_InitStruct);

  // 6. 配置中断优先级
  NVIC_SetPriority(EXTI15_10_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY + 1);  // PB14/15共用此中断线

  // 7. 使能中断
  NVIC_EnableIRQ(EXTI15_10_IRQn);  // 使能EXTI10-15中断
}

/**
 * @brief PB14/PB15中断回调（EXTI10-15共用）
 */
void EXTI15_10_IRQHandler(void)
{
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;

  // 处理PB14(EXTI14)中断
  if (LL_EXTI_IsActiveFlag_0_31(LL_EXTI_LINE_14))
    {
      LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_14);
      last_s3_level = LL_GPIO_IsInputPinSet(PB_PORT, S3_PIN);

      uint8_t s4_level = LL_GPIO_IsInputPinSet(PB_PORT, S4_PIN);
      if (s4_level)
        {
          ss2_total_count++;
        }
      else
        {
          ss2_total_count--;
        }
    }

  // 处理PB15(EXTI15)中断
  if (LL_EXTI_IsActiveFlag_0_31(LL_EXTI_LINE_15))
    {
      LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_15);
      last_s4_level = LL_GPIO_IsInputPinSet(PB_PORT, S4_PIN);
      uint8_t s3_level = LL_GPIO_IsInputPinSet(PB_PORT, S3_PIN);
      if (s3_level)
        {
          ss2_total_count--;
        }
      else
        {
          ss2_total_count++;
        }
    }
  portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

/**
 * @brief 初始化编码器
 */
void Encoder_Init(void)
{
  // 初始化互斥锁
  enc_mutex = xSemaphoreCreateMutex();
  configASSERT(enc_mutex != NULL);

  // 初始化TIM2和TIM3
  LL_TIM_EnableCounter(TIM2);
  encoders[ENCODER_TIM2].last_cnt = LL_TIM_GetCounter(TIM2);
	
	LL_TIM_EnableCounter(TIM3);
	encoders[ENCODER_TIM3].last_cnt = LL_TIM_GetCounter(TIM3);
	
  // 初始化外部中断
  Encoder_Exti_Init();

  // 初始化引脚电平
  last_s3_level = LL_GPIO_IsInputPinSet(PB_PORT, S3_PIN);
  last_s4_level = LL_GPIO_IsInputPinSet(PB_PORT, S4_PIN);
  ss2_total_count = 0;
}

/**
 * @brief 处理TIM2编码器
 */
static void Encoder_ProcessTIM2(void)
{
  uint32_t current_cnt = LL_TIM_GetCounter(TIM2);
  int16_t diff = (int16_t)(current_cnt - encoders[ENCODER_TIM2].last_cnt);

  encoders[ENCODER_TIM2].total_count += diff;
  encoders[ENCODER_TIM2].last_cnt = current_cnt;
}

/**
 * @brief 处理TIM3编码器
 */
static void Encoder_ProcessTIM3(void)
{
  uint32_t current_cnt = LL_TIM_GetCounter(TIM3);
  int16_t diff = (int16_t)(current_cnt - encoders[ENCODER_TIM3].last_cnt);

  encoders[ENCODER_TIM3].total_count += diff;
  encoders[ENCODER_TIM3].last_cnt = current_cnt;
}

/**
 * @brief 获取原始计数值
 */
uint32_t Encoder_GetRawCount(Encoder_ID id)
{
  configASSERT(id < ENCODER_MAX);
  uint32_t raw_cnt = 0;

  if (xSemaphoreTake(enc_mutex, portMAX_DELAY) == pdTRUE)
    {
      switch (id)
        {
        case ENCODER_TIM2:
          raw_cnt = LL_TIM_GetCounter(TIM2);
          break;
        case ENCODER_TIM3:
          raw_cnt = LL_TIM_GetCounter(TIM3);
          break;
				case ENCODER_SS2:
          raw_cnt = ss2_total_count;
					break;
        default:
          raw_cnt = 0;
          break;
        }
      xSemaphoreGive(enc_mutex);
    }
  return raw_cnt;
}

/**
 * @brief 获取编码器数据
 */
void Encoder_GetData(Encoder_ID id, Encoder_HandleTypeDef *data)
{
  configASSERT(id < ENCODER_MAX && data != NULL);
  if (xSemaphoreTake(enc_mutex, portMAX_DELAY) == pdTRUE)
    {
      *data = encoders[id];
      
      // 对于外部中断编码器，需要从专用变量复制数据
      if (id == ENCODER_SS2) {
        data->total_count = ss2_total_count;
        data->last_s3_level = last_s3_level;
        data->last_s4_level = last_s4_level;
      }
      
      xSemaphoreGive(enc_mutex);
    }
}

void Encoder_SetData(Encoder_ID id, const Encoder_HandleTypeDef *data)
{
  configASSERT(id < ENCODER_MAX && data != NULL);  // 参数校验

  // 加锁保证线程安全（与读取/处理函数互斥）
  if (xSemaphoreTake(enc_mutex, portMAX_DELAY) == pdTRUE)
  {
    switch (id)
    {
      case ENCODER_TIM2:
        // 1. 设置定时器当前计数（硬件计数器）
        LL_TIM_SetCounter(TIM2, data->last_cnt);  // 同步硬件计数
        // 2. 更新累计计数和历史值
        encoders[id].total_count = data->total_count;
        encoders[id].last_cnt = data->last_cnt;  // 同步历史计数
        break;

      case ENCODER_TIM3:
        // 与TIM2处理逻辑一致
        LL_TIM_SetCounter(TIM3, data->last_cnt);
        encoders[id].total_count = data->total_count;
        encoders[id].last_cnt = data->last_cnt;
        break;

      case ENCODER_SS2:  // 外部中断型编码器（SS2）
        // 更新专用变量
        ss2_total_count = data->total_count;
        last_s3_level = data->last_s3_level;
        last_s4_level = data->last_s4_level;
        
        // 同时更新数组中的副本
        encoders[id].total_count = data->total_count;
        encoders[id].last_s3_level = data->last_s3_level;
        encoders[id].last_s4_level = data->last_s4_level;
        break;

      default:
        break;
    }
    xSemaphoreGive(enc_mutex);  // 释放锁
  }
}

/**
 * @brief 编码器处理任务
 */
void vEncoderTask(void *argument)
{
  for (;;)
    {
      Encoder_ProcessTIM2();
      Encoder_ProcessTIM3();
		
      vTaskDelay(pdMS_TO_TICKS(20));
    }
}
