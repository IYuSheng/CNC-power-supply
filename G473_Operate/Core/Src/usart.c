/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    usart.c
  * @brief   This file provides code for the configuration
  *          of the USART instances.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "usart.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/* USART1 init function */

void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  LL_USART_InitTypeDef USART_InitStruct = {0};

  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

  LL_RCC_SetUSARTClockSource(LL_RCC_USART1_CLKSOURCE_PCLK2);

  /* Peripheral clock enable */
  LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_USART1);

  LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_GPIOA);
  /**USART1 GPIO Configuration
  PA9   ------> USART1_TX
  PA10   ------> USART1_RX
  */
  GPIO_InitStruct.Pin = LL_GPIO_PIN_9;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  GPIO_InitStruct.Alternate = LL_GPIO_AF_7;
  LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = LL_GPIO_PIN_10;
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
  GPIO_InitStruct.Alternate = LL_GPIO_AF_7;
  LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* USART1 interrupt Init */
  NVIC_SetPriority(USART1_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),0, 0));
  NVIC_EnableIRQ(USART1_IRQn);

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  USART_InitStruct.PrescalerValue = LL_USART_PRESCALER_DIV1;
  USART_InitStruct.BaudRate = 115200;
  USART_InitStruct.DataWidth = LL_USART_DATAWIDTH_8B;
  USART_InitStruct.StopBits = LL_USART_STOPBITS_1;
  USART_InitStruct.Parity = LL_USART_PARITY_NONE;
  USART_InitStruct.TransferDirection = LL_USART_DIRECTION_TX_RX;
  USART_InitStruct.HardwareFlowControl = LL_USART_HWCONTROL_NONE;
  USART_InitStruct.OverSampling = LL_USART_OVERSAMPLING_16;
  LL_USART_Init(USART1, &USART_InitStruct);
  LL_USART_SetTXFIFOThreshold(USART1, LL_USART_FIFOTHRESHOLD_1_8);
  LL_USART_SetRXFIFOThreshold(USART1, LL_USART_FIFOTHRESHOLD_1_8);
  LL_USART_DisableFIFO(USART1);
  LL_USART_ConfigAsyncMode(USART1);

  /* USER CODE BEGIN WKUPType USART1 */

  /* USER CODE END WKUPType USART1 */

  LL_USART_Enable(USART1);

  /* Polling USART1 initialisation */
  while((!(LL_USART_IsActiveFlag_TEACK(USART1))) || (!(LL_USART_IsActiveFlag_REACK(USART1))))
    {
    }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}
/* USART3 init function */

void MX_USART3_UART_Init(void)
{
  LL_USART_InitTypeDef USART_InitStruct = {0}; // USART配置结构体
  LL_GPIO_InitTypeDef GPIO_InitStruct = {0}; // GPIO配置结构体

  // 修改为USART3的时钟源
  LL_RCC_SetUSARTClockSource(LL_RCC_USART3_CLKSOURCE_PCLK1); // USART3通常在PCLK1上

  // 修改为USART3的时钟使能
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_USART3); // 启用USART3时钟
  LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_GPIOB); // 启用GPIOB时钟

  /* 配置USART3的TX和RX引脚 */
  GPIO_InitStruct.Pin = LL_GPIO_PIN_10 | LL_GPIO_PIN_11; // 选择PB10(TX)和PB11(RX)引脚
  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE; // 设置为复用模式
  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH; // 设置引脚速度
  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL; // 设置输出类型为推挽
  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO; // 设置无上下拉
  GPIO_InitStruct.Alternate = LL_GPIO_AF_7; // 设置复用功能为USART3
  LL_GPIO_Init(GPIOB, &GPIO_InitStruct); // 初始化GPIOB的PB10和PB11引脚

  /* 配置USART3参数 */
  USART_InitStruct.BaudRate = 115200; // 波特率
  USART_InitStruct.DataWidth = LL_USART_DATAWIDTH_8B; // 数据宽度
  USART_InitStruct.StopBits = LL_USART_STOPBITS_1; // 停止位
  USART_InitStruct.Parity = LL_USART_PARITY_NONE; // 无奇偶校验
  USART_InitStruct.TransferDirection = LL_USART_DIRECTION_TX_RX; // 收发方向
  USART_InitStruct.HardwareFlowControl = LL_USART_HWCONTROL_NONE; // 无硬件流控制
  USART_InitStruct.OverSampling = LL_USART_OVERSAMPLING_16; // 超采样设置
  LL_USART_Init(USART3, &USART_InitStruct); // 初始化USART3
  LL_USART_ConfigAsyncMode(USART3); // 配置为异步模式
  LL_USART_Enable(USART3); // 启用USART3
}

///* USART3 init function */

//void MX_USART3_UART_Init(void)
//{

//  /* USER CODE BEGIN USART3_Init 0 */

//  /* USER CODE END USART3_Init 0 */

//  LL_USART_InitTypeDef USART_InitStruct = {0};

//  LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

//  LL_RCC_SetUSARTClockSource(LL_RCC_USART3_CLKSOURCE_PCLK1);

//  /* Peripheral clock enable */
//  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_USART3);

//  LL_AHB2_GRP1_EnableClock(LL_AHB2_GRP1_PERIPH_GPIOB);
//  /**USART3 GPIO Configuration
//  PB10   ------> USART3_TX
//  PB11   ------> USART3_RX
//  */
//  GPIO_InitStruct.Pin = LL_GPIO_PIN_10;
//  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
//  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
//  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
//  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
//  GPIO_InitStruct.Alternate = LL_GPIO_AF_7;
//  LL_GPIO_Init(GPIOB, &GPIO_InitStruct);

//  GPIO_InitStruct.Pin = LL_GPIO_PIN_11;
//  GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
//  GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
//  GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
//  GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
//  GPIO_InitStruct.Alternate = LL_GPIO_AF_7;
//  LL_GPIO_Init(GPIOB, &GPIO_InitStruct);

//  /* USER CODE BEGIN USART3_Init 1 */

//  /* USER CODE END USART3_Init 1 */
//  USART_InitStruct.PrescalerValue = LL_USART_PRESCALER_DIV1;
//  USART_InitStruct.BaudRate = 115200;
//  USART_InitStruct.DataWidth = LL_USART_DATAWIDTH_8B;
//  USART_InitStruct.StopBits = LL_USART_STOPBITS_1;
//  USART_InitStruct.Parity = LL_USART_PARITY_NONE;
//  USART_InitStruct.TransferDirection = LL_USART_DIRECTION_TX_RX;
//  USART_InitStruct.HardwareFlowControl = LL_USART_HWCONTROL_NONE;
//  USART_InitStruct.OverSampling = LL_USART_OVERSAMPLING_16;
//  LL_USART_Init(USART3, &USART_InitStruct);
//  LL_USART_SetTXFIFOThreshold(USART3, LL_USART_FIFOTHRESHOLD_1_8);
//  LL_USART_SetRXFIFOThreshold(USART3, LL_USART_FIFOTHRESHOLD_1_8);
//  LL_USART_DisableFIFO(USART3);
//  LL_USART_ConfigAsyncMode(USART3);

//  /* USER CODE BEGIN WKUPType USART3 */

//  /* USER CODE END WKUPType USART3 */

//  LL_USART_Enable(USART3);

//  /* Polling USART3 initialisation */
//  while((!(LL_USART_IsActiveFlag_TEACK(USART3))) || (!(LL_USART_IsActiveFlag_REACK(USART3))))
//    {
//    }
//  /* USER CODE BEGIN USART3_Init 2 */

//  /* USER CODE END USART3_Init 2 */

//}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
