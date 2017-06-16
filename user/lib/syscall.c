#include <syscall.h>

// 此文件中定义了所有系统调用的内联汇编实现
// 分为sysenter版本 和 int 80版本
// sysenter使用eax.ebx.esi.edi传递参数
// int 80使用eax.ebx.ecx.edx
// 可在makefile中根据cpu支持情况去除-D选项

#ifdef FAST_SYS_CALL

int sys_open(char *_filename)
{
    int _ret;
    __asm__ __volatile__(
            "movl %%esp, %%ecx\n\t"
            "movl $1f, %%edx\n\t"
            "sysenter\n\t"
            "1:"
            :"=a"(_ret)
            :"0" (_NR_sys_open),"b"(_filename)
            :"ecx","edx");
    return _ret;
}

int sys_close(unsigned int _handle)
{
    int _ret;
    __asm__ __volatile__(
            "movl %%esp, %%ecx\n\t"
            "movl $1f, %%edx\n\t"
            "sysenter\n\t"
            "1:"
            :"=a"(_ret)
            :"0" (_NR_sys_close),"b"(_handle)
            :"ecx","edx");
    return _ret;
}

int sys_write(int _handle,char * _buffer,int _size)
{
    int _ret;
    __asm__ __volatile__(
            "movl %%esp, %%ecx\n\t"
            "movl $1f, %%edx\n\t"
            "sysenter\n\t"
            "1:"
            :"=a"(_ret)
            :"0" (_NR_sys_write),"b"(_handle),"S"(_buffer),"D"(_size)
            :"ecx","edx");
    return _ret;
}

int sys_read(int _handle,char * _buffer,int _size)
{
    int _ret;
    __asm__ __volatile__(
            "movl %%esp, %%ecx\n\t"
            "movl $1f, %%edx\n\t"
            "sysenter\n\t"
            "1:"
            :"=a"(_ret)
            :"0" (_NR_sys_read),"b"(_handle),"S"(_buffer),"D"(_size)
            :"ecx","edx");
    return _ret;
}

int sys_seek(int _handle, int _offset,unsigned int _mode)
{
    int _ret;
    __asm__ __volatile__(
            "movl %%esp, %%ecx\n\t"
            "movl $1f, %%edx\n\t"
            "sysenter\n\t"
            "1:"
            :"=a"(_ret)
            :"0" (_NR_sys_seek),"b"(_handle),"S"(_offset),"D"(_mode)
            :"ecx","edx");
    return _ret;
}

int sys_wait(unsigned int _pid)
{
    int _ret;
    __asm__ __volatile__(
            "movl %%esp, %%ecx\n\t"
            "movl $1f, %%edx\n\t"
            "sysenter\n\t"
            "1:"
            :"=a"(_ret)
            :"0" (_NR_sys_wait),"b"(_pid)
            :"ecx","edx");
    return _ret;
}

int sys_exit()
{
    int _ret;
    __asm__ __volatile__(
            "movl %%esp, %%ecx\n\t"
            "movl $1f, %%edx\n\t"
            "sysenter\n\t"
            "1:"
            :"=a"(_ret)
            :"0" (_NR_sys_exit)
            :"ecx","edx");
    return _ret;
}


int sys_exec(char * _path)
{
    int _ret;
    __asm__ __volatile__(
            "movl %%esp, %%ecx\n\t"
            "movl $1f, %%edx\n\t"
            "sysenter\n\t"
            "1:"
            :"=a"(_ret)
            :"0" (_NR_sys_exec),"b"(_path)
            :"ecx","edx");
    return _ret;
}

int sys_kill(unsigned int _tid)
{
    int _ret;
    __asm__ __volatile__(
            "movl %%esp, %%ecx\n\t"
            "movl $1f, %%edx\n\t"
            "sysenter\n\t"
            "1:"
            :"=a"(_ret)
            :"0" (_NR_sys_kill),"b"(_tid)
            :"ecx","edx");
    return _ret;
}

