#ifndef _pwm_H
#define _pwm_H

#include "system.h"


extern void TIM8_Init(u16 arr, u16 psc);
extern void servo_angle(int yaw_angle,int pitch_angle);
extern void Servo_Set_Yaw_Angle(float yaw_angle);
extern void Servo_Set_Pitch_Angle(float pitch_angle);
extern void Servo_Yaw_Step(float target_angle, float step_value);
extern void Servo_Pitch_Step(float target_angle, float step_value);  
extern void Servo_SetAngle(float yaw_angle,float pitch_angle);
extern void Servo_Step(float start_angle, float end_angle, float step); 

#endif
