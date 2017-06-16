#include <include/types.h>
#include <include/stdarg.h>
#include <include/task.h>
#include <dev/console.h>


static char buf[128];
static char writebuf[256];

/*
 * 向3号控制台打印内核消息
 * stdarg.h中的dmesg、panic均为对此函数的封装
 */
int printk(u8 attr, const char *fmt, ...)
{
    va_list args;
    int i,j;

    va_start(args, fmt);
    i=vsprintf(buf,fmt,args);
    va_end(args);
    for (j = 0; j < i; j++)
    {
        writebuf[j*2] = buf[j];
        writebuf[j*2+1] = attr;
    }
    con_write(&kfile_table[fcon3], writebuf, i*2);
    con_seek(&kfile_table[fcon3], 
            80*2 - (kfile_table[fcon3].f_pos[0] % (80*2)), 
            SEEK_CUR
            );
    return i;
}

