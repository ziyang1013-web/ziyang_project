//gps+wifi+云端信息下发+oled+蜂鸣器+压力传感器+oled显示经纬度数据+防盗模式+温湿度

#include "stm32f10x.h"
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "USART2.h"
#include "stdio.h"
#include "key.h"
#include "beep.h"
#include "oled.h"
#include "FSR402.h"
#include "adcx.h"
#include "dht11.h"
//声明
uint8_t Wstr[100];
volatile uint8_t Wi = 0;
uint8_t Wstate = 0;
uint16_t  pressure;
uint8_t alarm_state=0;
uint16_t pressure_Difference=0;
u8 temp;
u8 humi;
void errorLog(int num);
void parseGpsBuffer(void);
void printGpsBuffer(void);
extern void beep_Init(void);

extern void beep(void );
extern void beep_1(void );

    float  lat;
	float  lng;
	float  lat_data;
	float  lng_data;
	uint8_t KeyNum;
int main(void)
{	
	delay_init();

	NVIC_Configuration(); 	 //设置NVIC中断分组2:2位抢占优先级，2位响应优先级
	uart_init(9600);	 //串口初始化为9600
	Init_LEDpin();
	LED1 = 1;
	Serial_Init();//串口2
	clrStruct();
    beep_init();
	OLED_Init();
    FSR402_Init();//压力传感器
	Key_Init();//
	DHT11_Init();//温湿度
	OLED_ShowChinese(0,0,0,16,1);//经
	OLED_ShowChinese(16,0,1,16,1);//度
	//OLED_ShowChar(48,0,14,16,1);//：
	OLED_ShowChinese(0,16,2,16,1);//纬
	OLED_ShowChinese(16,16,3,16,1);//度
	//OLED_ShowChar(48,16,14,16,1);//：

	while(1)
	{
				parseGpsBuffer();
				//printGpsBuffer();
		
        		//pressure = FSR402_GetData();//获取压力值
				//OLED_ShowNum(100,200,pressure,5,8,1);
		
				sscanf (Save_Data .latitude ,"%f",&lat);
				//lat_data=lat/100+0.119400;
			//	lat_data=35.300551;//河工坐标
				sscanf (Save_Data .longitude ,"%f",&lng);
				//lng_data=113.954045;//河工坐标
				//lng_data=lng/100+0.386435;
        
		       // lat_data=35.301301;//河工文一坐标纬度
				//lng_data=113.952020;//河工文一坐标经度
		 
				lat_data=35.297000;//河工创新楼坐标纬度
				lng_data=113.955400;//河工创新楼坐标经度
		
				//分离纬度坐标数据的整数和小数
				int int_lat_data=(int)lat_data;  //取整数
				double frac_lat_data=lat_data-int_lat_data;//取小数
	    		int int_frac_lat_data=frac_lat_data*1000000;
		
				//分离经度坐标数据的整数和小数
				int int_lng_data=(int)lng_data;  //取整数
				double frac_lng_data=lng_data-int_lng_data;//取小数
	    		int int_frac_lng_data=frac_lng_data*1000000;
		if (Wstate == 1)
		{
			
			Wstate = 0;
			LED1 = 0;
			if (strcmp(Wstr, "+MQTTSUBRECV:0,\"attributes/push\",20,{\"alarm_state\":true}\r\n") == 0)//防盗模式开启
			{
				alarm_state=1;
			}
			if (strcmp(Wstr, "+MQTTSUBRECV:0,\"attributes/push\",21,{\"alarm_state\":false}\r\n") == 0)//正常模式开启
			{
				alarm_state=0;
				Serial_Printf("AT+MQTTPUB=0,\"attributes\",\"{ \\\"occupancy\\\": false}\",0,0\r\n");
			}

			if (strcmp(Wstr, "+MQTTSUBRECV:0,\"attributes/push\",14,{\"relay3\":\"1\"}\r\n") == 0)//查看定位
			{
				//alarm_state=0;
				Serial_Printf("AT+MQTTPUB=0,\"attributes\",\"{ \\\"location\\\":{ \\\"lat\\\": %f \\, \\\"lng\\\": %f}}\",0,0\r\n", lat_data , lng_data);
			}
			if (strcmp(Wstr, "+MQTTSUBRECV:0,\"attributes/push\",14,{\"relay3\":\"2\"}\r\n") == 0)//查看温湿度
			{
				DHT11_Read_Data(&temp,&humi);
				Serial_Printf("AT+MQTTPUB=0,\"attributes\",\"{\\\"temperature\\\":%d \\, \\\"humidity\\\":%d}\",0,0\r\n", temp, humi); // 串口2
	
				
			}
			
			//OLED_ShowNum(50,180,alarm_state,1,16,1);
			
			memset(Wstr, '\0', sizeof(Wstr));
		}
		switch (alarm_state )
		{
			case 0:
				OLED_ShowChinese(32,48,4,16,1);//正
			    OLED_ShowChinese(48,48,5,16,1);//常
			    OLED_ShowChinese(64,48,6,16,1);//模
			    OLED_ShowChinese(80,48,7,16,1);//式
				KeyNum = Key_GetNum();
				if (KeyNum == 1)
				{
					Serial_Printf("AT+MQTTUSERCFG=0,1,\"qzy\",\"wg05axqybq2o12gj\",\"s4qiLbls1o\",0,0,\"\"\r\n");
					LED1 =0;
					
					delay_ms(3000);
          		    Serial_Printf("AT+MQTTCONN=0,\"sh-1-mqtt.iot-api.com\",1883,1\r\n");
					delay_ms(3000);
					Serial_Printf("AT+MQTTSUB=0,\"attributes/push\",1\r\n");
			
				}

				if (KeyNum == 4)
				{
			
          		  Serial_Printf("AT+MQTTPUB=0,\"attributes\",\"{ \\\"location\\\":{ \\\"lat\\\": %f \\, \\\"lng\\\": %f}}\",0,0\r\n", lat_data , lng_data);
					LED1 =1;

//					Serial_Printf("AT+MQTTPUB=0,\"attributes\",\"{ \\\"occupancy\\\": true}\",0,0\r\n");
			
					//Serial_Printf("AT+MQTTPUB=0,\"attributes\",\"{ \\\"location\\\":{ \\\"lat\\\": %f \\, \\\"lng\\\": %f} \\,\\\"temperature\\\":%d \\, \\\"humidity\\\":%d}\",0,0\r\n", lat_data,lng_data,2, 2); // 串口2
	     		  // Serial_Printf("AT+MQTTPUB=0,\"attributes\",\"{\\\"temperature\\\":%d \\, \\\"humidity\\\":%d}\",0,0\r\n", 1, 1); // 串口2
		
					//oled显示纬度数据
					OLED_ShowNum(32,0,int_lng_data ,3,16,1);//整数部分
					OLED_DrawPoint(58,14,1);//点
					OLED_ShowNum(64,0,int_frac_lng_data ,6,16,1);//小数部分
					//oled显示经度数据
					OLED_ShowNum(32,16,int_lat_data,3,16,1);
           				 OLED_DrawPoint(58,30,1);//点
					OLED_ShowNum(64,16,int_frac_lat_data ,6,16,1);//小数部分
					delay_ms(3000);
				}
				break ;
		    case 1:
				   OLED_ShowChinese(32,48,8,16,1);//防
			       OLED_ShowChinese(48,48,9,16,1);//盗
			       OLED_ShowChinese(64,48,6,16,1);//模
			       OLED_ShowChinese(80,48,7,16,1);//式
				    pressure = FSR402_GetData();//获取压力值
					delay_ms (1000);
					uint16_t pressure_data=FSR402_GetData();
			        if(pressure>pressure_data)
					{
					pressure_Difference=pressure-pressure_data;
					}
			        if(pressure<pressure_data)
					{
					pressure_Difference=pressure_data-pressure;
					}
					
					//OLED_ShowNum(0,230,pressure,5,8,1);
					//OLED_ShowNum(0,240,pressure_data,5,8,1);
					//OLED_ShowNum(0,250,pressure_Difference,5,8,1);
			 		   if(pressure_Difference>300)
			  		{
					Serial_Printf("AT+MQTTPUB=0,\"attributes\",\"{ \\\"occupancy\\\": true}\",0,0\r\n");
					//oled显示纬度数据
					OLED_ShowNum(32,0,int_lng_data ,3,16,1);//整数部分
					OLED_DrawPoint(58,14,1);//点
					OLED_ShowNum(64,0,int_frac_lng_data ,6,16,1);//小数部分
					//oled显示经度数据
					OLED_ShowNum(32,16,int_lat_data,3,16,1);
          		    OLED_DrawPoint(58,30,1);//点
					OLED_ShowNum(64,16,int_frac_lat_data ,6,16,1);//小数部分
					beep();
	
			    	}

					break ;

				
		}
		
		//Serial_Printf("AT+MQTTPUB=0,\"attributes\",\"{ \\\"location\\\":{ \\\"lat\\\": %f \\, \\\"lng\\\": %f}}\",0,0\r\n", lat_data , lng_data);
		


	}
}

