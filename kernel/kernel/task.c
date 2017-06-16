#include <include/types.h>
#include <include/task.h>
#include <include/limits.h>
#include <include/string.h>
#include <include/system.h>
#include <include/stdarg.h>
#include <include/io.h>
#include <include/timer.h>
#include <dev/console.h>
#include <mm/memory.h>

/*
 * 这是个很搓的宏，模拟一个压栈
 */
#define _PUSH(esp, value) do{   \
     (esp) -=  sizeof(u32);     \
    *(u32*)(esp) = (u32)value;  \
    } while(0)


task_struct  task_list[MAX_TASK];        /* 任务队列 */
u32  current=0;                          /* 当前任务 */

/*
 * 全局的tss，任务切换时只用他的esp0
 * 其他的实际没啥用，io许可位图大于等于tss结构时，
 * 表示没有io许可位图
 */
tss_struct init_tss = {
    .link   = 0,
    .esp0   = 0,
    .ss0    = 0x10,
    .esp1   = 0,
    .ss1    = 0,
    .esp2   = 0,
    .ss2    = 0,
    .cr3    = K_PAGE_DIR-PAGE_OFFSET,
    .eip    = 0,
    .eflags = 0x200L,
    .eax    = 0,
    .ecx    = 0,
    .edx    = 0,
    .ebx    = 0,
    .esp    = 0,
    .ebp    = 0,
    .esi    = 0,
    .edi    = 0,
    .es     = 0x23,
    .cs     = 0x1B,
    .ss     = 0x23,
    .ds     = 0x23,
    .fs     = 0x23,
    .gs     = 0x23,
    .ldtr   = 0,
    .trace  = 0,
    .io_map_addr = sizeof(tss_struct)
};

/*
 * 一些功能函数
 */
static void setup_gdt_entry(struct desc_struct *, u32, u32, u8, u8);    /* 设置gdt入口，用于tss */
static void switch_context(u32,regs *);                                 /* 切换任务上下文 */
static void switch_mm(u32);                                             /* 切换pgd及esp0 */
static void make_task0();                                               /* 捏造0号任务 */
extern void system_call(void);
extern void fast_sys_call(void);


/*
 * 设置全局描述符
 */
static void setup_gdt_entry(
    struct desc_struct *item, 
    u32 base, 
    u32 limit, 
    u8 access, 
    u8 attribs)
{
    item->a = (base << 16) | (limit & 0xFFFF);
    item->b = base & 0xFF000000;
    item->b |= (attribs | (limit & 0x000F0000 >> 16) )<<16;
    item->b |= access<<8;
    item->b |= (base & 0x00FF0000)>>16;
}

/*
 *  在这里进行任务上下文的切换
 *
 *  通用寄存器的切换直接修改中断栈上的regs结构即可
 *  fpu寄存器是否save和restore则需根据当前任务的flags标志
 */
static void switch_context(u32 next,regs *p_int_regs)
{
    memcpy(&task_list[current].t_regs, p_int_regs, sizeof(regs));
    memcpy(p_int_regs, &task_list[next].t_regs, sizeof(regs));

    /*
     *  如果当前任务正在使用fpu，则fpu需保存
     *  同理，如果下一个任务正在使用fpu，则fpu需恢复
     *  当两个任务对fpu的使用情况不同时，cr0的ts位需要修改
     */
    if (task_list[current].flags & TASK_FLAG_FPU) 
    {
        save_fpu(&task_list[current].t_fpu_regs);
        if (task_list[next].flags & TASK_FLAG_FPU)
        {
            restore_fpu(&task_list[next].t_fpu_regs);
        }
        else
        {
            set_cr0_ts();
        }
    }
    else 
    {
        if (task_list[next].flags & TASK_FLAG_FPU)
        {
            clear_cr0_ts();
            restore_fpu(&task_list[next].t_fpu_regs);
        }
    }
    return;
}

/*
 * 切换cr3后用户态内存已经隔离开
 * 将init_tss.esp0修改为当前任务的内核栈
 * 当产生中断、发出系统调用、自陷等一切特权级切换时会使用该esp
 * SYSENTER_ESP_MSR也要修改,因为有可能使用sysenter进行系统调用
 */
