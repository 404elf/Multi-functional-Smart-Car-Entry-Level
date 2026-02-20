#ifndef __KEY_H
#define __KEY_H


extern volatile uint8_t Key_flag;
extern volatile uint32_t key_timer;
void MY_EXTI_Init(void);
#define KEY0 GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_4)
#define KEY1 GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_7)
#define KEY2 GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_12)


void Key_Init(void);
//uint8_t Key_GetNum(void);

#endif
