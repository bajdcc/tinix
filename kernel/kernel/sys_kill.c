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

#define _is_task_dead(_tid)  (task_list[_tid].state == TASK_NULL ||\
        task_list[_tid].state == TASK_STOPED)
#define _is_task_alive(_tid) (task_list[_tid].state != TASK_NULL &&\
        task_list[_tid].state != TASK_STOPED)

s32 sys_kill(u32 tid)
{
    u32 h,kfid,eflags;
    if (tid >= MAX_TASK || tid == 0) return -1;
    if (tid == current) sys_exit(); 

    _local_irq_save(eflags);
    if (_is_task_dead(tid)) 
    {
        _local_irq_restore(eflags);
        return -1;
    }

    /* 如果任务处于io阻塞中 释放掉其占用的设备文件 */
    if (task_list[tid].state == TASK_IOBLOCK)
    {
        for (h = 1; h < MAX_OPEN; h++)
        {
            if (task_list[tid].fid[h] < 0) continue;
            kfid = task_list[tid].fid[h];
            if (!(kfile_table[kfid].f_req)) continue;
            if (kfile_table[kfid].f_req->f_req_state == REQ_FREE) continue;
            if (kfile_table[kfid].f_req->f_req_task != tid) continue;

            kfile_table[kfid].f_req->f_req_state = REQ_FREE;
        }
    }

    /* 如果有人正在等待它， */
    if (task_list[tid].pwait != 0)
    {
        /* 且任务正在等待 已形成一条等待链 则需要改变等待关系 */
        if (task_list[tid].state == TASK_WAIT )      
        {
            /*  找到他正在等的那个任务,修改pwait */
            for (h = 0; h < MAX_TASK; h++)
            {
                if (task_list[h].pwait == tid && _is_task_alive(tid)) break;
            }
            task_list[h].pwait = task_list[tid].pwait;
            task_list[tid].pwait = 0;
        }
        else
        {
            task_wakeup(task_list[tid].pwait);
        } 
    }
    else
    {
        if (task_list[tid].state == TASK_WAIT )      
        {
            /*  找到他正在等的那个任务,修改pwait,告诉他我不等了 */
            for (h = 0; h < MAX_TASK; h++)
            {
                if (task_list[h].pwait == tid && _is_task_alive(tid)) break;
            }
            task_list[h].pwait = 0;
        }
    }

    /* 所有资源已经释放完毕 */
    task_list[tid].state = TASK_STOPED;
    _local_irq_restore(eflags);
    return tid;
}
