//(最后版本)(决赛2改，增加功能：可以增加四个人脸,增加声音)一切正常
#include <rtthread.h>
#include "aht10.h"
#include<board.h>
#include <rtdevice.h>
#include<math.h>
#include <drv_lcd.h>

#define PWM_DEV_NAME "pwm3"
#define PWM_DEV_NAME2 "pwm1"//用tim2,ch1,pa5,/tim1,ch4,pe14
#define PWM_DEV_CHAN1 1
#define PWM_DEV_CHAN2 2
struct rt_device_pwm *pwm_dev1_1;//第一张脸的第一个舵机,设备
struct rt_device_pwm *pwm_dev1_2;//第一张脸的第二个舵机,设备

struct rt_device_pwm *pwm_dev2_1;//第二张脸的第一个舵机,设备
struct rt_device_pwm *pwm_dev2_2;//第二张脸的第二个舵机,设备

struct rt_device_pwm *pwm_dev3_1;//第三张脸的第一个舵机,设备
struct rt_device_pwm *pwm_dev3_2;//第三张脸的第二个舵机,设备

struct rt_device_pwm *pwm_dev4_1;//第四张脸的第一个舵机,设备
struct rt_device_pwm *pwm_dev4_2;//第四张脸的第二个舵机,设备

struct rt_device_pwm *pwm_dev_encoder1;
struct rt_device_pwm *pwm_dev_encoder2;//旋转编码器控制舵机设备名
rt_uint32_t period = 20000000;//pwm周期

float length=6;//舵机扇叶长度

 static int angle1=10;//第一张脸的床靠背角度
 static int high1=9;//第一张脸的床高度

 static int angle2=20;//第二张脸的床靠背角度
 static int high2=14;//第二张脸的床高度

 static int angle3=30;//增加第三张脸的床靠背角度
 static int high3=10;//第三张脸的床高度

 static int angle4=40;//增加第四张脸的床靠背角度
 static int high4=13;//第四张脸的床高度

float pi=3.1415926;
/*rt_uint32_t pulse1_1=angle1/180*2000000+500000;//第一张脸的第一个舵机，脉宽
rt_uint32_t A1=atan(high1/length)/pi*180;//高度high1对应的角度
rt_uint32_t pulse1_2=(atan(high1/length)/pi*180)/180*2000000+500000;//第一张脸的第二个舵机，脉宽

rt_uint32_t pulse2_1=angle2/180*2000000+500000;//第一张脸的第一个舵机，脉宽
rt_uint32_t A2=atan(high2/length)/pi*180;//高度high1对应的角度
rt_uint32_t pulse2_2=A2/180*2000000+500000;//第一张脸的第二个舵机，脉宽*/

rt_uint8_t dir1=0;//定义脉宽增减标志
rt_uint8_t dir2=0;//定义脉宽增减标志

static int Encoder_Count=600000;//旋转编码器去改变的脉宽
static int Encoder_Count2=600000;//

rt_thread_t t_servo1;//定义第一张脸线程句柄
rt_thread_t t_servo2;//定义第二张脸线程句柄
//rt_thread_t t_lcd_data1;//定义lcd显示数据1线程句柄
//rt_thread_t t_lcd_data2;//定义lcd显示数据2线程句柄

rt_thread_t t_lcd;//定义lcd显示线程


rt_sem_t servo_dsem1;//定义第一张脸操控舵机的信号量句柄
rt_sem_t servo_dsem2;//定义第二张脸操控舵机的信号量句柄


rt_sem_t servo_dsem_key1;//定义按键left传输按键1信号量句柄
rt_sem_t servo_dsem_key2;//定义按键left传输按键1信号量句柄

rt_sem_t servo_dsem_data1;//定义数据1信号量句柄
rt_sem_t servo_dsem_data2;//定义数据2信号量句柄

struct serial_configure config=RT_SERIAL_CONFIG_DEFAULT;

#define DBG_TAG "main"
#define DBG_LVL         DBG_LOG
#include <rtdbg.h>


#define turn_right_1        GET_PIN(E, 4)     // 控制第一个舵机右转的计次器
#define turn_left_1        GET_PIN(E, 3)     // 控制第一个舵机左转的计次器
#define turn_right_2        GET_PIN(E, 5)     // 控制第一个舵机右转的计次器
#define turn_left_2        GET_PIN(E, 2)     // 控制第一个舵机左转的计次器

/* 配置引脚 */
#define PIN_LED_B       GET_PIN(F, 11)      // PF11 :  LED_B        --> LED
#define PIN_LED_R       GET_PIN(F, 12)      // PF12 :  LED_R        --> LED
#define KEY_LEFT        GET_PIN(C, 0)     // PC0:  KEY0         --> KEY_LEFT
#define KEY_DOWN        GET_PIN(C, 1)      // PC1 :  KEY1         --> KEY_BIN_DOWN
#define KEY_UP          GET_PIN(G, 5)     // PC5:  WK_UP        --> KEY
#define KEY_RIGHT       GET_PIN(G, 4)      // PC4 :  KEY2         --> KEY_RIGHT
#define MAX30100_beep   GET_PIN(E, 12)    //心率血氧超过阈值，报警
#define PIN_BEEP        GET_PIN(B, 0)      // PA1:  BEEP

float humidity, temperature;//温湿度量
aht10_device_t dev;
/* 总线名称温湿度传感器 */
const char *i2c_bus_name = "i2c3";

//串口变量：
#define uart2_name       "uart2"
#define uart3_name       "uart3"
/* 用于接收消息的信号量 */
static struct rt_semaphore rx2_sem;
static rt_device_t serial2;
static struct rt_semaphore rx3_sem;
static rt_device_t serial3;
char uart2_ch;//串口2存储的信息
char uart3_ch;//串口3存储的信息
//串口变量


