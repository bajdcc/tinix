#include <include/types.h>
#include <include/system.h>
#include <include/stdarg.h>
#include <include/timer.h>
#include <include/syscall.h>
#include <include/string.h>
#include <mm/memory.h>
#include <dev/floppy.h>
#include <dev/console.h>
#include <fs/fs.h>

/*
 *  cpuinfo操作接口及cpuinfo读写跳转表
 */
void init_cpuinfo();
s32 cpuinfo_seek(file *, s32, u32);
s32 cpuinfo_read(file *, u8*, u32);
file_operations cpuinfo_ops = {
    (void *)cpuinfo_seek,
    (void *)cpuinfo_read,
    (void *)0
};
u8 cpuinfo_buff[400];

/*
 *  meminfo操作接口及meminfo读写跳转表
 */
void init_meminfo();
s32 meminfo_seek(file *, s32, u32);
s32 meminfo_read(file *, u8*, u32);
file_operations meminfo_ops = {
    (void *)meminfo_seek,
    (void *)meminfo_read,
    (void *)0
};
u8 meminfo_buff[400];

/*
 *  sysinfo操作接口及sysinfo读写跳转表
 */
void init_sysinfo();
s32 sysinfo_seek(file *, s32, u32);
s32 sysinfo_read(file *, u8*, u32);
file_operations sysinfo_ops = {
    (void *)sysinfo_seek,
    (void *)sysinfo_read,
    (void *)0
};
u8 sysinfo_buff[400];

s32 cpuinfo_seek(file *cpudev, s32 offset, u32 mode)
{
    switch(mode)
    {
        case SEEK_SET:
        {
            if (offset >= 0 && offset < cpudev->f_size)
            {
                cpudev->f_pos[current] = offset;
                return cpudev->f_pos[current];
            }
            return -1;
        }
        case SEEK_CUR:
        {
            if (offset+cpudev->f_pos[current] >= 0 && \
                offset+cpudev->f_pos[current] < cpudev->f_size)
            {
                cpudev->f_pos[current] += offset;
                return cpudev->f_pos[current];
            }
            return -1;
        }
        case SEEK_END:
        {
            if (offset+cpudev->f_size >= 0 && offset <= 0)
            {
                cpudev->f_pos[current] = offset+cpudev->f_size;
                return cpudev->f_pos[current];
            }
            return -1;
        }    

    }
    return -1;
}

s32 cpuinfo_read(file *cpudev, u8* buff, u32 size)
{
    if (size == 0 || cpudev->f_pos[current]+size > cpudev->f_size) 
        return -1;
    memcpy(buff, (void *)(cpudev->f_base+cpudev->f_pos[current]), size);
    cpudev->f_pos[current] += size;
    return size;
}

s32 meminfo_seek(file *memdev, s32 offset, u32 mode)
{
    switch(mode)
    {
        case SEEK_SET:
        {
            if (offset >= 0 && offset < memdev->f_size)
            {
                memdev->f_pos[current] = offset;
                return memdev->f_pos[current];
            }
            return -1;
        }
        case SEEK_CUR:
        {
            if (offset+memdev->f_pos[current] >= 0 && \
                offset+memdev->f_pos[current] < memdev->f_size)
            {
                memdev->f_pos[current] += offset;
                return memdev->f_pos[current];
            }
            return -1;
        }
        case SEEK_END:
        {
            if (offset+memdev->f_size >= 0 && offset <= 0)
            {
                memdev->f_pos[current] = offset+memdev->f_size;
                return memdev->f_pos[current];
            }
            return -1;
        }    

    }
    return -1;
}

s32 meminfo_read(file *memdev, u8* buff, u32 size)
{
    if (size == 0 || memdev->f_pos[current]+size > memdev->f_size) 
        return -1;
    
    init_meminfo();
    memcpy(buff, (void *)(memdev->f_base+memdev->f_pos[current]), size);
    memdev->f_pos[current] += size;
    return size;
}

