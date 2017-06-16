#include <stdarg.h>
#include <syscall.h>
#include <string.h>

typedef struct __iobuf {
     int             _count;
     int             _fd;
     int             _flags;
     int             _bufsiz;
     unsigned char   *_buf;
     unsigned char   *_ptr;
} FILE;

#define _IOFBF          0x000
#define _IOREAD         0x001
#define _IOWRITE        0x002
#define _IONBF          0x004
#define _IOMYBUF        0x008
#define _IOEOF          0x010
#define _IOERR          0x020
#define _IOLBF          0x040
#define _IOREADING      0x080
#define _IOWRITING      0x100
#define _IOAPPEND       0x200
#define _IOFIFO         0x400

static char buff[512] = {1};

int scanf(const char *format, ...)
{
    va_list ap;
    int retval;
    FILE tmp_stream;

    retval = sys_read(1, buff, 512);
    if (retval < 2) return -1;

    buff[retval-1] = 0;

    va_start(ap, format);

    tmp_stream._fd     = -1;
    tmp_stream._flags  = _IOREAD + _IONBF + _IOREADING;
    tmp_stream._buf    = (unsigned char *) buff;
    tmp_stream._ptr    = (unsigned char *) buff;
    tmp_stream._count  = strlen(buff);

    retval = _doscan(&tmp_stream, format, ap);

    va_end(ap);

    return retval;
}