int num;//按键left按下=1，不按=0
/*-----------声明--------------*/
void led_init(void);
void lcd_config(void);
void uart2_config(void);
void uart3_config(void);
void key_config(void);
void beep(void);
void beep_and_lcd(void);
void write(void);
void key_left_config(void);
void key_down_config(void);
void key(void);
void encoder_config(void);
void encoder_pwm_control();
void encoder_pwm_control_2();
rt_int16_t Encoder_Get(void);
void encoder_lcd(void);
void t_servo1_entry(void);
void t_servo2_entry(void);
void t_servo3_entry(void);
void t_servo4_entry(void);
void temp_humi_init(void);
void temp_humi_lcd(void);
void rx_len_1_config(void);
void rx_len_2_config(void);
void rx_len_3_config(void);
void rx_len_4_config(void);
void deal_uart3_data(void);
void enter_user1(void);
void enter_user2(void);
void enter_user3(void);
void enter_user4(void);
/*-----------声明--------------*/
int a=0;int b=0;int c=0;int d=0;
//旋转编码器中断之后会在回调中将这些变量加加减减，然后在循环中判断，因为回调里改变脉宽会卡死在中断里，只能在循环里执行
void irq_callback1(void *args)
{
    if(rt_pin_read(turn_left_1)==0)
    {
        rt_thread_mdelay(10);
        if(rt_pin_read(turn_right_1)==0)
        {

            Encoder_Count=Encoder_Count -100000;//脉宽

            a=a+1;
            //encoder_pwm_control();
            rt_kprintf("Encoder_Count : %d \n",Encoder_Count);
        }
    }
}
void irq_callback2(void *args)
{
    if(rt_pin_read(turn_right_1)==0)
    {
        //rt_thread_mdelay(10);
        if(rt_pin_read(turn_left_1)==0)
        {
            Encoder_Count=Encoder_Count +100000;


            b=b+1;
            //encoder_pwm_control();
            rt_kprintf("Encoder_Count : %d \n",Encoder_Count);
        }
    }
}

void irq_callback3(void *args)
{
    if(rt_pin_read(turn_left_2)==0)
    {
        rt_thread_mdelay(2);
        if(rt_pin_read(turn_right_2)==0)
        {
            Encoder_Count2=Encoder_Count2 -100000;


            c=c+1;
            //encoder_pwm_control();
            rt_kprintf("Encoder_Count2 : %d \n",Encoder_Count2);
        }
    }
}
void irq_callback4(void *args)
{
    if(rt_pin_read(turn_right_2)==0)
    {
        rt_thread_mdelay(2);
        if(rt_pin_read(turn_left_2)==0)
        {
            Encoder_Count2=Encoder_Count2 +100000;


            d=d+1;
            //encoder_pwm_control();
            rt_kprintf("Encoder_Count2 : %d \n",Encoder_Count2);
        }
    }
}

void t_servo1_entry()//第一张脸的舵机角度入口函数
{
    //rt_sem_take(servo_dsem1,RT_WAITING_FOREVER);//获取第一张脸的舵机信号量
    LOG_D("1");
    pwm_dev1_1=(struct rt_device_pwm *)rt_device_find(PWM_DEV_NAME);//第一张脸的第一个舵机角度

    if(pwm_dev1_1==RT_NULL)
        {

            rt_kprintf("device : %s find failed!\n",PWM_DEV_NAME);


        }
    rt_pwm_enable(pwm_dev1_1, PWM_DEV_CHAN1);
    rt_pwm_set(pwm_dev1_1,PWM_DEV_CHAN1,period,angle1*2000000/180+500000);



    LOG_D("2");
    pwm_dev1_2=(struct rt_device_pwm *)rt_device_find(PWM_DEV_NAME);//第一张脸的第二个舵机角度
        if(pwm_dev1_2==RT_NULL)
            {

                rt_kprintf("device : %s find failed!\n",PWM_DEV_NAME);


            }
        switch (high1)
        {
        case 8 :rt_pwm_set(pwm_dev1_2, PWM_DEV_CHAN2, period, 25*2000000/180+500000);break;//
        case 9 :rt_pwm_set(pwm_dev1_2, PWM_DEV_CHAN2, period, 30*2000000/180+500000);break;//
        case 10 :rt_pwm_set(pwm_dev1_2, PWM_DEV_CHAN2, period, 36*2000000/180+500000);break;//
        case 11:rt_pwm_set(pwm_dev1_2, PWM_DEV_CHAN2, period, 40*2000000/180+500000);break;//
        case 12:rt_pwm_set(pwm_dev1_2, PWM_DEV_CHAN2, period, 48*2000000/180+500000);break;//
        case 13:rt_pwm_set(pwm_dev1_2, PWM_DEV_CHAN2, period, 50*2000000/180+500000);break;//
        case 14:rt_pwm_set(pwm_dev1_2, PWM_DEV_CHAN2, period, 54*2000000/180+500000);break;//
        case 15:rt_pwm_set(pwm_dev1_2, PWM_DEV_CHAN2, period, 60*2000000/180+500000);break;//
        }
        rt_pwm_enable(pwm_dev1_2, PWM_DEV_CHAN2);
        LOG_D("3");
/*        rt_thread_mdelay(3000);
        rt_pwm_disable(pwm_dev1_2, PWM_DEV_CHAN2);*/

}




