#include "stm32f10x.h"                  
#include "Delay.h"
#include "Key.h"
#include "LED.h"
#include "sys.h"

/* 按键初始化：用于切换模式 */
void Key_Init(void)
{
	/*开启时钟*/
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);		
	
	/*GPIO初始化*/
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode =GPIO_Mode_IPD;			
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;			
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	MY_EXTI_Init();
}

void MY_EXTI_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);		
	
	EXTI_InitTypeDef EXTI_InitStructure;

	
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource12);
	/*EXTI初始化*/
	EXTI_InitStructure.EXTI_Line = EXTI_Line12;					
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;					
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;			
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;		
	EXTI_Init(&EXTI_InitStructure);								
	
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource7);
	EXTI_InitStructure.EXTI_Line = EXTI_Line7;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
	EXTI_Init(&EXTI_InitStructure);
	
	/*NVIC配置*/
	NVIC_InitTypeDef NVIC_InitStructure;						
	NVIC_InitStructure.NVIC_IRQChannel =EXTI15_10_IRQn;		
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;				
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;	
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;			
	NVIC_Init(&NVIC_InitStructure);								
	NVIC_InitStructure.NVIC_IRQChannel =EXTI9_5_IRQn;		
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_Init(&NVIC_InitStructure);
}

volatile uint8_t Key_flag=0;
volatile uint32_t key_timer=0;
void EXTI9_5_IRQHandler(void)
{
	if (EXTI_GetITStatus(EXTI_Line7) == SET)		
	{
		/*如果出现数据乱跳的现象，可再次判断引脚电平，以避免抖动*/
		if (KEY1 == 1)						
		{	Key_flag = 1;		//经典的做法：留标志位，到主函数进一步处理
			key_timer = now;	//更新时间基准
		}
		EXTI_ClearITPendingBit(EXTI_Line7);		
	}
}

void EXTI15_10_IRQHandler(void)
{
	if (EXTI_GetITStatus(EXTI_Line12) == SET)		
	{
		/*如果出现数据乱跳的现象，可再次判断引脚电平，以避免抖动*/
		if (KEY2 == 1)
		{	Key_flag = 2;
			key_timer = now;
		}
		EXTI_ClearITPendingBit(EXTI_Line12);							
	}
}

