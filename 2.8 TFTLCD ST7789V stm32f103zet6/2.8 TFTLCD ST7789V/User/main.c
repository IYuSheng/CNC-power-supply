
/**************作者B站UP：Sangk-Hu****************/

/*****************QQ群：690367095******************/

/****关注UP后，私信或者加群获得资料或者其他资料*****/

#include "main.h" 
#include "pic.h" 

int main()
{	
	WRCACE_InitTypedef WRCACE_InitStruct;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOG, ENABLE);
	WRCACE_InitStruct.Adaptive_Brightness_Goal=0x10;
	WRCACE_InitStruct.Color_Enhancement_Cmd=1;
	WRCACE_InitStruct.Color_Enhancement_Extent=11;
	SysTick_Init(72);
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);  //中断优先级分组 分2组
	My_Usart_Init(115200);//可以换成自己的usart相关函数
	ST7789V_LcdInit();
	ST7789V_AdaptiveBrightnessColorEnhancementInit(&WRCACE_InitStruct);
	
	while(1)
	{
		ST7789V_FillLcdScreen(0,0,LCD_W,LCD_H,WHITE,USE_HORIZONTAL);
		ST7789V_LcdShowPicture(30,40,180,240,gImage_per,USE_HORIZONTAL);
		Rough_DelayMs(5000);
		ST7789V_FillLcdScreen(0,0,LCD_W,LCD_H,YELLOW,USE_HORIZONTAL);
		
		ST7789V_LcdShowChinese(0,0,"哔站",RED,WHITE,32,0,USE_HORIZONTAL);
	
		ST7789V_LcdShowString(32*2,0,"UP:Sangk-Hu",RED,WHITE,16,0,USE_HORIZONTAL);
		
		ST7789V_LcdShowString(0,32,"QQqun:690367095",RED,WHITE,32,0,USE_HORIZONTAL);
		Rough_DelayMs(5000);
	}
}