s32 sysinfo_seek(file *sysinfodev, s32 offset, u32 mode)
{
    switch(mode)
    {
        case SEEK_SET:
        {
            if (offset >= 0 && offset < sysinfodev->f_size)
            {
                sysinfodev->f_pos[current] = offset;
                return sysinfodev->f_pos[current];
            }
            return -1;
        }
        case SEEK_CUR:
        {
            if (offset+sysinfodev->f_pos[current] >= 0 && \
                offset+sysinfodev->f_pos[current] < sysinfodev->f_size)
            {
                sysinfodev->f_pos[current] += offset;
                return sysinfodev->f_pos[current];
            }
            return -1;
        }
        case SEEK_END:
        {
            if (offset+sysinfodev->f_size >= 0 && offset <= 0)
            {
                sysinfodev->f_pos[current] = offset+sysinfodev->f_size;
                return sysinfodev->f_pos[current];
            }
            return -1;
        }    

    }
    return -1;
}

s32 sysinfo_read(file *sysinfodev, u8* buff, u32 size)
{
    if (size == 0 || sysinfodev->f_pos[current]+size > sysinfodev->f_size) 
        return -1;
    
    init_sysinfo();
    memcpy(buff, (void *)(sysinfodev->f_base+sysinfodev->f_pos[current]), size);
    sysinfodev->f_pos[current] += size;
    return size;
}

int sprintf(char * buffer, const char *fmt, ...)
{
    int i;
    va_list args;
    va_start(args, fmt);
    i = vsprintf(buffer ,fmt ,args);
    va_end(args);

    return i;
}

/*
 *  只在init sysfs中调用一次
 *  理论上cpu是不会变的
 */
void init_cpuinfo()
{
    u32 eax,ebx,ecx,edx;
    char tmp[128];

    memset(cpuinfo_buff, 0, 400);

    /**
     * eax == 0
     * eax    : cpuid指令允许的最大eax输入值
     * ebx    : "Genu"
     * ecx    : "ntel"
     * edx    : "inel"
     */
    __asm__ __volatile__ (
        "cpuid"
        : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
        : "0"(0) );

    strcat(cpuinfo_buff, "Vendor    : ");
    strncat(cpuinfo_buff, (char *)&ebx, 4);
    strncat(cpuinfo_buff, (char *)&edx, 4);
    strncat(cpuinfo_buff, (char *)&ecx, 4);
    strcat(cpuinfo_buff, "\n");

    /*
     *  eax == 1，则在eax中返回Family/Model/Stepping等信息
     *  [0:3]    stepping
     *  [4:7]    model
     *  [8:11]    family
     *  [12:13]    processor type
     *  [16:19]    extended model ID
     *  [20:27]    extended family ID
     */
    __asm__ __volatile__ (
        "cpuid"
        : "=a"(eax)
        : "0"(1)
        );

    sprintf(tmp, "Ext Family: %d\n", (0xff00000 & eax) >> 20);
    strcat(cpuinfo_buff, tmp);
    sprintf(tmp, "Ext Model : %d\n", (0xf0000 & eax) >> 16);
    strcat(cpuinfo_buff, tmp);
    sprintf(tmp, "Family    : %d\n", (0xf00 & eax) >> 8);
    strcat(cpuinfo_buff, tmp);
    sprintf(tmp, "Model     : %d\n", (0xf0 & eax) >> 4);
    strcat(cpuinfo_buff, tmp);
    sprintf(tmp, "Stepping  : %d\n", (0xf & eax));
    strcat(cpuinfo_buff, tmp);

    /*
     * eax == 0x800000000
     * 如果CPU支持Brand String，则在EAX中返 >= 0x80000004的值。
     */
    __asm__ __volatile__ (
        "cpuid"
        : "=a"(eax)
        : "0"(0x80000000)
        );

    /*
     * 如果支持Brand String，则EAX从0x80000002到0x80000004，每次增1，CPUID指令返回：
     * EAX    : Processor Brand String
     * EBX    : Processor Brand String Continued
     * ECX    : Processor Brand String Continued
     * EDX    : Processor Brand String Continued
     */

    if(eax >= 0x80000004) {
        unsigned int brands[4]; //每次的eax、ebx、ecx、edx

        unsigned int i;

        strcat(cpuinfo_buff,"Brand     : ");

        memset(tmp, 0, 128);
        for (i = 0x80000002; i <= 0x80000004; i++) {
            __asm__ __volatile__ (
                "cpuid"
                : "=a"(brands[0]), "=b"(brands[1]), "=c"(brands[2]), "=d"(brands[3])
                : "0" (i)
                );
            strncat(tmp, (char *)brands, 16);
        }
        strcat(cpuinfo_buff, tmp);
        strcat(cpuinfo_buff, "\n");
    }


    /*
     * eax == 80000006h，返回L2 Cache的信息
     * 
     * ecx[31:16]    : L2 Cache size, in Kbytes
     * ecx[15:12]    : L2 Cache Associativity
     *           00h disabled
     *           01h direct mapped
     *           02h 2-Way
     *           04h 4-Way
     *           06h 8-Way
     *           08h 16-Way
     *           0Fh Fully associative
     * ecx[7:0]    : L2 Cache Line size in bytes
     */
    __asm__ __volatile__ (
        "cpuid"
        : "=c"(ecx)
        : "a"(0x80000006)
        );
    sprintf(tmp, "L2 Cache  : %d Kbytes\n", ( ecx >> 16 ) );
    strcat(cpuinfo_buff, tmp);
    sprintf(tmp, "Cache Alig: %d bytes\n", (ecx & 0xff));
    strcat(cpuinfo_buff, tmp);

    sprintf(tmp, "Cache Asso: ");
    strcat(cpuinfo_buff, tmp);
    switch ( (ecx & 0xf000) >> 12 )
    {
        case 0x00:
            strcat(cpuinfo_buff, "disabled\n");
            break;
        case 0x01:
            strcat(cpuinfo_buff, "direct mapped\n");
            break;
        case 0x02:
            strcat(cpuinfo_buff, "2-Way\n");
            break;
        case 0x04:
            strcat(cpuinfo_buff, "4-Way\n");
            break;
        case 0x06:
            strcat(cpuinfo_buff, "8-Way\n");
            break;
        case 0x08:
            strcat(cpuinfo_buff, "16-Way\n");
            break;
        case 0x0f:
            strcat(cpuinfo_buff, "Fully associative\n");
            break;
        default:
            strcat(cpuinfo_buff, "No such entry...\n");
    }

    /*
     * Input : eax == 4 && ecx == 0
     *
     * (eax[31:26] + 1) 是该物理处理器package上实现的core CPUs数目
     */
    __asm__ __volatile__ (
        "cpuid"
        : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
        : "0"(4), "2"(0)
);
    sprintf(tmp, "Nr of Core: %d\n", (eax >> 27) + 1 );
    strcat(cpuinfo_buff, tmp);
}


