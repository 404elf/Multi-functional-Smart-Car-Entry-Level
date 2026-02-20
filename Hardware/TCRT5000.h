#ifndef __TCRT5000_H
#define __TCRT5000_H

//读取红外对管
#define HW_1 GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_5)
#define HW_2 GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_4)
#define HW_3 GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_3)
#define HW_4 GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_15)

void TCRT5000_Init(void);
uint8_t TCRT5000_Read_Raw(void);
#endif