void t_servo2_entry()//第二张脸的舵机角度入口函数
{
    //rt_sem_take(servo_dsem2,RT_WAITING_FOREVER);//获取第二张脸的舵机信号量

    pwm_dev2_1=(struct rt_device_pwm *)rt_device_find(PWM_DEV_NAME);//第二张脸的第一个舵机角度
    if(pwm_dev2_1==RT_NULL)
        {

            rt_kprintf("device : %s find failed!\n",PWM_DEV_NAME);


        }
    rt_pwm_set(pwm_dev2_1,PWM_DEV_CHAN1,period,angle2*2000000/180+500000);
    rt_pwm_enable(pwm_dev2_1, PWM_DEV_CHAN1);

    pwm_dev2_2=(struct rt_device_pwm *)rt_device_find(PWM_DEV_NAME);//第二张脸的第二个舵机角度
        if(pwm_dev2_2==RT_NULL)
            {
                rt_kprintf("device : %s find failed!\n",PWM_DEV_NAME);
            }
        switch (high2)
        {
        case 8 :rt_pwm_set(pwm_dev2_2, PWM_DEV_CHAN2, period, 25*2000000/180+500000);break;//
        case 9 :rt_pwm_set(pwm_dev2_2, PWM_DEV_CHAN2, period, 30*2000000/180+500000);break;//
        case 10 :rt_pwm_set(pwm_dev2_2, PWM_DEV_CHAN2, period, 36*2000000/180+500000);break;//
        case 11:rt_pwm_set(pwm_dev2_2, PWM_DEV_CHAN2, period, 40*2000000/180+500000);break;//
        case 12:rt_pwm_set(pwm_dev2_2, PWM_DEV_CHAN2, period, 48*2000000/180+500000);break;//
        case 13:rt_pwm_set(pwm_dev2_2, PWM_DEV_CHAN2, period, 50*2000000/180+500000);break;//
        case 14:rt_pwm_set(pwm_dev2_2, PWM_DEV_CHAN2, period, 54*2000000/180+500000);break;//
        case 15:rt_pwm_set(pwm_dev2_2, PWM_DEV_CHAN2, period, 60*2000000/180+500000);break;//
        }
        rt_pwm_enable(pwm_dev2_2, PWM_DEV_CHAN2);

    //rt_sem_take(servo_dsem,RT_WAITING_FOREVER);//获取舵机信号量
}

void t_servo3_entry()//第三张脸的舵机角度入口函数
{
    //rt_sem_take(servo_dsem2,RT_WAITING_FOREVER);//获取第二张脸的舵机信号量

    pwm_dev3_1=(struct rt_device_pwm *)rt_device_find(PWM_DEV_NAME);//第二张脸的第一个舵机角度
    if(pwm_dev3_1==RT_NULL)
        {

            rt_kprintf("device : %s find failed!\n",PWM_DEV_NAME);


        }
    rt_pwm_set(pwm_dev3_1,PWM_DEV_CHAN1,period,angle3*2000000/180+500000);
    rt_pwm_enable(pwm_dev3_1, PWM_DEV_CHAN1);

    pwm_dev3_2=(struct rt_device_pwm *)rt_device_find(PWM_DEV_NAME);//第二张脸的第二个舵机角度
        if(pwm_dev3_2==RT_NULL)
            {
                rt_kprintf("device : %s find failed!\n",PWM_DEV_NAME);
            }
        switch (high3)
        {
        case 8 :rt_pwm_set(pwm_dev3_2, PWM_DEV_CHAN2, period, 25*2000000/180+500000);break;//
        case 9 :rt_pwm_set(pwm_dev3_2, PWM_DEV_CHAN2, period, 30*2000000/180+500000);break;//
        case 10 :rt_pwm_set(pwm_dev3_2, PWM_DEV_CHAN2, period, 36*2000000/180+500000);break;//
        case 11:rt_pwm_set(pwm_dev3_2, PWM_DEV_CHAN2, period, 40*2000000/180+500000);break;//
        case 12:rt_pwm_set(pwm_dev3_2, PWM_DEV_CHAN2, period, 48*2000000/180+500000);break;//
        case 13:rt_pwm_set(pwm_dev3_2, PWM_DEV_CHAN2, period, 50*2000000/180+500000);break;//
        case 14:rt_pwm_set(pwm_dev3_2, PWM_DEV_CHAN2, period, 54*2000000/180+500000);break;//
        case 15:rt_pwm_set(pwm_dev3_2, PWM_DEV_CHAN2, period, 60*2000000/180+500000);break;//
        }
        rt_pwm_enable(pwm_dev3_2, PWM_DEV_CHAN2);

    //rt_sem_take(servo_dsem,RT_WAITING_FOREVER);//获取舵机信号量
}
void t_servo4_entry()//第三张脸的舵机角度入口函数
{
    //rt_sem_take(servo_dsem2,RT_WAITING_FOREVER);//获取第二张脸的舵机信号量

    pwm_dev4_1=(struct rt_device_pwm *)rt_device_find(PWM_DEV_NAME);//第二张脸的第一个舵机角度
    if(pwm_dev4_1==RT_NULL)
        {

            rt_kprintf("device : %s find failed!\n",PWM_DEV_NAME);


        }
    rt_pwm_set(pwm_dev4_1,PWM_DEV_CHAN1,period,angle4*2000000/180+500000);
    rt_pwm_enable(pwm_dev4_1, PWM_DEV_CHAN1);

    pwm_dev4_2=(struct rt_device_pwm *)rt_device_find(PWM_DEV_NAME);//第二张脸的第二个舵机角度
        if(pwm_dev4_2==RT_NULL)
            {
                rt_kprintf("device : %s find failed!\n",PWM_DEV_NAME);
            }
        switch (high4)
        {
        case 8 :rt_pwm_set(pwm_dev4_2, PWM_DEV_CHAN2, period, 25*2000000/180+500000);break;//
        case 9 :rt_pwm_set(pwm_dev4_2, PWM_DEV_CHAN2, period, 30*2000000/180+500000);break;//
        case 10 :rt_pwm_set(pwm_dev4_2, PWM_DEV_CHAN2, period, 36*2000000/180+500000);break;//
        case 11:rt_pwm_set(pwm_dev4_2, PWM_DEV_CHAN2, period, 40*2000000/180+500000);break;//
        case 12:rt_pwm_set(pwm_dev4_2, PWM_DEV_CHAN2, period, 48*2000000/180+500000);break;//
        case 13:rt_pwm_set(pwm_dev4_2, PWM_DEV_CHAN2, period, 50*2000000/180+500000);break;//
        case 14:rt_pwm_set(pwm_dev4_2, PWM_DEV_CHAN2, period, 54*2000000/180+500000);break;//
        case 15:rt_pwm_set(pwm_dev4_2, PWM_DEV_CHAN2, period, 60*2000000/180+500000);break;//
        }
        rt_pwm_enable(pwm_dev4_2, PWM_DEV_CHAN2);

    //rt_sem_take(servo_dsem,RT_WAITING_FOREVER);//获取舵机信号量
}

