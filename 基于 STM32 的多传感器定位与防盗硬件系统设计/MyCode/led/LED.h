#ifndef __LED_H
#define __LED_H

#include "sys.h"

#define LED1 PCout(13)// PC13

#define LED_1 GPIOC, GPIO_Pin_13
extern void Init_LEDpin(void);
// 控制某个LED的开关
void LED_On(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin);
void LED_Off(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin);

// 翻转LED状态
void LED_Toggle(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin);

#endif


