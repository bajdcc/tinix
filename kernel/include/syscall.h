#ifndef _SYSCALL_H
#define _SYSCALL_H
#include <include/types.h>

extern s32 sys_open(u8 *);
extern s32 sys_close(s32);
extern s32 sys_write(s32 , u8* , u32 );
extern s32 sys_read(s32 , u8* , u32 );
extern s32 sys_seek(s32 , s32 , u32 );
extern s32 sys_wait(u32);
extern s32 sys_exit();
extern s32 sys_exec(u8 *);
extern s32 sys_kill(u32);
extern s32 sys_getpid();
extern s32 sys_getticks();
extern s32 sys_pstat(u32, void *, u32);
extern s32 sys_opendir(u8 *);
extern s32 sys_readdir(s32, void *, u32);
extern s32 sys_getdate(void *, u32);
extern s32 sys_reboot(u32);
extern s32 sys_sleep(u32);

extern s32 system_call_entry(u32, u32, u32, u32);

extern u32 syscall_counter;
extern u32 syscall_error;
#endif
