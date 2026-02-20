#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "OLED.h"
#include "MPU6050.h"
#include "sys.h"
#include "Motor.h"
#include "Key.h"
#include "TCRT5000.h"
#include "LED.h"
#include "HC05.h"
#include "HCSR04.h"
#include "Servo.h"
#include "AD.h"
#include "Control.h"
#include "PID.h" 
int main(void){
	/*模块初始化*/
	SysTick_Config(SystemCoreClock / 1000); //配置系统滴答定时器1ms中断
	DWT_Init();
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);		//预先分组
	AD_Init();		//AD初始化
	OLED_Init();	//OLED初始化
	MOTOR_Init();	//电机初始化
	Key_Init();		//按键初始化
	LED_Init();  	//LED初始化
	TCRT5000_Init();//红外对管初始化
	SR04_Init(); 	//HCSR04引脚初始化
	HC05_Init();	//蓝牙模块初始化
	Servo_Init();
	//PID_Init(结构体变量, kp, ki, kd,
	//	基准速度,限制最终输出幅度速度, 限制积分幅度maxInt速度)
	PID_Init(&steerPID, 0.0f, 0.0f, 0.0f,
		 10.0f, 10.0f, 10.0f);
	B_PWM(0);
	A_PWM(0);

	LED = 1; //点亮LED
	/* 主循环 */
	while(1){
		Car_Control_Loop();
	}
}