int k=0;//为了不卡死在中断里，在循环里执行中断所要的目的
/*串口2的回调*/
static rt_err_t uart2_input(rt_device_t dev2, rt_size_t size2)
{
    /* 串口接收到数据后产生中断，调用此回调函数，然后发送接收信号量 */
    rt_sem_release(&rx2_sem);

    rt_device_read(serial2, 0, &uart2_ch, 1);
    if(uart2_ch == '1')
    {

        k++;
    }
    if(uart2_ch == '2')
    {

        k=k+2;
    }
    if(uart2_ch == '3')
    {

        k=k+3;
    }
    if(uart2_ch == '4')
    {

        k=k+4;
    }
    return RT_EOK;
}

static void serial2_thread_entry(void *parameter)
{
rt_sem_take(&rx2_sem, RT_WAITING_FOREVER);
}

/*串口3的回调*/
int j=0;//为了不卡死在中断里，在循环里执行中断所要的目的
int m=0;//在串口回调中区别不同的人脸
int p=0;//和j的功能差不多，用于第二张脸的控制
int q=0;//和j的功能差不多，用于第三张脸的控制
int r=0;//和j的功能差不多，用于第四张脸的控制
  char str3[] = "H\r\n";
static rt_err_t uart3_input(rt_device_t dev3, rt_size_t size3)
{    rt_sem_release(&rx3_sem);
    /* 串口接收到数据后产生中断，调用此回调函数，然后发送接收信号量 */


    rt_device_read(serial3, 0, &uart3_ch, 1);

    if(uart3_ch == 'A')//开始增加第一张人脸
    {

        j++;
        //m=0;
        m++;

    }
    if(uart3_ch == 'B')//增加高度
    {
        switch (m)
        {
        case 1 :j=j+2;break;//如果是增加第1张人脸的话
        case 2 :p=p+2;;break;//如果是增加第2张人脸的话
        case 3 :q=q+2;break;//如果是增加第3张人脸的话
        case 4 :r=r+2;break;//如果是增加第4张人脸的话
        }
    }
    if(uart3_ch == 'C')//减小高度
    {
        switch (m)
        {
        case 1 :j=j+3;break;//如果是增加第1张人脸的话
        case 2 :p=p+3;;break;//如果是增加第2张人脸的话
        case 3 :q=q+3;break;//如果是增加第3张人脸的话
        case 4 :r=r+3;break;//如果是增加第4张人脸的话
        }
    }
    if(uart3_ch == 'D')//增加角度
    {
        switch (m)
        {
        case 1 :j=j+4;break;//如果是增加第1张人脸的话
        case 2 :p=p+4;;break;//如果是增加第2张人脸的话
        case 3 :q=q+4;break;//如果是增加第3张人脸的话
        case 4 :r=r+4;break;//如果是增加第4张人脸的话
        }
    }
    if(uart3_ch == 'E')//减小角度
    {
        switch (m)
        {
        case 1 :j=j+5;break;//如果是增加第1张人脸的话
        case 2 :p=p+5;;break;//如果是增加第2张人脸的话
        case 3 :q=q+5;break;//如果是增加第3张人脸的话
        case 4 :r=r+5;break;//如果是增加第4张人脸的话
        }
    }

    if(uart3_ch == 'F')//开始设置床的高度和角度
    {
        rt_device_write(serial3, 0, &uart3_ch, 1);//串口3给c8t6发信息：F
        j=j+6;
    }
    if(uart3_ch == 'G')//设置完成
    {
        rt_device_write(serial3, 0, &uart3_ch, 1);//串口3给c8t6发信息：G
        j=j+7;
    }
    if(uart3_ch == 'H')//开始增加第二张人脸(录入用户2人脸)
    {

        j=j+9;
        m=0;
        m=m+2;
    }
    if(uart3_ch == 'I')//开始增加第三张人脸(录入用户3人脸)
    {

        j=j+10;
        m=0;
        m=m+3;
    }
    if(uart3_ch == 'J')//开始增加第四张人脸(录入用户4人脸)
    {
/*
        rt_device_write(serial2, 0, str3, 1);//串口2给vision发信息
        rt_device_write(serial3, 0, &uart3_ch, 1);//串口3给c8t6发信息：J
*/
        j=j+11;
        m=0;
        m=m+4;
    }
    return RT_EOK;
}

static void serial3_thread_entry(void *parameter)
{
rt_sem_take(&rx3_sem, RT_WAITING_FOREVER);
}


