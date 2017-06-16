#include <include/types.h>
#include <include/syscall.h>
#include <include/task.h>
#include <include/string.h>
#include <include/stdarg.h>
#include <include/timer.h>
#include <include/limits.h>
#include <include/system.h>
#include <mm/memory.h>
#include <dev/console.h>
#include <fs/fs.h>


s32 sys_exit()
{
    u32 eflags;

    _local_irq_save(eflags);
    if (task_list[current].pwait != 0) 
        task_wakeup(task_list[current].pwait);

    task_list[current].state = TASK_STOPED;
    _local_irq_restore(eflags);

    /* 等待0号任务清空 */
    while(1){idle();};
}
