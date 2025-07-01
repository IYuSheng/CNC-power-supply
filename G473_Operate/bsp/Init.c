/* Init.c 文件 */
#include "Init.h"

void Error_Handler(void);

/**
  * @brief  初始化硬件
  */
void Init_Hardware(void)
{
  /* ------------------------------ 硬件初始化 ------------------------------------------- */
  /* 1. 初始化HAL库（必须先于所有硬件操作） */
  HAL_Init();

  /* 2. 初始化所有外设（GPIO、UART、SPI等） */
	UART_Init();
	fr_printf("Debug Init Success");
  MX_GPIO_Init();
	fr_printf("GPIO Init Success");
  MX_SPI1_Init();
	fr_printf("SPI1 Init Success");
  MX_USART1_UART_Init();
	fr_printf("USART1 Init Success");
  MX_USB_Device_Init();
	fr_printf("USB_Device Init Success");
  MX_I2C1_Init();
	fr_printf("I2C1 Init Success");
  MX_TIM2_Init();
	fr_printf("TIM2 Init Success");
  MX_TIM3_Init();
	fr_printf("TIM3 Init Success");
  MX_TIM15_Init();
	fr_printf("TIM15 Init Success");
}

/**
  * @brief  初始化系统监控功能任务
  */
void Init_Monitor(void)
{
#if Monitor_Flag
  BaseType_t xReturn;
  
  /* 初始化运行时间统计定时器 */
  configureTimerForRuntimeStats();

  /* 创建系统监控任务 */
  xReturn = xTaskCreate(vSystemMonitorTask, "Monitor", 256,
                        NULL, TASK_PRIO_MONITOR, NULL);
  if (xReturn != pdPASS)
  {
    Error_Handler();
  }
#endif
}


void SystemClock_Config(void)
{
	/* 2. 配置系统时钟（G473核心时钟配置） */
  LL_FLASH_SetLatency(LL_FLASH_LATENCY_4);
  while(LL_FLASH_GetLatency() != LL_FLASH_LATENCY_4)
    {
    }
  LL_PWR_EnableRange1BoostMode();
  LL_RCC_HSE_Enable();
  /* Wait till HSE is ready */
  while(LL_RCC_HSE_IsReady() != 1)
    {
    }

  LL_RCC_HSI48_Enable();
  /* Wait till HSI48 is ready */
  while(LL_RCC_HSI48_IsReady() != 1)
    {
    }

  LL_RCC_PLL_ConfigDomain_SYS(LL_RCC_PLLSOURCE_HSE, LL_RCC_PLLM_DIV_2, 85, LL_RCC_PLLR_DIV_2);
  LL_RCC_PLL_EnableDomain_SYS();
  LL_RCC_PLL_Enable();
  /* Wait till PLL is ready */
  while(LL_RCC_PLL_IsReady() != 1)
    {
    }

  LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL);
  LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_2);
  /* Wait till System clock is ready */
  while(LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL)
    {
    }

  /* Insure 1us transition state at intermediate medium speed clock*/
  for (__IO uint32_t i = (170 >> 1); i !=0; i--);

  /* Set AHB prescaler*/
  LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
  LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_1);
  LL_RCC_SetAPB2Prescaler(LL_RCC_APB2_DIV_1);
  LL_SetSystemCoreClock(170000000);

  /* Update the time base */
  if (HAL_InitTick (TICK_INT_PRIORITY) != HAL_OK)
    {
      Error_Handler();
    }
}

void Error_Handler(void)
{
  __disable_irq();
  while (1)
    {
			// 可添加错误闪烁逻辑
    }
}
