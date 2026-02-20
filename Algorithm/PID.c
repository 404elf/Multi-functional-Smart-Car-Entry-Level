#include "stm32f10x.h" // Device header
#include "PID.h"
#include "Motor.h"
#include "TCRT5000.h"
#include "sys.h"
#define MAX_SPEED 7200
PID_Controller steerPID;
/* PID初始化 */
void PID_Init(PID_Controller *pid, float kp, float ki,
              float kd, float basic, float maxOut, float maxInt)
{
    TIM4_timer_Init();
    pid->Kp = kp;
    pid->Ki = ki;
    pid->Kd = kd;
    pid->basic_speed = basic;
    pid->MaxOutput = maxOut;
    pid->MaxIntegral = maxInt;

    pid->Error = 0;
    pid->LastError = 0;
    pid->Integral = 0;
    pid->Target = 0;
    pid->Actual = 0;
    pid->Output = 0;
}

/* detaT采集数据 */
//10ms一次
void TIM4_timer_Init(void)
{
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);

    TIM_InternalClockConfig(TIM4);

    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
    TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInitStructure.TIM_Period = 7200 - 1;
    TIM_TimeBaseInitStructure.TIM_Prescaler = 100 - 1;
    TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(TIM4, &TIM_TimeBaseInitStructure);

    TIM_ClearFlag(TIM4, TIM_FLAG_Update);
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_Init(&NVIC_InitStructure);

    TIM_ITConfig(TIM4, TIM_IT_Update, ENABLE);

    TIM_Cmd(TIM4, ENABLE);
}
/* PID算法 */
//丢个误差进去才能算（后面有函数用于量化误差）
float PID_Compute(PID_Controller *pid, float target, float actual)
{
    pid->Target = target;
    pid->Actual = actual;

    // 误差
    pid->Error = pid->Target - pid->Actual;

    // KP
    float pTerm = pid->Kp * pid->Error;

    // KI与限幅（不要超过改变量太多）
    pid->Integral += pid->Error;
    if (pid->Integral > pid->MaxIntegral)
        pid->Integral = pid->MaxIntegral;
    if (pid->Integral < -pid->MaxIntegral)
        pid->Integral = -pid->MaxIntegral;
    float iTerm = pid->Ki * pid->Integral;

    // KD
    float dTerm = pid->Kd * (pid->Error - pid->LastError);

    // 输出PID
    pid->Output = pTerm + iTerm + dTerm;

    // 限幅
    if (pid->Output > pid->MaxOutput)
        pid->Output = pid->MaxOutput;
    if (pid->Output < -pid->MaxOutput)
        pid->Output = -pid->MaxOutput;

    // 记录
    pid->LastError = pid->Error;

    return pid->Output;
}

// 记录上一次的误差，用于丢线保护
volatile float last_error = 0;

//量化误差
float Get_Digital_Error(uint8_t status)
{
    // 定义权重
    float weights[4] = {-3.0f, -1.0f, 1.0f, 3.0f};

    float sum_weighted = 0.0f;
    int active_count = 0;

    // 遍历
    for (int i = 0; i < 4; i++)
    {
        if ((status >> i) & 0x01)
        {
            sum_weighted += weights[i];
            active_count++;
        }
    }
    // 丢线保护
    if (active_count == 0)
    {
        return last_error;
    }

    // 误差计算
    float current_error = sum_weighted / active_count;

    // 返回
    last_error = current_error;
    return current_error;
}
/* 处理误差数据 */
// 递归式滑动平均
#define N 5
float clean_error(float current_error)
{
    static float sum = 0;
    static float arr[N] = {0};
    static uint8_t index = 0;
    static uint8_t count = 0;
    float del_error = arr[index];
    arr[index] = current_error;
    index = (index + 1) % N;
    // 直至塞满数组
    if (count < N)
    {
        count++;
        sum += current_error;
        return sum / count;
    }
    // 数组满载状态
    sum -= del_error;
    sum += current_error;
    return sum / N;
}
/* 执行PID控制层 */
void TIM4_IRQHandler(void)
{
    if (TIM_GetITStatus(TIM4, TIM_IT_Update) != RESET)
    {
        // 只在MODE5启用
        if (Mode == 5)
        {
            int current_status = TCRT5000_Read_Raw();
            float current_error = Get_Digital_Error(current_status);
            float proce_error = clean_error(current_error);
            float steerAdjust = PID_Compute(&steerPID, 0, proce_error);
            // 最终输出
            Goward(steerPID.basic_speed + steerAdjust, steerPID.basic_speed - steerAdjust);
        }
        TIM_ClearITPendingBit(TIM4, TIM_IT_Update);
    }
}
