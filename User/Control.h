#ifndef __CONTROL_H
#define __CONTROL_H
void Car_Control_Loop(void);
void Do_Stop_Task(void);
void Do_Sound_Task(void);
void Do_Remote_Task(void);
void Do_Avoid_Task(void);
void Do_Track_Task(void);
void PID_Track_Task(void);
void OLED_Display(void);
void key_check(void);
uint16_t AD_data(void);


#endif
