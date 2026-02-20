#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "HCSR04.h"
#include "sys.h"
volatile uint8_t  TIM2CH3_CAPTURE_STA = 0; 
volatile uint16_t TIM2CH3_CAPTURE_VAL = 0;

volatile uint32_t time=0;
volatile float Distance=0;

/* 超声波模块HC-SR04的初始化 */
//测距精度为1us
void SR04_Init(void)
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);			
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);			
	
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);							
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_Init(GPIOA, &GPIO_InitStructure);	
	GPIO_ResetBits(GPIOA, GPIO_Pin_0);
	
	TIM_InternalClockConfig(TIM2);		
	//时基单元初始化
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;				
	TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;     
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up; 
	TIM_TimeBaseInitStructure.TIM_Period = 65536 - 1;               
	TIM_TimeBaseInitStructure.TIM_Prescaler = 72 - 1;               
	TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;            
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseInitStructure);             
	
	TIM_ClearFlag(TIM2, TIM_FLAG_Update);

	//输入捕获初始化
	TIM_ICInitTypeDef TIM_ICInitStructure;							
	TIM_ICInitStructure.TIM_Channel = TIM_Channel_3;				
	TIM_ICInitStructure.TIM_ICFilter = 0xF;							
	TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;		//上升沿触发——将CNT值传递给CCR
	TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;			
	TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;	
	TIM_ICInit(TIM2, &TIM_ICInitStructure);			
		
	NVIC_InitTypeDef NVIC_InitStructure;				
	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;		
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;	
	NVIC_Init(&NVIC_InitStructure);								

	//中断信号（中断更新|中断捕获）
	TIM_ITConfig (TIM2,TIM_IT_Update|TIM_IT_CC3,ENABLE);

	TIM_Cmd(TIM2, ENABLE);
}


/* 超声波的时间测量 */

/**
 * TIM2CH3_CAPTURE_STA：0000 0000 状态寄存器（以后还会经常出现）
 * 7bit捕获完成标志位（下降沿） 6bit捕获开始标志位（上升沿) 5~0bit溢出计数器
 * TIM2CH3_CAPTURE_VAL用于记录CCR的值
 */

void TIM2_IRQHandler(void)
{	//捕获开始但未结束，在此if计数
    if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)
    {
        if ((TIM2CH3_CAPTURE_STA & 0x40) != 0) 		//0x40--0100 0000（&只看6bit）
        {
            if ((TIM2CH3_CAPTURE_STA & 0x80) == 0)	//0x80--1000 0000（&只看7bit）
            {
                if ((TIM2CH3_CAPTURE_STA & 0x3F) == 0x3F) 	//0x3F--0011 1111（&只看0~5bit）
                {
                    TIM2CH3_CAPTURE_STA |= 0x80;	//强行标志结束 	
                    TIM2CH3_CAPTURE_VAL = 0xFFFF;	//溢出计数器 -- 满
                }
                else
                {
                    TIM2CH3_CAPTURE_STA++; 	//溢出计数器++
                }
            }
        }
        TIM_ClearITPendingBit(TIM2, TIM_IT_Update); 
    }
	//捕获开始与结束
    if (TIM_GetITStatus(TIM2, TIM_IT_CC3) != RESET)
    {	//if用于标志捕获结束，else用于标志捕获开始
        if ((TIM2CH3_CAPTURE_STA & 0x80) == 0)
        {	
            if ((TIM2CH3_CAPTURE_STA & 0x40) != 0)
            {
                TIM2CH3_CAPTURE_STA |= 0x80;	//标志捕获结束
                TIM2CH3_CAPTURE_VAL = TIM_GetCapture3(TIM2); 
                
                TIM_OC3PolarityConfig(TIM2, TIM_ICPolarity_Rising); 
            }
            else
            {	//捕获开始的初始化
                TIM2CH3_CAPTURE_STA = 0;                
                TIM2CH3_CAPTURE_VAL = 0;                
                TIM_SetCounter(TIM2, 0);                
                TIM2CH3_CAPTURE_STA |= 0x40;   //标志捕获开始         
                
                TIM_OC3PolarityConfig(TIM2, TIM_ICPolarity_Falling); 
            }
        }
        TIM_ClearITPendingBit(TIM2, TIM_IT_CC3); 
    }
}
//T是环境温度，这里返回距离cm
float SR04_Dist (float T)
{	
	HC_SR04 = 1;
	Delay_us (13);
	HC_SR04=0;
	if (TIM2CH3_CAPTURE_STA&0X80)
	{	
		time=TIM2CH3_CAPTURE_STA&0X3F;
		time*=65536;
		time+=TIM2CH3_CAPTURE_VAL;
        Distance = (time * (331.4f + 0.607f * T)) / 20000.0f;
		TIM2CH3_CAPTURE_STA=0;		//读取完后才清零，等待下一次捕获
	}
	return Distance;
}
//这里设置了定时发送

/* 超声波触发 */
void SR04_Trigger(uint32_t g_time)
{	static uint32_t trigger_timer=0;
	if (g_time-trigger_timer>50){	//每50ms触发一次
		HC_SR04 = 1;

		Delay_us (13); 	//Delay重复配置时钟导致冲突
		HC_SR04=0;
		trigger_timer = g_time;
	}
}
/* 超声波读取 */
float SR04_Revice(float T)
{
	if (TIM2CH3_CAPTURE_STA&0X80)
	{	
		time=TIM2CH3_CAPTURE_STA&0X3F;
		time*=65536;
		time+=TIM2CH3_CAPTURE_VAL;
		Distance = (time * (331.4f + 0.607f * T)) / 20000.0f;
		TIM2CH3_CAPTURE_STA=0;	//读取完后才清零，等待下一次捕获
	}
	
	return Distance;
}