static void switch_mm(u32 next)
{
    set_cr3(task_list[next].pgd);
    init_tss.esp0 = task_list[next].k_stack;

    #ifdef FAST_SYS_CALL
    wrmsr(SYSENTER_ESP_MSR, task_list[next].k_stack);
    #endif

    return;
}

/*
 * 任务调度器，目前主要是轮询
 * 发现可调度的即进行调度，没有优先级
 */
void scheduler(regs *p_int_regs)
{
    u32 next;

    /*
     * 先给时间片
     */
    task_list[current].counter = MAX_TASK_COUNTER;                 

    /* 
     * 遍历任务队列,找个可以调度的任务
     */
    if (current == MAX_TASK-1) next=0;
    else next = current+1;
    while (1)                                       
    {
        /*
         * 找一个时间片不为0且正在运行的
         */
        if (task_list[next].counter > 0 && \
            task_list[next].state == TASK_RUNNING)
            break;

        /*
         * 如果遍历了一遍都找不到，则继续当前任务
         */
        if (next == current) break;
        if (next == MAX_TASK-1) next=0;
        else next++;
    }

    if (next != current)
    {
        switch_context(next, p_int_regs);
        switch_mm(next);
        current = next;
    }
    return;
}


/*
 * 任务进入等待循环中，直到被动唤醒才会返回
 * 进入此函数时，任务必须事先状态已经是wait
 * 否则一定是内核逻辑出现问题，下文ioblock同理
 */
void task_wait()
{
    while(task_list[current].state == TASK_WAIT)
    {
        idle();
        /* 内存屏障 */
        __asm__ __volatile__("":::"memory");    
    }
}

/*
 * 唤醒一个正在wait的任务
 * 进入此函数时，任务必须事先状态已经是wait
 * 否则一定是内核逻辑出现问题，下文ioblock同理
 */
void task_wakeup(u32 taskid)
{
    if( taskid >= MAX_TASK ||\
        task_list[taskid].state != TASK_WAIT)
    {
        panic("try to wakeup a non-wait task! kernel die! cur %d wake %d",
            current, taskid);
    }
    task_list[taskid].state = TASK_RUNNING;
}

void task_ioblock()
{
    while(task_list[current].state == TASK_IOBLOCK)
    {
        idle();
        /* 内存屏障 */
        __asm__ __volatile__("":::"memory");
    }
}

void task_iowakeup(u32 taskid)
{
    if( taskid >= MAX_TASK ||\
        task_list[taskid].state != TASK_IOBLOCK)
    {
        panic("try to wakeup a non-block task! kernel die! cur %d wake %d",
            current, taskid);
    }
    task_list[taskid].state = TASK_RUNNING;
}

/*
 * 捏造一个0号任务
 */
static void make_task0()
{
    u32 i;
    current = 0;                                
    task_list[0].state      = TASK_RUNNING;
    task_list[0].flags      = 0;            
    task_list[0].counter    = 5;
    task_list[0].father     = 0;
    task_list[0].pwait      = 0;
    task_list[0].user_time  = 0;
    task_list[0].system_time= 0;
    task_list[0].start_time = 0;
    task_list[0].pgd        = K_PAGE_DIR-PAGE_OFFSET;
    for (i=0;i<MAX_OPEN;++i) task_list[0].fid[i] = -1;
    task_list[0].k_stack    = 0xC0090000;
    set_cr0_ts();
}

/*
 * 添加一个任务，或者说调度单位
 *
 * fid为这个任务的执行体文件id，此id用于入口函数解析elf文件
 * task_entry即新加任务的入口代码地址
 * 新任务的0号句柄指向的fid即参数指定的id
 * 新任务的1号句柄将直接继承父任务的1号句柄，其余句柄置空
 */
