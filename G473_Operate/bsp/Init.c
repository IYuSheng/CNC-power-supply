/* Init.c 文件 */
#include "Init.h"

void Error_Handler(void);

uint8_t S_F = 0;	// 共享内存，定义操作系统是否启动

/**
  * @brief  初始化硬件
  */
void Init_Hardware(void)
{
  /* ------------------------------ 硬件初始化 ------------------------------------------- */
  /* 1. 初始化HAL库（必须先于所有硬件操作） */
  HAL_Init();

  /* 2. 初始化所有外设（GPIO、UART、SPI等） */
	MX_GPIO_Init();
  UART_Init();
  UART1_Init();
  MX_SPI1_Init();
  MX_USART1_UART_Init();
  MX_USB_Device_Init();
  MX_I2C1_Init();
	MX_TIM2_Init();
  MX_TIM3_Init();
	fr_printf("\r\n[version]HR CNC POWER SUPPLY V1.0");//  当前版本号
	fr_printf("Hardware Init Success");
}

void Init_App(void)
{
  /* ------------------------------ 应用初始化 ------------------------------------------- */
	DWT_Init();
	Gui_Init();
  Key_Init();
  CommandProcessorInit();
  RegisterAllCommands();
  Encoder_Init();
	fr_printf("App Init Success");
}

/**
  * @brief  初始化系统时钟
  */
void SystemClock_Config(void)
{
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
//  LL_CRS_SetSyncDivider(LL_CRS_SYNC_DIV_1);
//  LL_CRS_SetSyncPolarity(LL_CRS_SYNC_POLARITY_RISING);
//  LL_CRS_SetSyncSignalSource(LL_CRS_SYNC_SOURCE_USB);
//  LL_CRS_SetReloadCounter(__LL_CRS_CALC_CALCULATE_RELOADVALUE(48000000,1000));
//  LL_CRS_SetFreqErrorLimit(34);
//  LL_CRS_SetHSI48SmoothTrimming(32);
}

void Error_Handler(void)
{
  __disable_irq();
  while (1)
    {
      fr_printf("System Error\r\n");
    }
}
