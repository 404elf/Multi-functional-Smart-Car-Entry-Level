#ifndef __MOTOR_H
#define __MOTOR_H

#include "stm32f10x.h"                  // Device header
#include "sys.h"

#define AIN1    PBout(13)    // 电机A控制引脚1
#define AIN2    PBout(12)    
#define BIN1    PBout(1)   
#define BIN2    PBout(0)   


void MOTOR_Init(void);
void PWM_Motor_Init(void);
void Forward(uint16_t speed);
void Backward(uint16_t speed);
void Goward(int16_t left_speed,int16_t right_speed);
void Stop(void);
void B_PWM(uint16_t Compare);
void A_PWM(uint16_t Compare);
#endif