void init_meminfo()
{
    u32 memtotal,used,freed,u_use,k_use;
    u8 tmp[64];

    memset(meminfo_buff, 0, 400);

    calc_physical_mem(&memtotal, &used, &k_use);
    u_use = used - k_use;
    freed = memtotal - used;

    sprintf(tmp, "MemTotal  : %-7d Kbytes\n", memtotal/1024);
    strcat(meminfo_buff, tmp);

    sprintf(tmp, "MemFree   : %-7d Kbytes\n", freed/1024);
    strcat(meminfo_buff, tmp);

    sprintf(tmp, "KernelUsed: %-7d Kbytes\n", k_use/1024);
    strcat(meminfo_buff, tmp);

    sprintf(tmp, "UserUsed  : %-7d Kbytes\n", u_use/1024);
    strcat(meminfo_buff, tmp);
    return;
}

void init_sysinfo()
{
    memset(sysinfo_buff, 0, 400);
    sprintf(
        sysinfo_buff,
        "syscall-counter %-7d\nsyscall-error   %-7d\n",
        syscall_counter,
        syscall_error);
}

void init_sysfs()
{
    init_cpuinfo();
    init_meminfo();
    init_sysinfo();
    fs_add_file(DIR_SYS, 0, (u32)cpuinfo_buff, strlen(cpuinfo_buff), &cpuinfo_ops, 0, "cpuinfo");
    fs_add_file(DIR_SYS, 0, (u32)meminfo_buff, strlen(meminfo_buff), &meminfo_ops, 0, "meminfo");
    fs_add_file(DIR_SYS, 0, (u32)sysinfo_buff, strlen(sysinfo_buff), &sysinfo_ops, 0, "sysinfo");
}