int w=0;//用于联系按键右和上下按键改变高度角度的关系

char send_str[10]="1\r\n";//测试发送功能
char send_msg[]="11\r\n";//测试发送功能
rt_int16_t encoder_num;
int main(void)
{
        led_init();
        lcd_config();
        uart2_config();
        uart3_config();
        encoder_config();
        temp_humi_init();

    while(1)
    {
    temp_humi_lcd();//实时显示温度湿度
    if(a==1|b==1)//判断旋转编码器是否中断
    {
        //lcd_show_num(80,180,Encoder_Count,1,24);
        encoder_pwm_control();
        a=0;
        b=0;

    }
    if(c==1|d==1)//判断旋转编码器是否中断
    {
        //lcd_show_num(80,200,Encoder_Count2,1,24);
        encoder_pwm_control_2();
        c=0;
        d=0;
    }
    /*if(k==1)
    {

        rx_len_1_config();
        rt_device_write(serial3, 0, &uart2_ch, 1);//串口3给c8t6发信息：1，识别成功为用户1
        //将人脸识别后的值赋给旋转编码器的计数值，为了人脸识别后舵机接着用户人脸识别设置的角度调节，而不是乱跳
        //Encoder_Count=high1*6000000/180+500000;
        Encoder_Count2=angle1*2000000/180+500000;
        k=0;
    }*/
    if(k==1)
    {

        rx_len_2_config();
        rt_device_write(serial3, 0, &uart2_ch, 1);//串口3给c8t6发信息：2，识别成功为用户2
        //Encoder_Count=high2*6000000/180+500000;
        Encoder_Count2=angle2*2000000/180+500000;
        k=0;
    }
    if(k==2)
    {

        rx_len_3_config();
        rt_device_write(serial3, 0, &uart2_ch, 1);//串口3给c8t6发信息：3，识别成功为用户3
        //Encoder_Count=high3*6000000/180+500000;
        Encoder_Count2=angle3*2000000/180+500000;
        k=0;
    }
    if(k==3)
    {

        rx_len_4_config();
        rt_device_write(serial3, 0, &uart2_ch, 1);//串口3给c8t6发信息：4，识别成功为用户4
        //Encoder_Count=high4*6000000/180+500000;
        Encoder_Count2=angle4*2000000/180+500000;
        k=0;
    }
    if(j==1)
    {//录入用户1人脸
        enter_user1();


    }
    if(j==9)
    {//录入用户2人脸
        enter_user2();
    }
    if(j==10)
    {//录入用户3人脸
        enter_user3();
    }
    if(j==11)
    {//录入用户4人脸
        enter_user4();
    }
    if(j==2)
    {
        //第一张人脸的床增加高度
        high1 ++;
        if(high1>15)
        {
            high1=8;
        }
        lcd_fill(130,10,180,40,WHITE);//清除*上半部分
        lcd_show_num(130,20,high1,1,24);
        j=0;
    }
    if(p==2)
    {
        //第二张人脸的床增加高度
        high2 ++;
        if(high2>15)
        {
            high2=8;
        }
        lcd_fill(130,10,180,40,WHITE);//清除*上半部分
        lcd_show_num(130,20,high2,1,24);
        p=0;
    }
    if(q==2)
    {
        //第三张人脸的床增加高度
        high3 ++;
        if(high3>15)
        {
            high3=8;
        }
        lcd_fill(130,10,180,40,WHITE);//清除*上半部分
        lcd_show_num(130,20,high3,1,24);
        q=0;
    }
    if(r==2)
    {
        //第四张人脸的床增加高度
        high4 ++;
        if(high4>15)
        {
            high4=8;
        }
        lcd_fill(130,10,180,40,WHITE);//清除*上半部分
        lcd_show_num(130,20,high4,1,24);
        r=0;
    }
    if(j==3)
    {
        //第二张人脸的床减小高度
        high1 --;
        if(high1<8)
        {
            high1=15;
        }
        lcd_fill(130,10,180,40,WHITE);//清除*上半部分
        lcd_show_num(130,20,high1,1,24);
        j=0;
    }
    if(p==3)
    {
        //第二张人脸的床减小高度
        high2 --;
        if(high2<8)
        {
            high2=15;
        }
        lcd_fill(130,10,180,40,WHITE);//清除*上半部分
        lcd_show_num(130,20,high2,1,24);
        p=0;
    }
    if(q==3)
    {
        //第三张人脸的床减小高度
        high3 --;
        if(high3<8)
        {
            high3=15;
        }
        lcd_fill(130,10,180,40,WHITE);//清除*上半部分
        lcd_show_num(130,20,high3,1,24);
        q=0;
    }
    if(r==3)
    {
        //第四张人脸的床减小高度
        high4 --;
        if(high4<8)
        {
            high4=15;
        }
        lcd_fill(130,10,180,40,WHITE);//清除*上半部分
        lcd_show_num(130,20,high4,1,24);
        r=0;
    }
    if(j==4)
    {
        //录入信息的时候，设置第一张脸增加角度
        angle1 ++;
        lcd_fill(140,40,180,80,WHITE);
        lcd_show_num(140,50,angle1,1,24);
        j=0;
    }
    if(p==4)
    {
        //第二张人脸的床增加角度
        angle2 ++;
        lcd_fill(140,40,180,80,WHITE);
        lcd_show_num(140,50,angle2,1,24);
        p=0;
    }
    if(q==4)
    {
        //第三张人脸的床增加角度
        angle3 ++;
        lcd_fill(140,40,180,80,WHITE);
        lcd_show_num(140,50,angle3,1,24);
        q=0;
    }
    if(r==4)
    {
        //第四张人脸的床增加角度
        angle4 ++;
        lcd_fill(140,40,180,80,WHITE);
        lcd_show_num(140,50,angle4,1,24);
        r=0;
    }
    if(j==5)
    {
        //第一张人脸的床减小角度
        angle1 --;
        lcd_fill(140,40,180,80,WHITE);
        lcd_show_num(140,50,angle1,1,24);
        j=0;
    }
    if(p==5)
    {
        //第二张人脸的床减小角度
        angle2 --;
        lcd_fill(140,40,180,80,WHITE);
        lcd_show_num(140,50,angle2,1,24);
        p=0;
    }
    if(q==5)
    {
        //第三张人脸的床减小角度
        angle3 --;
        lcd_fill(140,40,180,80,WHITE);
        lcd_show_num(140,50,angle3,1,24);
        q=0;
    }
    if(r==5)
    {
        //第四张人脸的床减小角度
        angle4 --;
        lcd_fill(140,40,180,80,WHITE);
        lcd_show_num(140,50,angle4,1,24);
        r=0;
    }
    if(j==6)
    {//开始设置
        lcd_fill(120,210,210,220,RED);//set下划线
        rt_thread_mdelay(800);
        lcd_fill(130,10,180,40,WHITE);//清除*上半部分
        lcd_fill(140,40,200,70,WHITE);//清除*下半部分
        j=0;
    }

    if(j==7)
    {   //设置完成
        lcd_fill(130,10,180,40,WHITE);//清除*上半部分
        lcd_fill(140,40,180,80,WHITE);//清除*下半部分
        lcd_fill(10,170,220,250,WHITE);//清除设置界面，name下半部分
        lcd_fill(70,130,150,180,WHITE);//清除name后部分
        lcd_show_string(130, 20, 24, "*");
        lcd_show_string(140, 50, 24, "*");
        lcd_fill(110,170,115,250,BLACK);//竖划线
        lcd_show_string(10, 180, 24, "add face");
        lcd_show_string(122, 180, 24, "settings");//设置参数
        j=0;
    }
/*    if(rt_pin_read(MAX30100_beep)==0)
    {
        beep();
    }*/
    }
        return RT_EOK;
}





