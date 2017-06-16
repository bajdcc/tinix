#include <include/types.h>
#include <include/stdarg.h>
#include <include/task.h>
#include <include/timer.h>
#include <include/traps.h>
#include <include/system.h>
#include <include/syscall.h>
#include <include/string.h>
#include <include/io.h>
#include <mm/memory.h>
#include <dev/console.h>
#include <dev/floppy.h>
#include <dev/keyboard.h>
#include <fs/fs.h>
#include <fs/fat12.h>
#include <fs/sysfs.h>

/*
 * 检查各个控制台是否有任务
 * 没有则为这个控制台启动一个任务
 */
void alive_console()
{
    u32 tid;
    u32 is_alive_con0 = 0;
    u32 is_alive_con1 = 0;
    u32 is_alive_con2 = 0;

    /*
     * 如果发现某个控制台已经没有进程在使用 
     * 为他启动一个sh
     */
    for (tid = 0; tid<MAX_TASK; tid++)
    {
        if (task_list[tid].state != TASK_NULL)
        {
            if (task_list[tid].fid[1] == fcon0)
                is_alive_con0 = 1;
            else if (task_list[tid].fid[1] == fcon1)
                is_alive_con1 = 1;
            else if (task_list[tid].fid[1] == fcon2)
                is_alive_con2 = 1;
        }
    }

    if (!is_alive_con0)
    {
        dmesg("Restart a shell on con0.");
        task_list[0].fid[1] = fcon0;
        sys_exec("/bin/sh");
    }
    if (!is_alive_con1)
    {
        dmesg("Restart a shell on con1.");
        task_list[0].fid[1] = fcon1;
        sys_exec("/bin/sh");
    }
    if (!is_alive_con2)
    {
        dmesg("Restart a shell on con2.");
        task_list[0].fid[1] = fcon2;
        sys_exec("/bin/sh");
    }
    task_list[0].fid[1] = fcon3;

    task_list[0].counter = 1;     
    idle();
}


void k_main()
{
    init_mem();
    init_fs();
    init_con();
    init_kbd();
    init_timer();
    init_task();

    #ifndef NO_FLOPPY
    init_fd();
    #endif
    
    init_traps();
    sti();
    init_fat12fs();
    init_sysfs();

    task_list[0].fid[0] = -1;
    sys_open("/boot/oskernel");

    /*
     * 为三个空闲控制台分别启动一个shell
     * 因为控制台句柄，1号句柄自动从父任务继承
     * 启动前将任务0的1号句柄设置为对应的控制台
     */
    task_list[0].fid[1] = fcon0;
    sys_exec("/bin/sh");
    
    task_list[0].fid[1] = fcon1;
    sys_exec("/bin/sh");
    
    task_list[0].fid[1] = fcon2;
    sys_exec("/bin/sh");
    
    task_list[0].fid[1] = fcon3;
    switch_con(fcon0);

    while(1)
    {
        if (task_kill_stoped())
            alive_console();
        sys_sleep(1);
    }
}


