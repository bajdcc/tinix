#include <include/types.h>
#include <include/task.h>
#include <include/limits.h>
#include <include/string.h>
#include <include/system.h>
#include <include/stdarg.h>
#include <include/syscall.h>
#include <include/io.h>
#include <include/timer.h>
#include <dev/console.h>
#include <mm/memory.h>

#define     _NR_sys_open        0       //s32 sys_open(u8 *);
#define     _NR_sys_close       1       //s32 sys_close(s32);
#define     _NR_sys_write       2       //s32 sys_write(s32 , u8* , u32 );
#define     _NR_sys_read        3       //s32 sys_read(s32 , u8* , u32 );
#define     _NR_sys_seek        4       //s32 sys_seek(s32 , s32 , u32 );
#define     _NR_sys_wait        5       //s32 sys_wait(u32);
#define     _NR_sys_exit        6       //s32 sys_exit();
#define     _NR_sys_exec        7       //s32 sys_exec(u8 *);
#define     _NR_sys_kill        8       //s32 sys_kill(u32);
#define     _NR_sys_getpid      9       //s32 sys_getpid(u32);
#define     _NR_sys_getticks    10      //s32 sys_getticks();
#define     _NR_sys_pstat       11      //s32 sys_pstat(u32 pid, void *st_pstat, u32 size);
#define     _NR_sys_opendir     12      //s32 sys_opendir(u8 * path);
#define     _NR_sys_readdir     13      //s32 sys_readdir(s32 handle, void *st_fstat, u32 size);
#define     _NR_sys_getdate     14      //s32 sys_getdate(void *, u32);
#define     _NR_sys_reboot      15      //s32 sys_reboot(u32);
#define     _NR_sys_sleep       16      //s32 sys_sleep(u32);

u32 syscall_counter = 0;
u32 syscall_error   = 0;
/*
 * 所有系统调用的总入口
 * 第一个参数为调用号，其余三个或更少为参数
 */
s32 system_call_entry(u32 eax, u32 ebx, u32 ecx, u32 edx)
{
    s32 ret;
    s8 *service;
    switch (eax)
    {
        case _NR_sys_open:      ret = sys_open(ebx);                service = "sys_open";       break;
        case _NR_sys_close:     ret = sys_close(ebx);               service = "sys_close";      break;
        case _NR_sys_write:     ret = sys_write(ebx, ecx, edx);     service = "sys_write";      break;
        case _NR_sys_read:      ret = sys_read(ebx, ecx, edx);      service = "sys_read";       break;
        case _NR_sys_seek:      ret = sys_seek(ebx, ecx, edx);      service = "sys_seek";       break;
        case _NR_sys_wait:      ret = sys_wait(ebx);                service = "sys_wait";       break;
        case _NR_sys_exit:      ret = sys_exit();                   service = "sys_exit";       break;
        case _NR_sys_exec:      ret = sys_exec(ebx);                service = "sys_exec";       break;
        case _NR_sys_kill:      ret = sys_kill(ebx);                service = "sys_kill";       break;
        case _NR_sys_getpid:    ret = sys_getpid();                 service = "sys_getpid";     break;
        case _NR_sys_getticks:  ret = sys_getticks();               service = "sys_getticks";   break;
        case _NR_sys_pstat:     ret = sys_pstat(ebx, ecx, edx);     service = "sys_pstat";      break;
        case _NR_sys_opendir:   ret = sys_opendir(ebx);             service = "sys_opendir";    break;
        case _NR_sys_readdir:   ret = sys_readdir(ebx, ecx, edx);   service = "sys_readdir";    break;
        case _NR_sys_getdate:   ret = sys_getdate(ebx, ecx);        service = "sys_getdate";    break;
        case _NR_sys_reboot:    ret = sys_reboot(ebx);              service = "sys_reboot";     break;
        case _NR_sys_sleep:     ret = sys_sleep(ebx);               service = "sys_sleep";      break;
        default:                ret = -1;                           service = "bad nr";         break;
    }
    
    syscall_counter++;
    syscall_error += ret<0?1:0;
    
    return ret;
}

