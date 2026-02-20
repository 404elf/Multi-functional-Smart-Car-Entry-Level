#include "stm32f10x.h" // Device header
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

static float g_dist;
volatile static uint32_t last_time = 0;
static uint8_t start_flag = 0;
/* 小车控制主循环 */
void Car_Control_Loop(void)
{
    static MODEID_t Last_Mode = (MODEID_t)-1;
    now = millis();    // 时间快照
    SR04_Trigger(now); // 60ms发送一次
    g_dist = SR04_Revice(15);
    OLED_Display(); // 200ms刷新一次
    key_check();    // 20ms check时间

    // 切换模式初始化
    if (Last_Mode != (MODEID_t)Mode)
    {   Servo_SetAngle(90);
        last_time = now;
        Last_Mode = (MODEID_t)Mode;
        start_flag = 0; // 用于新模式的初始化判断
        g_USART1_FLAG=0;
        // PID清除旧数据
        if ((MODEID_t)Mode == MODE_PID_TRACK)
        {
            steerPID.Integral = 0;
            steerPID.LastError = 0;
        }
    }
    // 模式判断
    switch (Mode)
    {
    case MODE_STOP:
        Do_Stop_Task();
        break;
    case MODE_SOUND_TRACK:
        Do_Sound_Task();
        break;
    case MODE_REMOTE:
        Do_Remote_Task();
        break;
    case MODE_SOUND_AVOID:
        Do_Avoid_Task();
        break;
    case MODE_TRACK:
        Do_Track_Task();
        break;
    case MODE_PID_TRACK:
        PID_Track_Task();
        break;
    }
}
/* 按键判断（消抖处理） */
void key_check(void)
{
    switch (Key_flag)
    {
    case 1:
        if (now - key_timer > 20)
        {
            if (KEY1 == 1)
            {
                switch (Mode)
                {
                case 5:
                    Mode = 0;
                    break;
                default:
                    Mode = Mode + 1;
                    break;
                }
            }
            Key_flag = 0;
        }
        break;
    case 2:
        if (now - key_timer > 20)
        {
            if (KEY2 == 1)
            {
                Mode = 0;
                LED_Turn();
            }
            Key_flag = 0;
        }
        break;
    default:
        Key_flag=0;
        break;
    }
}
/* OLED显示 */
// 显示ADC，距离，模式
void OLED_Display(void)
{ // 字符数组
    char mode_buf[20];
    char dis_buf[20];
    char AD_buf[20];
    static uint32_t olede_timer = 0; // 内置钟表
    if (now - olede_timer > 200)
    {
        olede_timer = now;
        uint16_t AD_vol = AD_data();
        sprintf(mode_buf, "MODE:%d", Mode);
        sprintf(dis_buf, "DIS:%.2fcm", g_dist);
        sprintf(AD_buf, "AD:%d.%03dV", AD_vol / 1000, AD_vol % 1000);
        /* OLED显示 */
        OLED_ShowString(1, 1, mode_buf);
        OLED_ShowString(2, 1, dis_buf);
        OLED_ShowString(3, 1, AD_buf);
        /* 串口显示 */
        UsartPrintf(USART1, "MODE:%d\r\n", Mode);
        UsartPrintf(USART1, "DIS:%.2fcm\r\n", g_dist);
        UsartPrintf(USART1, "AD:%d.%03dV\r\n", AD_vol / 1000, AD_vol % 1000);
    }
}
/* 电压值读取 */
uint16_t AD_data(void)
{
    uint16_t AD_Value = AD_GetValue();
    return (uint16_t)((uint32_t)AD_Value * 3300 / 4095) * 5;
}