s32 task_add_task(u32 fid, void * task_entry)
{
    u32 tid,i,eflags;

    tid = 0;
    _local_irq_save(eflags);
    for (i = 1; i<MAX_TASK; i++)
    {
        if (task_list[i].state == TASK_NULL)
        {
            tid = i;
            break;
        }
    }

    if (tid == 0) 
    {
        _local_irq_restore(eflags);
        return -1;                            /* 任务队列已满 */
    }

    task_list[tid].counter      = MAX_TASK_COUNTER;     /* 任务初始时间片 */
    task_list[tid].flags        = 0;                    /* 任务标志位图 */
    task_list[tid].father       = current;              /* 任务父进程 */
    task_list[tid].pwait        = 0;                    /* 没有等待该任务的任务 */
    task_list[tid].user_time    = 0;                    /* 用户态时间 */
    task_list[tid].system_time  = 0;                    /* 内核态时间 */
    task_list[tid].start_time   = jiffies;              /* 起始时间为当前滴答数 */
    task_list[tid].pgd          = u_get_page_table()-PAGE_OFFSET;

    memset(&(task_list[tid].t_regs),0,sizeof(regs));
    task_list[tid].t_regs.ds = KERNEL_DS;               /* 任务的初始数据段是 KERNEL_DS */
    task_list[tid].t_regs.gs = KERNEL_DS;
    task_list[tid].t_regs.es = KERNEL_DS;
    task_list[tid].t_regs.fs = KERNEL_DS;
    task_list[tid].t_regs.org_esp = k_get_page() + PAGE_SIZE;   /* 取一个内核态页作为原始 esp */
    task_list[tid].k_stack = task_list[tid].t_regs.org_esp;     /* 他也是这个任务的esp0 */

    _PUSH(task_list[tid].t_regs.org_esp, 0x200);        /* 在栈上构造一个无特权级切换的陷阱帧 eflags */
    _PUSH(task_list[tid].t_regs.org_esp, KERNEL_CS);    /* 初始cs为KERNEL_CS */
    _PUSH(task_list[tid].t_regs.org_esp, task_entry);   /* 初始eip为参数指定的入口 */

    for (i = 0; i < MAX_OPEN; ++i) 
        task_list[tid].fid[i] = -1;                     /* 句柄表为空 */
    task_list[tid].fid[0] = fid;                        /* 0号句柄是他的执行体 */
    task_list[tid].fid[1] = task_list[current].fid[1];  /* 1号句柄继承父进程的控制台句柄 */

    task_list[tid].state = TASK_RUNNING;                /* 下一次任务调度将会运行该任务 */

    _local_irq_restore(eflags);

    return tid;
}

/*
 * 杀死已结束的任务
 *
 * 这个函数仅在main中调用
 * 返回非0表示已发生了任务清空工作
 */
u32 task_kill_stoped()
{
    u32 stoped_id,child_id,eflags;
    u32 ret = 0;

    for (stoped_id = 1; stoped_id < MAX_TASK; stoped_id++)
    {
        if (task_list[stoped_id].state == TASK_STOPED)
        {
            /*
             * 过继子任务，这里需要关中断，
             * 因为有可能在修改进程关系的时候被调度，父进程退出
             */
            ret++;
            _local_irq_save(eflags);
            for (child_id = 0; child_id<MAX_TASK; child_id++)
            {
                if (task_list[child_id].state != TASK_NULL &&\
                    task_list[child_id].father == stoped_id)
                {
                    task_list[child_id].father = task_list[stoped_id].father;
                }
            }
            _local_irq_restore(eflags);

            /* 释放用户态内存 */
            u_free_page_table(task_list[stoped_id].pgd + PAGE_OFFSET);
            /* 释放释放内核栈和pgd */
            k_free_page(task_list[stoped_id].pgd + PAGE_OFFSET);
            k_free_page(task_list[stoped_id].k_stack - PAGE_SIZE);
            task_list[stoped_id].state = TASK_NULL;
        }
    }
    return ret;
}


/* 
 * 初始化任务队列，加载全局tss
 *
 * 如果启用了快速系统调用，则设置响应msr寄存器
 */
void init_task()
{
    u32 i;
    for (i = 0; i < MAX_TASK; i++)
        task_list[i].state = TASK_NULL;        /* 将所有任务置为空 */

    make_task0();

    setup_gdt_entry(&gdt[5], (u32) & init_tss, sizeof(tss_struct), ACS_TSS, 0);
    ltr(0x28);

    set_system_gate(0x80, &system_call);

    /* 
     * 提供一个快速系统入口，具体使用哪个由用户态决定
     * SYSENTER_ESP_MSR在任务切换时switch_mm中重写
     */

    #ifdef FAST_SYS_CALL
    wrmsr(SYSENTER_CS_MSR, KERNEL_CS);
    wrmsr(SYSENTER_EIP_MSR, &fast_sys_call);
    dmesg("Fast system call [sysenter] is supported!");
    #endif

    dmesg("Init scheduler completed!");
}
