/*
 * 任务管理
 */

#ifndef _TASK_H
#define _TASK_H

#include <include/types.h>
#include <include/limits.h>
#include <fs/fs.h>

/* 任务状态 */
#define TASK_NULL               0   /* 此位置没有任务 */
#define TASK_RUNNING            1   /* 运行 */
#define TASK_SLEEP              2   /* 睡眠 */
#define TASK_IOBLOCK            3   /* IO阻塞 */
#define TASK_WAIT               4   /* 等待 */
#define TASK_STOPED             5   /* 已终止 */

/*
 * 任务标志位定义
 */
#define TASK_FLAG_FPU           1   /* 任务需要使用fpu */

/* TSS 结构描述 */
typedef struct
{
    /* TSS for 386+ */
    u32 link;
    u32 esp0;
    u32 ss0;
    u32 esp1;
    u32 ss1;
    u32 esp2;
    u32 ss2;
    u32 cr3;
    u32 eip;
    u32 eflags;
    u32 eax;
    u32 ecx;
    u32 edx;
    u32 ebx;
    u32 esp;
    u32 ebp;
    u32 esi;
    u32 edi;
    u32 es;
    u32 cs;
    u32 ss;
    u32 ds;
    u32 fs;
    u32 gs;
    u32 ldtr;
    u16 trace;
    u16 io_map_addr;
} __attribute__((packed)) tss_struct;

/* 通用寄存器结构 */
typedef struct _regs
{
    u32 eax;
    u32 ebx;
    u32 ecx;
    u32 edx;
    u32 esi;
    u32 edi;
    u32 ebp;
    u32 fs;
    u32 gs;
    u32 es;
    u32 ds;
    u32 err_code;
    u32 org_esp;
    // u32 eip;
    // u32 cs;
    // u32 eflags;
    // u32 esp;                                /* 仅在特权切换时才出现 */
    // u32 ss;                                 /* 仅在特权切换时才出现 */
}__attribute__((packed)) regs;

/*
 *  FPU 寄存器结构
 */
typedef struct _fpu_regs 
{
    u32     cwd;
    u32     swd;
    u32     twd;
    u32     fip;
    u32     fcs;
    u32     foo;
    u32     fos;
    u32     st_space[20];                       /* 8*10 bytes for each FP-reg = 80 bytes */
}__attribute__((packed)) fpu_regs;

/* 任务结构 */
typedef struct _task_struct
{
    /* 任务状态 */
    u32         state;                          /* 任务状态 */
    u32         flags;                          /* 任务标志位图，目前仅用于fpu */
    u32         father;                         /* 父任务id */
    u32         pwait;                          /* 要等待的任务id */
    u32         counter;                        /* 可使用时间片 */
    u32         user_time;                      /* 用户态运行时间(滴答数) */
    u32         system_time;                    /* 内核态运行时间(滴答数) */
    u32         start_time;                     /* 任务开始的时间 */

    /* 任务资源 */
    s32         fid[MAX_OPEN];                  /* 任务使用的文件表,0号是执行文件本身，1号文件是con */
    regs        t_regs;                         /* 通用寄存器 */
    fpu_regs    t_fpu_regs;                     /* FPU 寄存器 */
    u32         k_stack;                        /* esp0 */
    u32         pgd;                            /* 任务页目录 */
} __attribute__((packed)) task_struct;

extern tss_struct   init_tss;
extern task_struct  task_list[MAX_TASK];        /* 任务队列 */
extern u32  current;                            /* 当前任务 */


extern void init_task();                        /* 任务队列初始化 */
extern void scheduler(regs *);                  /* 调度器 */
extern void task_ioblock();                     /* 任务陷入io阻塞中 */
extern void task_iowakeup(u32);                 /* io完成唤醒 */
extern void task_wait();                        /* 任务陷入等待 */
extern void task_wakeup(u32);                   /* 等待完成唤醒 */
extern s32  task_add_task(u32, void *);         /* 新增任务,执行体文件id,入口指针 */
extern u32  task_kill_stoped();                 /* 杀死已结束的任务 */
#endif
