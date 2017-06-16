#include <stdarg.h>
#include <syscall.h>

#define LEN_LINE                160

char _char_color = 7;

static char buff[1024];
static char writebuff[2048];


int printf(const char *fmt, ...)
{
    va_list args;
    int i,j,con_pos;
    int  pos=0;

    va_start(args, fmt);
    i = vsprintf(buff ,fmt ,args);
    va_end(args);

    for (j = 0; j < i; j++)
    {
        if (buff[j] == '\t')
        {
            sys_write(1, writebuff, pos);
            pos = 0;
            con_pos  = sys_seek(1, 0, SEEK_CUR);
            sys_seek(1, 8-(con_pos % 8), SEEK_CUR);
        }
        else if (buff[j] == '\n')
        {
            sys_write(1, writebuff, pos);
            pos = 0;
            con_pos  = sys_seek(1, 0, SEEK_CUR);
            sys_seek(1, LEN_LINE - (con_pos % LEN_LINE), SEEK_CUR);
        }
        else
        {
            writebuff[pos++] = buff[j];
            writebuff[pos++] = _char_color;
        }
    }
    if (pos) sys_write(1, writebuff, pos);
    return i;
}
