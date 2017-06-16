#include <stdarg.h>
#include <syscall.h>


int sprintf(char * buffer, const char *fmt, ...)
{
    int i;
    va_list args;
    va_start(args, fmt);
    i = vsprintf(buffer ,fmt ,args);
    va_end(args);

    return i;
}