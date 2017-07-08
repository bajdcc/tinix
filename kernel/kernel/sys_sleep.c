#include <include/types.h>
#include <include/syscall.h>
#include <include/task.h>
#include <include/string.h>
#include <include/stdarg.h>
#include <include/timer.h>
#include <include/limits.h>
#include <include/system.h>
#include <mm/memory.h>
#include <fs/fs.h>
#include <dev/console.h>

/*
 *  这个地方可能有bug
 *  因为对应任务可能因为别的原因被kill
 */
void wakeup(u32 handle)
{
    if (task_list[handle].state == TASK_SLEEP)
    {
        task_list[handle].state = TASK_RUNNING;
        return;
    }    
    warning("Wake up task %d error.", handle);
}

s32 sys_sleep(u32 stime)
{
    u32 eflags;

    if(stime == 0) return 0;

    _local_irq_save(eflags);
    task_list[current].state = TASK_SLEEP;
    add_timer((dotimefun_ptr)&wakeup, TIMER_ONCE, stime, current);
    _local_irq_restore(eflags);

    while(task_list[current].state == TASK_SLEEP)
    {
        idle();
        __asm__ __volatile__("":::"memory");
    }

    return stime;
}