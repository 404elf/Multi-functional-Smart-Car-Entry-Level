#ifndef __SYS_H
#define __SYS_H

#include "stm32f10x.h" 
/* 导出变量 */
extern volatile uint16_t Mode;
extern volatile uint32_t g_ms_ticks;
extern volatile uint32_t now;
extern volatile uint32_t g_serial_rx_time;
/* 导出函数 */
uint32_t millis(void);

/* 导出枚举类型 */
typedef enum {
    MODE_STOP = 0,
    MODE_SOUND_TRACK,
    MODE_REMOTE,
    MODE_SOUND_AVOID,
    MODE_TRACK,
    MODE_PID_TRACK,
} MODEID_t;

typedef enum {
    MOTOR_STOP = 0,
    MOTOR_LEFT,
    MOTOR_RIGHT,
    MOTOR_BACKWARD,
    MOTOR_FORWARD,
} MotorID_t;

typedef enum {
    LAST_FLAG = 0,
    ON_LINE,
} StepFlagStatus_t;

typedef enum {
    S_FRONT = 0,   // 看前方
    S_LOOK_R,      // 扭头看右边
    S_TURN_R,      // 执行右转
    S_LOOK_L,      // 扭头看左边
    S_TURN_L,      // 执行左转
    S_BACK_UP,     // 后退
    S_EMERGENCY    // 紧急转身
} AvoidStep_t;

/* 导出结构体 */
// 定义 PID 的所有家当
typedef struct {
    float Kp;
    float Ki;
    float Kd;
    float Error;
    float Last_Error;
    float Integral;
    float Output;
} PID_Typedef;

/* 导出宏定义 */
#define BITBAND(addr, bit) ((addr & 0xF0000000) + 0x02000000 + ((addr & 0xFFFFF) << 5) + (bit << 2))
#define MEM_ADDR(addr)     *((volatile unsigned long *)(addr))


#define PA_ODR (GPIOA_BASE + 0x0C)
#define PA_IDR (GPIOA_BASE + 0x08)
#define PB_ODR (GPIOB_BASE + 0x0C)
#define PB_IDR (GPIOB_BASE + 0x08)
#define PC_ODR (GPIOC_BASE + 0x0C)
#define PC_IDR (GPIOC_BASE + 0x08)


#define PAout(n) MEM_ADDR(BITBAND(PA_ODR, n))
#define PAin(n)  MEM_ADDR(BITBAND(PA_IDR, n))

#define PBout(n) MEM_ADDR(BITBAND(PB_ODR, n))
#define PBin(n)  MEM_ADDR(BITBAND(PB_IDR, n))

#define PCout(n) MEM_ADDR(BITBAND(PC_ODR, n))
#define PCin(n)  MEM_ADDR(BITBAND(PC_IDR, n))


/*
    PAout(5) = 1;      
    PBout(1) = 0;      
    if(PCin(13) == 0)  
*/


#endif