/* 模式0：停止 */
void Do_Stop_Task(void)
{ // 仅执行一次
    if (start_flag == 0)
    {
        Stop();
        Servo_SetAngle(90);
        start_flag = 1;
    }
}
/* 模式1：超声波跟随 */
void Do_Sound_Task(void)
{
    static float last_dist = -1;
    static float dist = -1;
    if (start_flag == 0)
    {
        last_dist = g_dist;
        dist = g_dist;
        last_time = now;
        start_flag = 1;
    }
    else
    {
        if (last_dist != g_dist)
        {
            // 距离不一致  -- 采用一阶滤波
            dist = dist * 0.7 + g_dist * 0.3;
            last_dist = g_dist;
        }
        if (dist > 20 && dist <= 30)
        {
            Forward(10);
        }
        else if(dist > 30){
            Forward(15);
        }
        else if (dist < 15 &&dist >=10)
        {
            Backward(10);
        }
        else if (dist<10){
            Backward(13);
        }
        else
            Stop();
    }
}
/* 模式2：蓝牙遥控模式 */
// 逻辑为单点
void Do_Remote_Task(void)
{
    static uint16_t last_flag = 0xFFFF;
    uint32_t rx_time = Serial_timer();
    if (now - rx_time > 15000)
    {
        g_USART1_FLAG = 0; // 超时15s停止
        Stop();
    }
    //? 移动方向切换标志（有啥作用忘记了）--好像是LED~显示
    if (g_USART1_FLAG != last_flag)
    {
        last_flag = g_USART1_FLAG;
        LED = ~LED;
    }
    switch (g_USART1_FLAG)
    {
    case 1: // 前
        Forward(15);
        break;
    case 2: // 后
        Backward(15);
        break;
    case 3: // 右
        Goward(20,-20);
        break;
    case 4: // 左
        Goward(-20,20);
        break;
    case 5: // 停止
        Stop();
        break;
    default:
        Stop();
        break;
    }
}
/* 模式3：超声波避障 */
void Do_Avoid_Task(void)
{
    static float last_dist = -1;
    static float dist = -1;
    static uint8_t step_flag = 0; // 步骤标志位
    static uint8_t last_step_flag = 0;
    // 初始化判断
    if (start_flag == 0)
    {
        last_time = now;
        start_flag = 1;
        last_step_flag = 0;
        dist = g_dist;
    }
    if (last_step_flag != step_flag)
    {
        last_step_flag = step_flag;
        dist = g_dist; // 重置防加权
    }
    // 因为50ms检测一次，所以这里应该也是50ms“真正的判断”一次
    if (last_dist != g_dist)
    {   
        if (g_dist > 100 && last_dist < 40) {
        // 如果距离突然跳得非常远，很有可能是斜角反射，保持旧值或判定为“极近”
        dist = 10; // 强制判定为有障碍
        } 
        else {
        // 距离不一致  -- 采用一阶滤波
            dist = dist * 0.3 + g_dist * 0.7;
            last_dist = g_dist;
        }    
    }
    switch (step_flag)
    {
    case 0:
        Servo_SetAngle(90); // 舵机向前 使超声波朝前方
        if (now - last_time > 300)
        {
            if (dist < 30)
            {
                step_flag = 1;
                last_time = now;
                Stop();
            }
            else
            {
                Forward(15);
            }
        }
        break;
    case 1:
        Servo_SetAngle(70); // 舵机向右 使超声波朝右边
        if (now - last_time > 300)
        {
            if (dist < 30)
            {
                step_flag = 2;
                last_time = now;
            }
            else
            {
                Goward(20,-20);
                step_flag = 10;
            }
        }
        break;
    case 2:
        Servo_SetAngle(110); // 舵机向左 使超声波朝左边
        if (now - last_time > 300)
        {
            if (dist < 30)
            {
                step_flag = 3;
                last_time = now;
            }
            else
            {
                Goward(-20,20);
                step_flag = 10;
            }
        }
        break;
    case 3:
        Servo_SetAngle(90); // 舵机向前 使超声波朝前方
        Backward(15);
        if (now - last_time > 1000)
        {
            step_flag = 4;
            last_time = now;
        }
        break;
    case 4:
        Goward(20,-20);
        step_flag = 10;
        break;
    case 10:
        if (now - last_time > 300)
        {
            step_flag = 0;
            last_time = now;
        }
        break;
    }
}

void Do_Track_Task(void)
{   
    static uint32_t track_timer=0;
    /* 位置状态寄存器 */
    //(左 HW4 -> 右 HW1) 
    uint8_t track_status = (HW_4 << 3) | (HW_3 << 2) | (HW_2 << 1) | HW_1;
    if (now - track_timer >50){
        /*位置判断 */
        //二进制不给编译通过
        switch (track_status)
        {
        /*直行*/
        case 0x00: // 0000
        case 0x06: // 0110
            Forward(20);break;

        /*左转*/
        case 0x08: // 1000
            Goward(10,40);break;
        case 0x04: // 0100
            Goward(10,30);break;
        //case 0x0C: // 1100
        //case 0x0E: // 1110
            

        /*右转*/
        case 0x01: // 0001
            Goward(40,10);break;
        case 0x02: // 0010
            Goward(30,10);break;
        //case 0x03: // 0011
        //case 0x07: // 0111

        /*特殊状态*/
        case 0x0F: // 1111（黑色十字路）
            //Stop();
            break;

        default:
            //Forward(); // 默认执行上一次状态
            break;
        }
        track_timer=now;
    }
}

void PID_Track_Task(void)
{
    // 定时器会执行，这里占个位。
}
