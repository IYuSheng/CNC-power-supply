/* Init.h 文件 */
#ifndef __INIT_H
#define __INIT_H

#include "stm32g4xx_ll_utils.h"
#include "stm32g4xx_ll_system.h"
#include "stm32g4xx_ll_bus.h"
#include "stm32g4xx_ll_pwr.h"
#include "stm32g4xx_ll_rcc.h"
#include "stm32g4xx_ll_gpio.h"
#include "usb_device.h"
#include "i2c.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "usb_device.h"
#include "gpio.h"
#include "Task_init.h"
#include "Monitor.h"
#include "Debug.h"
#include "Watchdog.h"
#include "st7789.h"
#include "Key.h"
#include "m24c64.h"
#include "Uart_comm.h"
#include "Uart_Debug.h"
#include "CommandHandlers.h"
#include "Encoder.h"
#include "Comm.h"
#include "lvgl.h"
#include "lv_port_disp.h"
#include "LVGL_Init.h"


/* 系统监控相关定义 */
#if Monitor_Flag
void Init_Monitor(void);
void configureTimerForRuntimeStats(void);
uint32_t getRuntimeCounterValue(void);
#endif

void Init_Hardware(void);
void SystemClock_Config(void);
void Init_App(void);

#endif /* __INIT_H */
