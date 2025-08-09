/* Gui_Change.h 文件 */
#ifndef __GUI_CHANGE_H
#define __GUI_CHANGE_H

#include "lvgl.h"
#include "lv_port_disp.h"
#include "LVGL_Init.h"
#include "Uart_Debug.h"
#include "Uart_comm.h"
#include "Control.h"

// 控制模式枚举（电压环/电流环）
typedef enum {
  Disable_LOOP = 0,  // 关闭环路模式
  Voltage_LOOP = 1,  // 电压环模式
  Current_LOOP = 2   // 电流环模式
} ControlMode;

// 系统状态枚举（运行/停止）
typedef enum {
  Stop = 0,          // 停止状态（默认值0，上电安全状态）
  Run = 1            // 运行状态
} SystemState;

void Gui_Event_Data(void);

#endif /* __GUI_CHANGE_H */
