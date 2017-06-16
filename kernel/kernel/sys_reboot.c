#include <include/types.h>
#include <include/syscall.h>
#include <include/task.h>
#include <include/string.h>
#include <include/stdarg.h>
#include <include/timer.h>
#include <include/limits.h>
#include <include/system.h>
#include <include/io.h>
#include <mm/memory.h>
#include <dev/console.h>
#include <fs/fs.h>

#define     APM_ADDR            ((u32 *)0xC0090008)
u32 delaytime;

void reboot(u32 handle)
{
    if (delaytime > 0) 
    {
        warning("System will reboot after %d seconds...", delaytime);
        delaytime--;
        return;
    }
    outb(0xFE,0x64);
}

s32 sys_reboot(u32 mode)
{
    if (mode == 0)
    {
        delaytime = 3;
        switch_con(fcon3);
        warning("System will reboot.");
        add_timer((dotimefun_ptr)&reboot, TIMER_LOOP, HZ, 0);
        return 0;
    }
    else
    {
        outb(0x3c,0x805);
    }
    return 0;
}