int sys_getpid()
{
    int _ret;
    __asm__ __volatile__(
            "movl %%esp, %%ecx\n\t"
            "movl $1f, %%edx\n\t"
            "sysenter\n\t"
            "1:"
            :"=a"(_ret)
            :"0" (_NR_sys_getpid)
            :"ecx","edx");
    return _ret;
}

int sys_getticks()
{
    int _ret;
    __asm__ __volatile__(
            "movl %%esp, %%ecx\n\t"
            "movl $1f, %%edx\n\t"
            "sysenter\n\t"
            "1:"
            :"=a"(_ret)
            :"0" (_NR_sys_getticks)
            :"ecx","edx");
    return _ret;
}

int sys_pstat(unsigned int _pid, struct _task_stat *_st_pstat, unsigned int _size)
{
    int _ret;
    __asm__ __volatile__(
            "movl %%esp, %%ecx\n\t"
            "movl $1f, %%edx\n\t"
            "sysenter\n\t"
            "1:"
            :"=a"(_ret)
            :"0" (_NR_sys_pstat),"b"(_pid),"S"(_st_pstat),"D"(_size)
            :"ecx","edx");
    return _ret;
}

int sys_opendir(char * _path)
{
    int _ret;
    __asm__ __volatile__(
            "movl %%esp, %%ecx\n\t"
            "movl $1f, %%edx\n\t"
            "sysenter\n\t"
            "1:"
            :"=a"(_ret)
            :"0" (_NR_sys_opendir),"b"(_path)
            :"ecx","edx");
    return _ret;
}

int sys_readdir(int _handle, struct _file_stat *_st_fstat, unsigned int _size)
{
    int _ret;
    __asm__ __volatile__(
            "movl %%esp, %%ecx\n\t"
            "movl $1f, %%edx\n\t"
            "sysenter\n\t"
            "1:"
            :"=a"(_ret)
            :"0" (_NR_sys_readdir),"b"(_handle),"S"(_st_fstat),"D"(_size)
            :"ecx","edx");
    return _ret;
}

int sys_getdate(struct _rtc_date *_st_rtcdate, unsigned int _size)
{
    int _ret;
    __asm__ __volatile__(
            "movl %%esp, %%ecx\n\t"
            "movl $1f, %%edx\n\t"
            "sysenter\n\t"
            "1:"
            :"=a"(_ret)
            :"0" (_NR_sys_getdate),"b"(_st_rtcdate),"S"(_size)
            :"ecx","edx");
    return _ret;
}

int sys_reboot(unsigned int _mode)
{
    int _ret;
    __asm__ __volatile__(
            "movl %%esp, %%ecx\n\t"
            "movl $1f, %%edx\n\t"
            "sysenter\n\t"
            "1:"
            :"=a"(_ret)
            :"0" (_NR_sys_reboot),"b"(_mode)
            :"ecx","edx");
    return _ret;
}

int sys_sleep(unsigned int _stime)
{
    int _ret;
    __asm__ __volatile__(
            "movl %%esp, %%ecx\n\t"
            "movl $1f, %%edx\n\t"
            "sysenter\n\t"
            "1:"
            :"=a"(_ret)
            :"0" (_NR_sys_sleep),"b"(_stime)
            :"ecx","edx");
    return _ret;
}

#else

int sys_open(char *_filename)
{
    int _ret;
    __asm__ __volatile__(
            "int  $0x80"
            :"=a"(_ret)
            :"0" (_NR_sys_open),"b"(_filename));
    return _ret;
}

int sys_close(unsigned int _handle)
{
    int _ret;
    __asm__ __volatile__(
            "int  $0x80"
            :"=a"(_ret)
            :"0" (_NR_sys_close),"b"(_handle));
    return _ret;
}