void errorLog(int num)
{
	
	while (1)
	{
	  	printf("ERROR%d\r\n",num);
	}
}

void parseGpsBuffer()
{
	char *subString;
	char *subStringNext;
	char i = 0;
	if (Save_Data.isGetData)
	{
		Save_Data.isGetData = false;
		//printf("**************\r\n");
		//printf(Save_Data.GPS_Buffer);

		
		for (i = 0 ; i <= 6 ; i++)
		{
			if (i == 0)
			{
				if ((subString = strstr(Save_Data.GPS_Buffer, ",")) == NULL)
					errorLog(1);	//解析错误
			}
			else
			{
				subString++;
				if ((subStringNext = strstr(subString, ",")) != NULL)
				{
					char usefullBuffer[2]; 
					switch(i)
					{
						case 1:memcpy(Save_Data.UTCTime, subString, subStringNext - subString);break;	//获取UTC时间
						case 2:memcpy(usefullBuffer, subString, subStringNext - subString);break;	//获取UTC时间
						case 3:memcpy(Save_Data.latitude, subString, subStringNext - subString);break;	//获取纬度信息
						case 4:memcpy(Save_Data.N_S, subString, subStringNext - subString);break;	//获取N/S
						case 5:memcpy(Save_Data.longitude, subString, subStringNext - subString);break;	//获取经度信息
						case 6:memcpy(Save_Data.E_W, subString, subStringNext - subString);break;	//获取E/W

						default:break;
					}

					subString = subStringNext;
					Save_Data.isParseData = true;
					if(usefullBuffer[0] == 'A')
						Save_Data.isUsefull = true;
					else if(usefullBuffer[0] == 'V')
						Save_Data.isUsefull = false;

				}
				else
				{
					errorLog(2);	//解析错误
				}
			}


		}
	}
}

void printGpsBuffer()
{
	if (Save_Data.isParseData)
	{
		Save_Data.isParseData = false;
		
		printf("Save_Data.UTCTime = ");
		printf(Save_Data.UTCTime);
		printf("\r\n");

		if(Save_Data.isUsefull)
		{
			Save_Data.isUsefull = false;
			printf("Save_Data.latitude = ");
			printf(Save_Data.latitude);
			printf("\r\n");


			printf("Save_Data.N_S = ");
			printf(Save_Data.N_S);
			printf("\r\n");

			printf("Save_Data.longitude = ");
			printf(Save_Data.longitude);
			printf("\r\n");

			printf("Save_Data.E_W = ");
			printf(Save_Data.E_W);
			printf("\r\n");
		}
		else
		{
			printf("GPS DATA is not usefull!\r\n");
		}
		
	}
}