void uart2_config()
{
    /*------------------------------uart2配置begin------------------------------*/
    //char str2[] = "hello uart2!\r\n";

        /* 查找系统中的串口设备 */
           serial2 = rt_device_find(uart2_name);
           if (!serial2)
           {
               rt_kprintf("find %s failed!\n", uart2_name);
               return RT_ERROR;
           }
           /* 初始化信号量 */
           rt_sem_init(&rx2_sem, "rx2_sem", 0, RT_IPC_FLAG_FIFO);

/*
           config.baud_rate = 115200;
           rt_device_control(serial2, RT_DEVICE_CTRL_CONFIG, &config);//配置参数
*/

           /* 以中断接收及轮询发送模式打开串口设备 */
           rt_device_open(serial2, RT_DEVICE_FLAG_INT_RX);
           /* 设置接收回调函数 */
           rt_device_set_rx_indicate(serial2, uart2_input);
           /* 发送字符串 */
           //rt_device_write(serial2, 0, str2, (sizeof(str2) - 1));
           /* 创建 serial 线程 */
           rt_thread_t thread2 = rt_thread_create("serial2", serial2_thread_entry, RT_NULL, 1024, 25, 10);
           /* 创建成功则启动线程 */
               rt_thread_startup(thread2);
       /*------------------------------uart2配置end------------------------------*/

}




void uart3_config()
{
        /*------------------------------uart3配置begin-----------蓝牙-------------------*/
        //char str3[] = "Welcome to use!\r\n";

        /* 查找系统中的串口设备 */
        serial3 = rt_device_find(uart3_name);
        if (!serial3)
        {
            rt_kprintf("find %s failed!\n", uart3_name);
            return RT_ERROR;
        }
        /* 初始化信号量 */
        rt_sem_init(&rx3_sem, "rx3_sem", 0, RT_IPC_FLAG_FIFO);

        config.baud_rate = 9600;
        rt_device_control(serial3, RT_DEVICE_CTRL_CONFIG, &config);//配置参数

        /* 以中断接收及轮询发送模式打开串口设备 */
        rt_device_open(serial3, RT_DEVICE_FLAG_INT_RX);
        /* 设置接收回调函数 */
        rt_device_set_rx_indicate(serial3, uart3_input);
        /* 发送字符串 */
        //rt_device_write(serial3, 0, str3, (sizeof(str3) - 1));
        /* 创建 serial 线程 */
        rt_thread_t thread3 = rt_thread_create("serial3", serial3_thread_entry, RT_NULL, 1024, 25, 10);
        /* 创建成功则启动线程 */
        rt_thread_startup(thread3);


        /*------------------------------uart3配置end------------------------------*/
}


void beep()
{   int i;
    for(i=1;i<10;i++)
    {

        rt_pin_write(PIN_BEEP,PIN_HIGH);

        rt_thread_mdelay(200);
        rt_pin_write(PIN_BEEP,PIN_LOW);

        rt_thread_mdelay(200);
    }
}

void beep_and_lcd()//蜂鸣器响几声并且在lcd上显示write等
{
    rt_pin_mode(PIN_BEEP, PIN_MODE_OUTPUT);
     int i;
    for(i=1;i<10;i++)
    {

        rt_pin_write(PIN_BEEP,PIN_HIGH);
        write();
        //rt_thread_mdelay(60);
        rt_pin_write(PIN_BEEP,PIN_LOW);
        write();
        //rt_thread_mdelay(60);
    }
    lcd_show_string(55, 200, 32, "success!");
}

