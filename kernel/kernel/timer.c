#include <include/types.h>
#include <include/timer.h>
#include <include/limits.h>
#include <include/task.h>
#include <include/system.h>
#include <include/stdarg.h>
#include <include/io.h>

/*
 *  本文件定义了时钟中断处理器和内核定时器
 *
 *  目前内核定时器还没啥大用，只在sys_reboot里使用到
 */
timer_struct timer_list[MAX_TIMER];      /* 定时器队列 */
u32  jiffies=0;                          /* 开机后的滴答数 */

void timer_handler(u32 cpl, regs *p_int_regs)
{
    u32 i;

    ++jiffies;
    
    if (cpl == 3) {
        task_list[current].user_time++;
    } else {
        task_list[current].system_time++;
    }    


    /*
     *  触发所有内核定时器
     *  触发时中断处于关闭状态
     *  因此定时器例程不能wait或sleep
     */
    for (i = 0; i < MAX_TIMER; i++)
    {
        if (timer_list[i].state != TIMER_NULL)
        {
            if ((--timer_list[i].counter) == 0) 
            {
                timer_list[i].timer_dirver(timer_list[i].handle);
                if (timer_list[i].state == TIMER_LOOP)
                {
                    timer_list[i].counter = timer_list[i].start_counter;
                } else {
                    timer_list[i].state = TIMER_NULL;
                }
            }  
        }
    }
    
    /*
     * 如果时间片被用完
     * 或已处于非运行态则需要进行调度 
     */
    if (task_list[current].state != TASK_RUNNING)
    {
        scheduler(p_int_regs);
        return;
    } 
    else if ((--task_list[current].counter) == 0) 
    {
        scheduler(p_int_regs);
        return;
    }
    else
    {
        return;
    }
}


/*
 *  在定时器队列中添加一个定时器
 *  触发时中断处于关闭状态
 *  因此定时器例程不能wait或sleep
 */
s32 add_timer(dotimefun_ptr fun, u8 state, u32 counter, s32 handle)
{
    u32 i,eflags;
    for (i = 0; i < MAX_TIMER; i++)
    {
        _local_irq_save(eflags);
        if (timer_list[i].state == TIMER_NULL)
        {
            timer_list[i].timer_dirver = fun;
            timer_list[i].start_counter = counter;
            timer_list[i].counter = counter;
            timer_list[i].handle = handle;
            timer_list[i].state = state;
            _local_irq_restore(eflags);
            return i;
        }
        _local_irq_restore(eflags);
    }
    return -1;
}

void init_timer()
{
    u32 i;
    jiffies = 0;
    for (i = 0; i < MAX_TIMER; i++)
        timer_list[i].state = TIMER_NULL;

    outb_p(0x36, 0x43);                     /* binary, mode 3, LSB/MSB, ch 0 */
    outb_p(LATCH & 0xff, 0x40);             /* LSB */
    outb(LATCH >> 8, 0x40);                 /* MSB 初始化时钟中断频率 */
    dmesg("Init timer completed!");
}

