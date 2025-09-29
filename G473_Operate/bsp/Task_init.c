#include "Task_init.h"

BaseType_t xReturn;  /* 保存xTaskCreate返回值 */

extern void Error_Handler(void);

/**
  * @brief  初始化Debug串口调试任务
  */
void Debug_uart_task_create(void)
{
  xReturn = xTaskCreate(vUartProcessTask, "UartProc", 512,
                        NULL, TASK_PRIO_UART, NULL);
  if (xReturn != pdPASS)
    {
      fr_printf("Debug task create Failed");
      Error_Handler();
    }
}

/**
  * @brief  初始化Monitor系统监控任务
  */
void Monitor_task_create(void)
{
#if Monitor_Flag

  /* 初始化运行时间统计定时器 */
  configureTimerForRuntimeStats();
  /* 创建系统监控任务 */
  xReturn = xTaskCreate(vSystemMonitorTask, "Monitor", 512,
                        NULL, TASK_PRIO_MONITOR, NULL);
  if (xReturn != pdPASS)
    {
      fr_printf("Monitor task create Failed");
      Error_Handler();
    }

#endif
}

/**
  * @brief  初始化通信任务
  */
void Comm_task_create(void)
{
  xReturn = xTaskCreate(vUart1ProcessTask, "Comm", 512,
                        NULL, TASK_PRIO_Comm, NULL);
  if (xReturn != pdPASS)
    {
      fr_printf("Comm task create Failed");
      Error_Handler();
    }
}

/**
  * @brief  初始化看门狗任务
  */
void Watchdog_task_create(void)
{
  xReturn = xTaskCreate(vWatchdogTask, "Watchdog", 64,
                        NULL, TASK_PRIO_WATCHDOG, NULL);
  if (xReturn != pdPASS)
    {
      fr_printf("Watchdog task create Failed");
      Error_Handler();
    }
}

/**
  * @brief  初始化按键任务
  */
void Key_task_create(void)
{
  xReturn = xTaskCreate(vKeyScanTask, "Key", 128,
                        NULL, TASK_PRIO_KEY, NULL);
  if (xReturn != pdPASS)
    {
      fr_printf("Key task create Failed");
      Error_Handler();
    }
}

/**
  * @brief  初始化屏幕任务
  */
void TFT_task_create(void)
{
  xReturn = xTaskCreate(vTFTTask, "TFT", 1024,
                        NULL,TASK_PRIO_TFT, NULL);
  if (xReturn != pdPASS)
    {
      fr_printf("TFT task create Failed");
      Error_Handler();
    }
}

/**
  * @brief  初始化存储任务
  */
void Storage_task_create(void)
{
  xReturn = xTaskCreate(vStorageTask, "Storage", 512,
                        NULL, TASK_PRIO_STORAGE, NULL);
  if (xReturn != pdPASS)
    {
      fr_printf("Storage task create Failed");
      Error_Handler();
    }
}

/**
  * @brief  初始化编码器任务
  */
void Encoder_task_create(void)
{
  xReturn = xTaskCreate(vEncoderTask, "Encoder", 512,
                        NULL, TASK_PRIO_ENCODER, NULL);
  if (xReturn != pdPASS)
    {
      fr_printf("Encoder task create Failed");
      Error_Handler();
    }
}

/**
  * @brief  初始化控制任务
  */
void Control_task_create(void)
{
  xReturn = xTaskCreate(vControlTask, "Control", 512,
                        NULL, TASK_PRIO_CONTROL, NULL);
  if (xReturn != pdPASS)
    {
      fr_printf("Control task create Failed");
      Error_Handler();
    }
}

// 新增打印任务（专门处理打印，避免阻塞扫描）
void vPrintTask(void *pvParameters)
{
  char msg[64];
//  uint8_t cycle_count = 0;
  vTaskDelay(pdMS_TO_TICKS(4000)); // 等待系统稳定
  for (;;)
    {
      // 等待队列消息（阻塞，直到有消息）
      if (xQueueReceive(control_msg_queue, msg, pdMS_TO_TICKS(1)) == pdPASS)
        {
          dma_printf("%s", msg);  // 在这里执行打印
        }

      // 获取数据
      // UART_RxStruct comm_data = get_uart_rx_data();

      // // 发送给上位机
      // dma_printf("%d,%d",
      //            comm_data.voltage_out,
      //            comm_data.current_out);

      // // 实时性要求不高，每20次发送完整数据
      // if (++cycle_count >= 20)
      //   {
      //     dma_printf(",%d,%d,%d,%d,%d,%d,%d,%d",
      //                comm_data.voltage_in,
      //                comm_data.current_in,
      //                comm_data.adc_tmp1,
      //                comm_data.adc_tmp2,
      //                comm_data.voltage_12V_in,
      //                comm_data.voltage_5V_in,
      //                comm_data.mode_stop,
      //                comm_data.mode_flag);
      //     cycle_count = 0;
      //   }
      vTaskDelay(pdMS_TO_TICKS(1));
    }
}

/**
  * @brief  初始化测试任务
  */
void Test_task_create(void)
{
  xReturn = xTaskCreate(vPrintTask, "Print", 512,
                        NULL, TASK_PRIO_Print, NULL);
  if (xReturn != pdPASS)
    {
      fr_printf("Test task create Failed");
      Error_Handler();
    }
}

void vUSBTask(void *pvParameters)
{
  // USB任务处理逻辑
  for (;;)
    {
      uint8_t message[] = "Your data here";
      CDC_Transmit_FS(message, strlen((char*)message));
      
      vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void USB_task_create(void)
{
  xReturn = xTaskCreate(vUSBTask, "USB", 512,
                        NULL, TASK_PRIO_USB, NULL);
  if (xReturn != pdPASS)
    {
      fr_printf("USB task create Failed");
      Error_Handler();
    }
}
