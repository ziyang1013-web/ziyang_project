#include "LED.h"

void Init_LEDpin(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);	 //使能PC端口时钟

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;				 // 端口配置
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO口速度为50MHz
	GPIO_Init(GPIOC, &GPIO_InitStructure);					 //根据设定参数初始化GPIOC
	
	GPIO_SetBits(GPIOC,GPIO_Pin_13);					//初始化设置为0
}

// 控制某个LED的开关
void LED_On(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin)
{
    if (GPIOx == GPIOC)
    {
        GPIO_SetBits(GPIOx, GPIO_Pin);
    }
    else if (GPIOx == GPIOE)
    {
        GPIO_ResetBits(GPIOx, GPIO_Pin);
    }
}
void LED_Off(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin)
{
    if (GPIOx == GPIOC)
    {
        GPIO_ResetBits(GPIOx, GPIO_Pin);
    }
    else if (GPIOx == GPIOE)
    {
        GPIO_SetBits(GPIOx, GPIO_Pin);
    }
}

// 翻转LED状态
void LED_Toggle(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin)
{

    if (GPIO_ReadOutputDataBit(GPIOx, GPIO_Pin) == 0)
    {
        LED_Off(GPIOx, GPIO_Pin);
    }
    else
    {
        LED_On(GPIOx, GPIO_Pin);
    }
}