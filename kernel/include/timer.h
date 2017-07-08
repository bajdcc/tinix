#ifndef _TIMER_H
#define _TIMER_H

#include <include/types.h>
#include <include/limits.h>
#include <include/task.h>

#define HZ 1000
#define LATCH (1193180 / HZ)

/* 定时器状态 */
#define TIMER_NULL              0   /* 无效定时器 */
#define TIMER_ONCE              1   /* 正在使用的定时器 */
#define TIMER_LOOP              2   /* 循环计数器 */

typedef void (*dotimefun_ptr)(s32 handle);      /* 定时器回调 原型 */

/*
 *  定时器的结构描述
 */
typedef struct _timer_struct
{
    u8  state;
    u32 start_counter;
    u32 counter;
    dotimefun_ptr timer_dirver;
    s32 handle;
}__attribute__((packed)) timer_struct;


/*
 * 日期结构，实际上数据根本不需要u32
 * 但此结构涉及和应用层通信，为避免对齐问题统一使用u32
 */
typedef struct  {
    u32 sec;
    u32 min;
    u32 hour;
    u32 dayofweek;
    u32 day;
    u32 month;
    u32 year;
}__attribute__((packed)) rtc_date;

extern timer_struct timer_list[MAX_TIMER];      /* 定时器队列 */
extern u32  jiffies;                            /* 开机后的滴答数 */

extern void init_timer();
extern void time_hander(u32, regs *);
extern s32  add_timer(dotimefun_ptr, u8, u32, s32);

#endif

