/* Task_init.h 文件 */
#ifndef __TASK_INIT_H
#define __TASK_INIT_H

#include "Init.h"

/* USER CODE BEGIN Includes */
// 定义优先级常量（数值越大优先级越高）
#define TASK_PRIO_WATCHDOG    4    // 看门狗：最高优先级，防止系统死机
#define TASK_PRIO_Comm        3    // 通信任务：与采集端通信，需快速响应
#define TASK_PRIO_CONTROL     3    // 控制任务：实时性要求高，需快速响应
#define TASK_PRIO_ENCODER     3    // 编码器任务：旋钮调参，需快速响应
#define TASK_PRIO_KEY         3    // 按键检测：用户交互需及时响应
#define TASK_PRIO_UART        2    // 串口调试：实时性要求高，需快速响应
#define TASK_PRIO_MONITOR     1    // 系统监控：重要但非实时，可稍后执行
#define TASK_PRIO_STORAGE     1    // 存储操作：I/O密集型，允许适当延迟
#define TASK_PRIO_TFT         1    // 屏幕显示：视觉更新可接受一定延迟
#define TASK_PRIO_Print       1    // 调试接口打印：允许延迟 


void Debug_uart_task_create(void);
void Monitor_task_create(void);
void Comm_task_create(void);
void Watchdog_task_create(void);
void TFT_task_create(void);
void Storage_task_create(void);
void Key_task_create(void);
void Encoder_task_create(void);
void Control_task_create(void);
void Test_task_create(void);

#endif /* __TASK_INIT_H */
