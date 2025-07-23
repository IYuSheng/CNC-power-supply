#include "Encoder.h"

static Encoder_HandleTypeDef encoders[ENCODER_MAX] = {0};
static SemaphoreHandle_t enc_mutex;

// 引脚定义
#define S3_PIN     	 LL_GPIO_PIN_14
#define S4_PIN       LL_GPIO_PIN_15
#define PB_PORT      GPIOB

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
      xSemaphoreTakeFromISR(enc_mutex, &xHigherPriorityTaskWoken);
      encoders[ENCODER_SS2].last_s3_level = LL_GPIO_IsInputPinSet(PB_PORT, S3_PIN);

      uint8_t s4_level = LL_GPIO_IsInputPinSet(PB_PORT, S4_PIN);
      if (s4_level)
        {
          encoders[ENCODER_SS2].total_count++;
        }
      xSemaphoreGiveFromISR(enc_mutex, &xHigherPriorityTaskWoken);
    }

  // 处理PB15(EXTI15)中断
  if (LL_EXTI_IsActiveFlag_0_31(LL_EXTI_LINE_15))
    {
      LL_EXTI_ClearFlag_0_31(LL_EXTI_LINE_15);
      xSemaphoreTakeFromISR(enc_mutex, &xHigherPriorityTaskWoken);
      encoders[ENCODER_SS2].last_s4_level = LL_GPIO_IsInputPinSet(PB_PORT, S4_PIN);
      uint8_t s3_level = LL_GPIO_IsInputPinSet(PB_PORT, S3_PIN);
      if (s3_level)
        {
          encoders[ENCODER_SS2].total_count--;
        }
      xSemaphoreGiveFromISR(enc_mutex, &xHigherPriorityTaskWoken);
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

  // 初始化TIM3
  LL_TIM_EnableCounter(TIM2);
  encoders[ENCODER_TIM2].last_cnt = LL_TIM_GetCounter(TIM2);
	
	LL_TIM_EnableCounter(TIM3);
	encoders[ENCODER_TIM3].last_cnt = LL_TIM_GetCounter(TIM3);
	
  // 初始化外部中断
  Encoder_Exti_Init();

  // 初始化引脚电平
  encoders[ENCODER_SS2].last_s3_level = LL_GPIO_IsInputPinSet(PB_PORT, S3_PIN);
  encoders[ENCODER_SS2].last_s4_level = LL_GPIO_IsInputPinSet(PB_PORT, S4_PIN);
}

/**
 * @brief 处理TIM2编码器
 */
static void Encoder_ProcessTIM2(void)
{
  uint32_t current_cnt = LL_TIM_GetCounter(TIM2);
  int16_t diff = current_cnt - encoders[ENCODER_TIM2].last_cnt;

  encoders[ENCODER_TIM2].total_count += diff;
  encoders[ENCODER_TIM2].last_cnt = current_cnt;
}

/**
 * @brief 处理TIM3编码器
 */
static void Encoder_ProcessTIM3(void)
{
  uint32_t current_cnt = LL_TIM_GetCounter(TIM3);
  int16_t diff = current_cnt - encoders[ENCODER_TIM3].last_cnt;

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
          raw_cnt = encoders[ENCODER_SS2].total_count;
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
 * @brief 编码器处理任务
 */
void vEncoderTask(void *argument)
{
  for (;;)
    {
      if (xSemaphoreTake(enc_mutex, portMAX_DELAY) == pdTRUE)
        {
					Encoder_ProcessTIM2();
          Encoder_ProcessTIM3();
          xSemaphoreGive(enc_mutex);
        }
		
      vTaskDelay(pdMS_TO_TICKS(10));
    }
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
      xSemaphoreGive(enc_mutex);
    }
}