void encoder_pwm_control()
{
    pwm_dev_encoder1=(struct rt_device_pwm *)rt_device_find(PWM_DEV_NAME);//第一张脸的第一个舵机角度

        rt_pwm_set(pwm_dev_encoder1,PWM_DEV_CHAN1,period,Encoder_Count+500000);

        rt_pwm_enable(pwm_dev_encoder1, PWM_DEV_CHAN1);
}

void encoder_pwm_control_2()
{
    pwm_dev_encoder2=(struct rt_device_pwm *)rt_device_find(PWM_DEV_NAME);//第一张脸的第一个舵机角度

        rt_pwm_set(pwm_dev_encoder2,PWM_DEV_CHAN2,period,Encoder_Count2+500000);

        rt_pwm_enable(pwm_dev_encoder2, PWM_DEV_CHAN2);
}

rt_int16_t Encoder_Get(void)
{
    rt_int16_t Temp;
    Temp = Encoder_Count;
    //Encoder_Count = 0;
    return Temp;
}





void encoder_config()
{
    /* 设置引脚为输入模式 */
    rt_pin_mode(turn_right_1, PIN_MODE_INPUT_PULLUP);
    rt_pin_mode(turn_left_1, PIN_MODE_INPUT_PULLUP);
    rt_pin_mode(turn_right_2, PIN_MODE_INPUT_PULLUP);
    rt_pin_mode(turn_left_2, PIN_MODE_INPUT_PULLUP);
    /* 设置 KEY_LEFT 引脚的模式为输入上拉模式 */
    /*rt_pin_mode(KEY_LEFT, PIN_MODE_INPUT_PULLUP);
    rt_pin_mode(KEY_RIGHT, PIN_MODE_INPUT_PULLUP);
    rt_pin_mode(KEY_UP, PIN_MODE_INPUT_PULLUP);
    rt_pin_mode(KEY_DOWN, PIN_MODE_INPUT_PULLUP);*/

    /* 设置中断模式与中断回调函数 */
    rt_pin_attach_irq(turn_right_1, PIN_IRQ_MODE_FALLING, irq_callback1, RT_NULL);
    rt_pin_attach_irq(turn_left_1, PIN_IRQ_MODE_FALLING, irq_callback2, RT_NULL);
    rt_pin_attach_irq(turn_right_2, PIN_IRQ_MODE_FALLING, irq_callback3, RT_NULL);
    rt_pin_attach_irq(turn_left_2, PIN_IRQ_MODE_FALLING, irq_callback4, RT_NULL);

    /*rt_pin_attach_irq(KEY_LEFT, PIN_IRQ_MODE_FALLING, irq_callback_KEY_LEFT, RT_NULL);//按键中断
    rt_pin_attach_irq(KEY_RIGHT, PIN_IRQ_MODE_FALLING, irq_callback_KEY_RIGHT, RT_NULL);
    rt_pin_attach_irq(KEY_UP, PIN_IRQ_MODE_FALLING, irq_callback_KEY_UP, RT_NULL);
    rt_pin_attach_irq(KEY_DOWN, PIN_IRQ_MODE_FALLING, irq_callback_KEY_DOWN, RT_NULL);*/
    /* 使能中断 */
    rt_pin_irq_enable(turn_right_1, PIN_IRQ_ENABLE);
    rt_pin_irq_enable(turn_left_1, PIN_IRQ_ENABLE);
    rt_pin_irq_enable(turn_right_2, PIN_IRQ_ENABLE);
    rt_pin_irq_enable(turn_left_2, PIN_IRQ_ENABLE);

    /*rt_pin_irq_enable(KEY_LEFT, PIN_IRQ_ENABLE);
    rt_pin_irq_enable(KEY_RIGHT, PIN_IRQ_ENABLE);
    rt_pin_irq_enable(KEY_UP, PIN_IRQ_ENABLE);
    rt_pin_irq_enable(KEY_DOWN, PIN_IRQ_ENABLE);*/

}



void temp_humi_init()
{




        /* 等待传感器正常工作 */
        rt_thread_mdelay(200);

        /* 初始化 aht10 */
        dev = aht10_init(i2c_bus_name);
        if (dev == RT_NULL)
        {
            LOG_E(" The sensor initializes failure");

        }

}

void temp_humi_lcd()
{
    /* 读取湿度 */
    humidity = aht10_read_humidity(dev);

    lcd_show_num(60,110,humidity,1,24);
    /* 读取温度 */
    temperature = aht10_read_temperature(dev);

    lcd_show_num(63,80,temperature,1,24);

}

void rx_len_1_config()
{
    lcd_fill(10,170,220,250,WHITE);//清除设置界面，name下半部分
    // 如果接收到的是一个字节，发送信号量
    beep_and_lcd();
    lcd_show_string(70, 140, 24, "user1 ");
    LOG_D("zhang face");
    lcd_fill(130,10,180,40,WHITE);//清除*上半部分
    lcd_fill(140,40,180,80,WHITE);//清除*下半部分
    lcd_show_num(130,20,high1,1,24);
    lcd_show_num(140,50,angle1,1,24);
    t_servo1_entry();
    rt_thread_mdelay(3000);
    lcd_fill(10,170,220,250,WHITE);//清除设置界面，name下半部分
    lcd_fill(110,170,115,250,BLACK);//竖划线
    lcd_show_string(10, 180, 24, "add face");
    lcd_show_string(122, 180, 24, "settings");//设置参数
    //rt_sem_release(servo_dsem1);
}

