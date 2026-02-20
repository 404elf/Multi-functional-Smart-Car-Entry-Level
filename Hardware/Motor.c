#include "stm32f10x.h"                  // Device header
#include "sys.h"
#include "Motor.h"
#define MAX_SPEED 7200
//电机的AB相初始化
void MOTOR_Init(void)
{   
	PWM_Motor_Init();
    /*开启时钟*/
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);     
    /*电机AB初始化*/
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Pin =GPIO_Pin_0 | GPIO_Pin_1 |GPIO_Pin_12 | GPIO_Pin_13;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);                    

    /*设置GPIO初始化后的默认电平*/
    GPIO_SetBits(GPIOB, GPIO_Pin_0 | GPIO_Pin_1 |GPIO_Pin_12 | GPIO_Pin_13);             
}

/* 电机PWM引脚初始化 */
void PWM_Motor_Init(void)
{
	/*开启时钟*/
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);			
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);			
	
	/*GPIO初始化*/
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8|GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);							
														
	/*配置时钟源*/
	TIM_InternalClockConfig(TIM1);		
	
	/*时基单元初始化*/
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;				
	TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;     
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up; 
	TIM_TimeBaseInitStructure.TIM_Period = 7200-1;                 //f=10khz T=100us
	TIM_TimeBaseInitStructure.TIM_Prescaler = 0;               
	TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;            
	TIM_TimeBaseInit(TIM1, &TIM_TimeBaseInitStructure);             
	
	/*输出比较初始化*/ 
	TIM_OCInitTypeDef TIM_OCInitStructure;							
	TIM_OCStructInit(&TIM_OCInitStructure);                         
                                                       
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;               
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;       
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;   
	TIM_OCInitStructure.TIM_Pulse = 0;								
	TIM_OC1Init(TIM1, &TIM_OCInitStructure);                        
	TIM_OC4Init(TIM1, &TIM_OCInitStructure);                        
	
	/*TIM使能*/
	TIM_Cmd(TIM1, ENABLE);			
	TIM_CtrlPWMOutputs(TIM1, ENABLE);
}

//设置  电机转速（PWM占空比）  函数
//! 取值范围 (0,100)
void A_PWM(uint16_t Compare)
{   
    if(Compare > 100) Compare =100;
	TIM_SetCompare4(TIM1, Compare*MAX_SPEED/100);		
}

void B_PWM(uint16_t Compare)
{   
    if(Compare > 100) Compare = 100;
	TIM_SetCompare1(TIM1, Compare*MAX_SPEED/100);		
}

/* 方向控制 */
// 电机方向控制
void Forward(uint16_t speed)
{
    AIN1 = 1;
    AIN2 = 0;
    BIN1 = 1;
    BIN2 = 0;
    B_PWM(speed);
    A_PWM(speed);
}
void Backward(uint16_t speed)
{
    AIN1 = 0;
    AIN2 =1;
    BIN1 = 0;
    BIN2 = 1;
    B_PWM(speed);
    A_PWM(speed);
}
//左右转
void Goward(int16_t left_speed,int16_t right_speed)
{   if (left_speed>0){
        AIN1 = 1;
        AIN2 = 0;
    }
    else{
        left_speed=-left_speed;
        AIN1 = 0;
        AIN2 = 1;
    }
    if (right_speed>0){
        BIN1 = 1;
        BIN2 = 0;
    }
    else{
        right_speed=-right_speed;
        BIN1 = 0;
        BIN2 = 1;
    }
    A_PWM((uint16_t)left_speed);
    B_PWM((uint16_t)right_speed);
}

void Stop(void)
{
    AIN1 = 0;
    AIN2 = 0;
    BIN1 = 0;
    BIN2 = 0;
    B_PWM(0);
    A_PWM(0);
}

// void Rightward(uint16_t left_speed,uint16_t right_speed)
// {
//     AIN1 = 1;
//     AIN2 = 0;
//     BIN1 = 0;
//     BIN2 = 1;
//     B_PWM(left_speed);
//     A_PWM(right_speed);
// }
// void Leftward(uint16_t left_speed,uint16_t right_speed)
// {
//     AIN1 = 0;
//     AIN2 = 1;
//     BIN1 = 1;
//     BIN2 = 0;
//     B_PWM(left_speed);
//     A_PWM(right_speed);
// }

