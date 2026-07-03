/*******************************************************************************
*                 
*                 		       普中科技
--------------------------------------------------------------------------------
* 实 验 名		 : PWM呼吸灯实验
* 实验说明       : 
* 连接方式       : 
* 注    意		 : PWM驱动程序在pwm.c内
*******************************************************************************/

#include "system.h"
#include "SysTick.h"
#include "led.h"
#include "pwm.h"
#include "key.h"
#include "exti.h"


/*******************************************************************************
* 函 数 名         : main
* 函数功能		   : 主函数
* 输    入         : 无
* 输    出         : 无
*******************************************************************************/
void ms_Delay(uint16_t t_ms)
{
	uint32_t t=t_ms*4000;
	while(t--);
}




int main(void)
{ 
	SysTick_Init(168);
  	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);  //中断优先级分组 分2组
	LED_Init();
	KEY_Init();
	My_EXTI_Init();  //外部中断初始化
	TIM8_Init(200-1,16800-1);//20ms
	Servo_Yaw_Step(0,1);
	Servo_Pitch_Step(0,1);
	delay_ms(4000);
 
	while(1)
	{		
	  
		LED1=!LED1; //LED1状态取反
		Servo_Yaw_Step(0,1);
		Servo_Pitch_Step(0,1);
		delay_ms(2000); 
		Servo_Yaw_Step(50,1);
		Servo_Pitch_Step(20,1);//第一个角度为上下舵机 第二个角度为左右舵机
		delay_ms(2000);
		

		
	}
}


