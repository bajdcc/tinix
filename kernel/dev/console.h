#ifndef _CONSOLE_H
#define _CONSOLE_H

#include <include/types.h>
#include <fs/fs.h>

extern void init_con();
extern s32 	con_write(file *, u8 *, u32 );
extern s32 	con_read(file *, u8 *, u32 );
extern s32 	con_seek(file *, s32, u32);
extern void switch_con(u32);
extern void con_putch(u8);

extern u32 fcon0,fcon1,fcon2,fcon3,current_con;        /* 4个控制台文件句柄 */

#endif /* _CONSOLE_H */

