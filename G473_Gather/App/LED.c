#include "LED.h"

void LED_State(void)
{
	if(mode == Current_LOOP)	//电流环模式
	{
		LL_GPIO_ResetOutputPin(GPIOA, LL_GPIO_PIN_11);
		LL_GPIO_SetOutputPin(GPIOB, LL_GPIO_PIN_2);
	}
	else	//电压环模式
	{
		LL_GPIO_ResetOutputPin(GPIOB, LL_GPIO_PIN_2);
		LL_GPIO_SetOutputPin(GPIOA, LL_GPIO_PIN_11);
	}
	
	if(system_stop_flag == Run)	//系统运行
	{
		LL_GPIO_ResetOutputPin(GPIOB, LL_GPIO_PIN_0);
	}
	else	//系统停止
	{
			LL_GPIO_SetOutputPin(GPIOB, LL_GPIO_PIN_0);
	}
}
