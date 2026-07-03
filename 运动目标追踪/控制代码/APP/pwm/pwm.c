#include "stdio.h"
#include "system.h"
#include "SysTick.h"
#include "pwm.h"


static int previous_Yaw_angle = 0;  // 上一次执行函数的目标角度
static int previous_Pitch_angle = 0;
/*******************************************************************************
* 函 数 名         : TIM8_CH1_PWM_Init
* 函数功能		   : TIM8通道1 PWM初始化函数
* 输    入         : per:重装载值
					 psc:分频系数
* 输    出         : 无
*******************************************************************************/
void TIM8_Init(u16 arr, u16 psc)
{
 
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	GPIO_InitTypeDef  GPIO_InitStructure;
	TIM_OCInitTypeDef TIM_OCInitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM8,ENABLE);  	//TIM8时钟使能    
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);//使能GPIOC时钟
	GPIO_PinAFConfig(GPIOC,GPIO_PinSource6,GPIO_AF_TIM8); //GPIOC6复用为定时器8(这句话也要加上去)
	GPIO_PinAFConfig(GPIOC,GPIO_PinSource7,GPIO_AF_TIM8); //GPIOC7复用为定时器8（这句话也要加上去）
    GPIO_InitStructure.GPIO_Pin =GPIO_Pin_6 |GPIO_Pin_7; //TIM8对应引脚PC6,PC7
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;        //复用功能
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;	//速度100MHz
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;      //推挽复用输出
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;        //上拉
    GPIO_Init(GPIOC,&GPIO_InitStructure);              //初始化PC6,7
    
	/*** Initialize timer 8 || 初始化定时器8 ***/
	//Set the counter to automatically reload //设定计数器自动重装值 
	TIM_TimeBaseStructure.TIM_Period = arr; 
	//Pre-divider //预分频器 
	TIM_TimeBaseStructure.TIM_Prescaler = psc; 	
	//Set the clock split: TDTS = Tck_tim //设置时钟分割:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; 
	//TIM up count mode //TIM向上计数模式	
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; 
	TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
	//Initializes the timebase unit for TIMX based on the parameter specified in TIM_TimeBaseInitStruct
	//根据TIM_TimeBaseInitStruct中指定的参数初始化TIMx的时间基数单位
	TIM_TimeBaseInit(TIM8, &TIM_TimeBaseStructure); 
	
	TIM_OCStructInit(&TIM_OCInitStructure);
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;/*PWM模式*/
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;/*输出*/
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;/*比较极性高*/
	TIM_OCInitStructure.TIM_Pulse = 0;		//CCR
    TIM_OC1Init(TIM8,&TIM_OCInitStructure); 	
    TIM_OC2Init(TIM8,&TIM_OCInitStructure); 	
    TIM_OC1PreloadConfig(TIM8,TIM_OCPreload_Enable);/*输出比较预装载使能*/
	TIM_OC2PreloadConfig(TIM8,TIM_OCPreload_Enable);/*输出比较预装载使能*/
    	// Advanced timer output must be enabled
 
	TIM_ARRPreloadConfig(TIM8,ENABLE);/*自动重载预装载使能*/
    TIM_CtrlPWMOutputs(TIM8,ENABLE);//高级定时器输出必须使能这句
	//Enable timer //使能定时器
	TIM_Cmd(TIM8, ENABLE); 		
}


/*
周期为20ms 
0.5ms——-90°
1  ms——-45°
1.5ms—— -0°
2  ms—— 45°
2.5ms—— 90°
当定时器周期为20ms时 
arr=200   pcs=16800   
TIM8_Init(200,16800);//5ms
高电平时间  t=(crr/arr)*20ms
当ccr为10时对应的高电平为1ms
servo_angle=90/10*crr-135 ==>crr=(servo_angle+135)/9
*/
void servo_angle(int yaw_angle,int pitch_angle)
{
   int ccr;
   if(yaw_angle>90) yaw_angle=90;
   else if(yaw_angle<-90) yaw_angle=-90;
   if(pitch_angle>90) pitch_angle=90;
   else if(pitch_angle<-90) pitch_angle=-90;
   TIM8->CCR1=(yaw_angle+135)/9; //(这个安装刚刚好)
   TIM8->CCR2=(pitch_angle+135-50)/9;//(-50)是为了矫正安装误差
}
/*
周期为20ms 
0.5ms—— 0°
1  ms——45°
1.5ms——90°
2  ms——135°
2.5ms——180°
当定时器周期为20ms时 
arr=200   pcs=16800   
TIM8_Init(200,16800);//5ms
高电平时间  t=(crr/arr)*20ms
当ccr为10时对应的高电平为1ms
servo_angle=90/10*crr-135 ==>crr=(servo_angle+135)/9
完全正：Servo_SetAngle(90,40);	（因为是刚刚接触，所以没有找到正确的脚度就可以安装，所以存在误差）
pitch_angle越小，越往下（但好像不只是180°）
yaw_angle越小，从上往下看顺时针
*/
void Servo_Set_Yaw_Angle(float yaw_angle)
{
	if(yaw_angle>270) yaw_angle=270;
    else if(yaw_angle<0) yaw_angle=0;
	TIM_SetCompare1(TIM8, (yaw_angle * 20/ 270  + 5));
	
}
 
void Servo_Set_Pitch_Angle(float pitch_angle)
{
	
    if(pitch_angle>180) pitch_angle=180;
    else if(pitch_angle<0) pitch_angle=0;
	TIM_SetCompare2(TIM8, (pitch_angle  * 20/ 180 + 5));
}


void Servo_Yaw_Step(float target_angle, float step_value) 
{
    // 舵机当前角度
   int current_angle = previous_Yaw_angle;


    
    while (current_angle != target_angle) 
	{

		

        // 逐步调整角度
        if (current_angle < target_angle) {
            current_angle += step_value;  // 增加步进值
            if (current_angle > target_angle) 
			{
                current_angle = target_angle;
            }
        } else {
            current_angle -= step_value;  // 减小步进值
            if (current_angle < target_angle) 
			{
                current_angle = target_angle;
            }
        }
        Servo_Set_Yaw_Angle(current_angle);
    }
    
    // 更新上一次目标角度
    previous_Yaw_angle = target_angle;
}

void Servo_Pitch_Step(float target_angle, float step_value) 
{
    // 舵机当前角度
   int current_angle = previous_Pitch_angle;
		int is_paused = 0;  // 标记是否已经暂停
    
    while (current_angle != target_angle) {


	

        // 逐步调整角度
        if (current_angle < target_angle) {
            current_angle += step_value;  // 增加步进值
            if (current_angle > target_angle) {
                current_angle = target_angle;
            }
        } else {
            current_angle -= step_value;  // 减小步进值
            if (current_angle < target_angle) {
                current_angle = target_angle;
            }
        }
        Servo_Set_Pitch_Angle(current_angle);
    }
    
    // 更新上一次目标角度
    previous_Pitch_angle = target_angle;
}