int sys_write(int _handle,char * _buffer,int _size)
{
    int _ret;
    __asm__ __volatile__(
            "int  $0x80"
            :"=a"(_ret)
            :"0" (_NR_sys_write),"b"(_handle),"c" (_buffer),"d"(_size));
    return _ret;
}

int sys_read(int _handle,char * _buffer,int _size)
{
    int _ret;
    __asm__ __volatile__(
            "int  $0x80"
            :"=a"(_ret)
            :"0" (_NR_sys_read),"b"(_handle),"c" (_buffer),"d"(_size));
    return _ret;
}

int sys_seek(int _handle, int _offset,unsigned int _mode)
{
    int _ret;
    __asm__ __volatile__(
            "int  $0x80"
            :"=a"(_ret)
            :"0" (_NR_sys_seek),"b"(_handle),"c" (_offset),"d"(_mode));
    return _ret;
}

int sys_wait(unsigned int _pid)
{
    int _ret;
    __asm__ __volatile__(
            "int  $0x80"
            :"=a"(_ret)
            :"0" (_NR_sys_wait),"b"(_pid));
    return _ret;
}

int sys_exit()
{
    int _ret;
    __asm__ __volatile__(
            "int  $0x80"
            :"=a"(_ret)
            :"0" (_NR_sys_exit));
    return _ret;
}


int sys_exec(char * _path)
{
    int _ret;
    __asm__ __volatile__(
            "int  $0x80"
            :"=a"(_ret)
            :"0" (_NR_sys_exec),"b"(_path));
    return _ret;
}

int sys_kill(unsigned int _tid)
{
    int _ret;
    __asm__ __volatile__(
            "int  $0x80"
            :"=a"(_ret)
            :"0" (_NR_sys_kill),"b"(_tid));
    return _ret;
}

int sys_getpid()
{
    int _ret;
    __asm__ __volatile__(
            "int  $0x80"
            :"=a"(_ret)
            :"0" (_NR_sys_getpid));
    return _ret;
}

int sys_getticks()
{
    int _ret;
    __asm__ __volatile__(
            "int  $0x80"
            :"=a"(_ret)
            :"0" (_NR_sys_getticks));
    return _ret;
}

int sys_pstat(unsigned int _pid, struct _task_stat *_st_pstat, unsigned int _size)
{
    int _ret;
    __asm__ __volatile__(
            "int  $0x80"
            :"=a"(_ret)
            :"0" (_NR_sys_pstat),"b"(_pid),"c" (_st_pstat),"d"(_size));
    return _ret;
}

int sys_opendir(char * _path)
{
    int _ret;
    __asm__ __volatile__(
            "int  $0x80"
            :"=a"(_ret)
            :"0" (_NR_sys_opendir),"b"(_path));
    return _ret;
}

int sys_readdir(int _handle, struct _file_stat *_st_fstat, unsigned int _size)
{
    int _ret;
    __asm__ __volatile__(
            "int  $0x80"
            :"=a"(_ret)
            :"0" (_NR_sys_readdir),"b"(_handle),"c" (_st_fstat),"d"(_size));
    return _ret;
}

int sys_getdate(struct _rtc_date *_st_rtcdate, unsigned int _size)
{
    int _ret;
    __asm__ __volatile__(
            "int  $0x80"
            :"=a"(_ret)
            :"0" (_NR_sys_getdate),"b"(_st_rtcdate),"c" (_size));
    return _ret;
}

int sys_reboot(unsigned int _mode)
{
    int _ret;
    __asm__ __volatile__(
            "int  $0x80"
            :"=a"(_ret)
            :"0" (_NR_sys_reboot),"b"(_mode));
    return _ret;
}

int sys_sleep(unsigned int _stime)
{
    int _ret;
    __asm__ __volatile__(
            "int  $0x80"
            :"=a"(_ret)
            :"0" (_NR_sys_sleep),"b"(_stime)
            :"ecx","edx");
    return _ret;
}

#endif

