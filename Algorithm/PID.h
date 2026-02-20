#ifndef __PID_H
#define __PID_H

typedef struct {
    // 1. PID 参数 (可以通过调试修改)
    float Kp;
    float Ki;
    float Kd;

    // 2. 目标值与测量值
    float Target;   // 想要达到的值
    float Actual;   // 传感器读到的当前值

    // 3. 误差记录 (核心记忆)
    float Error;        // 当前误差
    float LastError;    // 上次误差 (用于D计算)
    float Integral;     // 误差积分 (用于I计算)

    // 4. 输出限制 (防止小车发疯)
    float basic_speed;  //基础速度
    float MaxOutput;    // PWM最大值 (如 7200)
    float MaxIntegral;  // 积分限幅 (防止积分饱和/Anti-Windup)
    
    float Output;       // 计算后的输出值
} PID_Controller;

extern PID_Controller steerPID;

void PID_Init(PID_Controller *pid, float kp, float ki, float kd,float basic, float maxOut, float maxInt);
float PID_Compute(PID_Controller *pid, float target, float actual);
void TIM4_timer_Init(void);
#endif

