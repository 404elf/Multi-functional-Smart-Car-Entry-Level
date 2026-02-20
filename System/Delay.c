#include "stm32f10x.h"                  // Device header  
 
// 定义与数据观察与跟踪（DWT）模块相关的寄存器地址和位掩码  
#define DEM_CR             *((volatile uint32_t *)0xE000EDFC) 
#define DWT_CR             *((volatile uint32_t *)0xE0001000) 
#define DWT_CYCCNT         *((volatile uint32_t *)0xE0001004) 
#define DEM_CR_TRCENA      0x01000000   
#define DWT_CR_CYCCNTENA   0x00000001 
 
 
// 初始化DWT（数据观察与跟踪）模块的函数  
void DWT_Init(void)  
{  
    DEM_CR |= DEM_CR_TRCENA;  		// 启用数据观察与跟踪模块的控制寄存器中的跟踪使能位  
    DWT_CYCCNT = 0;           		// 将周期计数器清零  
    DWT_CR |= DWT_CR_CYCCNTENA; 	// 启用周期计数器  
}  
 

void Delay_us(uint32_t time)
{
    // 1. 计算总共需要跳动多少次（72MHz下延时1us需要72次计数）
    uint32_t cycles = time * 72;
    
    // 2. 记录出发时的“表盘读数”
    uint32_t start_count = DWT_CYCCNT; // 修正语法：去掉 -> 改为下划线
    
	// 关闭全局中断（保存当前PRIMASK寄存器值）
    //uint32_t primask = __get_PRIMASK();
    //__disable_irq();
	
    // 3. 只要 (现在读数 - 出发读数) 还没达到 目标步数，就一直等
    while ((DWT_CYCCNT - start_count) < cycles); 
	// 恢复全局中断状态
	//__set_PRIMASK(primask);
}
 
// 延迟指定毫秒数的函数  
void Delay_ms(uint32_t time)  
{  
    while(time--)
    {
        Delay_us(1000);
    }
}  
 
// 延迟指定秒数的函数  
void Delay_s(uint32_t time)  
{  
    while(time--)
    {
        Delay_ms(1000);
    }
}
