#ifndef __TASK_H
#define __TASK_H

#include "stm32g4xx_ll_iwdg.h"
#include "Sys_Timer.h"
#include "Uart_Debug.h"
#include "Uart_comm.h"
#include "DAC8562.h"
#include "SGM58031.h"
#include "Common_ADC.h"
#include "Key_Stop.h"
#include "Calculate.h"
#include "LED.h"
#include "math.h"

#define CURRENT_IN  0  	// 输入电压
#define CURRENT_OUT 1  	// 输出电流
#define VOLTAGE_OUT 2  	// 输出电压
#define VOLTAGE_IN  3   // 输出电流

#define PID_BIG_ERROR_THRESHOLD  0.002f   // 大误差阈值
#define PID_SMALL_ERROR_THRESHOLD 0.0001f  // 小误差阈值

#define ADC_Calibration 1.00087f  // ADC设置校准

// 任务ID枚举
typedef enum
{
  TASK_ID_ReadADC,
  TASK_ID_Read_Common_ADC,
  TASK_ID_SetDAC,
  TASK_ID_Comm_Recv,
	TASK_ID_Comm_Send,
  TASK_ID_Debug,
  TASK_ID_Stop,
  TASK_ID_PID,
  TASK_NUM
} TaskID;

// 任务优先级枚举
typedef enum
{
  PRIO_HIGHEST = 0,
  PRIO_HIGH = 1,
  PRIO_MEDIUM = 2,
  PRIO_LOW = 3,
} TaskPrio;

// 任务结构体
typedef struct
{
  TaskID id;
  volatile bool *flag;
  void (*handler)(void);
  TaskPrio prio;
} Task_t;

// 任务标志位
extern volatile bool task_RADC_flag;
extern volatile bool task_SDAC_flag;
extern volatile bool task_Comm_Recv_flag;
extern volatile bool task_Comm_Send_flag;
extern volatile bool task_Debug_flag;
extern volatile bool task_Stop_flag;
extern volatile bool pid_flag;

// 任务列表
extern Task_t tasks[TASK_NUM];

// 控制模式枚举（电压环/电流环）
typedef enum {
  Disable_LOOP = 0,
  Voltage_LOOP = 1,  // 电压环模式
  Current_LOOP = 2   // 电流环模式
} ControlMode;

// 系统状态枚举（运行/停止）
typedef enum {
  Stop = 0,          // 停止状态（默认值0，上电安全状态）
  Run = 1            // 运行状态
} SystemState;

// 全局变量声明（保持不变）
extern UART_TxStruct sensor_data;
extern volatile uint8_t system_stop_flag;
extern float voltages[4];
extern float V_SetDAC_PID;
extern PI_HandleTypeDef voltage_pi;

// 任务处理函数声明
void Task_ReadADC_Handler(void);
void Task_Read_Common_ADC_Handler(void);
void Task_SetDAC_Handler(void);
void Task_Comm_Recv_Handler(void);
void Task_Comm_Send_Handler(void);
void Task_Debug_Handler(void);
void Task_Stop_Handler(void);
void Task_PID_Handler(void);

#endif /* __TASK_H */
