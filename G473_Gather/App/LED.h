/* LED.h 文件 */
#ifndef __LED_H
#define __LED_H

#include "stm32g4xx_ll_gpio.h"
#include "Task.h"

extern volatile uint8_t mode;	//工作模式(电压环0-电流环1)
extern volatile uint8_t system_stop_flag;

void LED_State(void);

#endif /* __LED_H */
