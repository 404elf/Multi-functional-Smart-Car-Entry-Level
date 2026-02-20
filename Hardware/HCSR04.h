#ifndef __HCSR04_H
#define __HCSR04_H

#include "sys.h"
#define HC_SR04    PAout(0)    //HCSR04´¥·¢½Å

extern volatile uint8_t  TIM2CH2_CAPTURE_STA; 
extern volatile uint16_t TIM2CH2_CAPTURE_VAL;
extern volatile float Distance;
void SR04_Trigger(uint32_t g_time);
float SR04_Revice(float T);
extern volatile uint32_t time;

void SR04_Init(void);
float SR04_Dist (float T);

#endif
