#include "stm32f10x.h"                  // Device header
volatile uint16_t Mode = 0; //模式变量
volatile uint32_t g_ms_ticks = 0; // 全局毫秒计数器
volatile uint32_t g_serial_rx_time = 0; //串口接收时间变量
volatile uint32_t now;
//返回时间
//!长达五十天溢出
uint32_t millis(void)
{
    return g_ms_ticks;
}

typedef enum {  
    BACKFORWARD = 0,
    FORWARD,
    RIGHT,    
} STATUS_Typedef;
