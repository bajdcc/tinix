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
typedef struct _task_stat
{
    /* 任务状态 */
    u32     state;                              /* 进程状态 */
    u32     father;                             /* 父进程id */
    u32     utime;                              /* 用户态运行时间(滴答数) */
    u32     ktime;                              /* 内核态运行时间(滴答数) */
    u32     stime;                              /* 进程开始的时间 */
    u32     usedmm;                             /* 进程使用的内存 */
    u8      conname[16];                        /* 控制台名称 */
    u8      comm[16];                           /* 执行体名称 */
} __attribute__((packed)) task_stat;

s32 sys_pstat(u32 pid, void *st_pstat, u32 size)
{
    if (pid >= MAX_TASK) return -1;
    if (task_list[pid].state == TASK_NULL) return -1;
    if (size != sizeof(task_stat)) return -1;

    task_stat *pstat = (task_stat*)st_pstat;
    
    memset(st_pstat, 0, sizeof(task_stat));
    pstat->state   = task_list[pid].state;
    pstat->father  = task_list[pid].father;
    pstat->utime   = task_list[pid].user_time;
    pstat->ktime   = task_list[pid].system_time;
    pstat->stime   = task_list[pid].start_time;
    pstat->usedmm  = calc_taskmm(pid);
    strcpy(pstat->comm, kfile_table[task_list[pid].fid[0]].f_name);
    strcpy(pstat->conname, kfile_table[task_list[pid].fid[1]].f_name);

    return pid;
}

