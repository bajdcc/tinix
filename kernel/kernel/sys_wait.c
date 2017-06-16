#include <include/types.h>
#include <include/syscall.h>
#include <include/task.h>
#include <include/string.h>
#include <include/stdarg.h>
#include <include/timer.h>
#include <include/limits.h>
#include <include/system.h>
#include <fs/fs.h>


s32 sys_wait(u32 pid)
{
    u32 eflags;

    if (pid >= MAX_TASK || pid == current) return -1;

    _local_irq_save(eflags);
    if (task_list[pid].pwait == 0 && \
        task_list[pid].state != TASK_NULL && \
        task_list[pid].state != TASK_STOPED)
    {
        task_list[pid].pwait = current;
        task_list[current].state = TASK_WAIT;
        _local_irq_restore(eflags);
        task_wait();
        return pid;
    }
    _local_irq_restore(eflags);
    return -1;
}