void rx_len_2_config()
{
    lcd_fill(10,170,220,250,WHITE);//清除设置界面，name下半部分
    beep_and_lcd();
    lcd_show_string(70, 140, 24, "user1 ");
    LOG_D("jin face");
    t_servo2_entry();
    //rt_sem_release(servo_dsem2);
    lcd_fill(130,10,180,40,WHITE);//清除*上半部分
    lcd_fill(140,40,180,80,WHITE);//清除*下半部分
    lcd_show_num(130,20,high2,1,24);
    lcd_show_num(140,50,angle2,1,24);
    rt_thread_mdelay(3000);
    lcd_fill(10,170,220,250,WHITE);//清除设置界面，name下半部分
    lcd_fill(110,170,115,250,BLACK);//竖划线
    lcd_show_string(10, 180, 24, "add face");
    lcd_show_string(122, 180, 24, "settings");//设置参数

}
void rx_len_3_config()
{
    lcd_fill(10,170,220,250,WHITE);//清除设置界面，name下半部分
    beep_and_lcd();
    lcd_show_string(70, 140, 24, "user2 ");

    t_servo3_entry();
    lcd_fill(130,10,180,40,WHITE);//清除*上半部分
    lcd_fill(140,40,180,80,WHITE);//清除*下半部分
    lcd_show_num(130,20,high3,1,24);
    lcd_show_num(140,50,angle3,1,24);
    rt_thread_mdelay(3000);
    lcd_fill(10,170,220,250,WHITE);//清除设置界面，name下半部分
    lcd_fill(110,170,115,250,BLACK);//竖划线
    lcd_show_string(10, 180, 24, "add face");
    lcd_show_string(122, 180, 24, "settings");//设置参数
}
void rx_len_4_config()
{
    lcd_fill(10,170,220,250,WHITE);//清除设置界面，name下半部分
    beep_and_lcd();
    lcd_show_string(70, 140, 24, "user3 ");

    t_servo4_entry();
    lcd_fill(130,10,180,40,WHITE);//清除*上半部分
    lcd_fill(140,40,180,80,WHITE);//清除*下半部分
    lcd_show_num(130,20,high4,1,24);
    lcd_show_num(140,50,angle4,1,24);
    rt_thread_mdelay(3000);
    lcd_fill(10,170,220,250,WHITE);//清除设置界面，name下半部分
    lcd_fill(110,170,115,250,BLACK);//竖划线
    lcd_show_string(10, 180, 24, "add face");
    lcd_show_string(122, 180, 24, "settings");//设置参数
}
void led_init()
{
    /* 设置 RGB 红灯引脚的模式为输出模式 */
        rt_pin_mode(PIN_LED_B, PIN_MODE_OUTPUT);
        rt_pin_mode(PIN_LED_R, PIN_MODE_OUTPUT);

}
void enter_user1()//人脸录入用户1
{
    lcd_fill(10,210,100,220,RED);//addface下划线
    rt_thread_mdelay(800);
    lcd_fill(10,170,105,250,WHITE);//清除addface半部分
    rt_device_write(serial2, 0, str3, 1);//串口2给vision发信息
    rt_device_write(serial3, 0, str3, 1);//串口3给c8t6发信息：A
    lcd_show_string(10, 180, 24, "begin...");
    rt_thread_mdelay(10000);
    lcd_fill(10,170,105,250,WHITE);//清除addface半部分
    lcd_show_string(10, 180, 24, "add ok!");
    lcd_show_string(70, 140, 24, "user1");

/*    rt_pin_write(MAX30100_beep, 1);
    rt_thread_mdelay(300);
    rt_pin_write(MAX30100_beep, 0);*/
    j=0;
}
void enter_user2()//人脸录入用户2
{
    lcd_fill(10,210,100,220,RED);//addface下划线
    rt_thread_mdelay(800);
    lcd_fill(10,170,105,250,WHITE);//清除addface半部分
    rt_device_write(serial2, 0, str3, 1);//串口2给vision发信息
    rt_device_write(serial3, 0, str3, 1);//串口3给c8t6发信息：H
    lcd_show_string(10, 180, 24, "begin...");
    rt_thread_mdelay(10000);
    lcd_fill(10,170,105,250,WHITE);//清除addface半部分
    lcd_show_string(10, 180, 24, "add ok!");
    lcd_show_string(70, 140, 24, "user1");

    j=0;
}
void enter_user3()//人脸录入用户3
{
    lcd_fill(10,210,100,220,RED);//addface下划线
    rt_thread_mdelay(800);
    lcd_fill(10,170,105,250,WHITE);//清除addface半部分
    rt_device_write(serial2, 0, str3, 1);//串口2给vision发信息
    rt_device_write(serial3, 0, str3, 1);//串口3给c8t6发信息：I
    lcd_show_string(10, 180, 24, "begin...");
    rt_thread_mdelay(10000);
    lcd_fill(10,170,105,250,WHITE);//清除addface半部分
    lcd_show_string(10, 180, 24, "add ok!");
    lcd_show_string(70, 140, 24, "user2");
    j=0;
}
void enter_user4()//人脸录入用户4
{
    lcd_fill(10,210,100,220,RED);//addface下划线
    rt_thread_mdelay(800);
    lcd_fill(10,170,105,250,WHITE);//清除addface半部分
    rt_device_write(serial2, 0, str3, 1);//串口2给vision发信息
    rt_device_write(serial3, 0, str3, 1);//串口3给c8t6发信息：I
    lcd_show_string(10, 180, 24, "begin...");
    rt_thread_mdelay(10000);
    lcd_fill(10,170,105,250,WHITE);//清除addface半部分
    lcd_show_string(10, 180, 24, "add ok!");
    lcd_show_string(70, 140, 24, "user3");
    j=0;
}
