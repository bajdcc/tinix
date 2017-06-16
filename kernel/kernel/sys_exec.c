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

/*
 * 利用iret降低特权级并转移至_addr
 * _stack为3环esp
 * cs为0x23，ds等为0x1b，rpl均为3
 */
#define iret_to_user(_addr, _stack) \
    __asm__ __volatile__(           \
        "movl $0x23, %%eax\n\t"     \
        "mov %%ax,%%ds\n\t"         \
        "mov %%ax,%%es\n\t"         \
        "mov %%ax,%%fs\n\t"         \
        "mov %%ax,%%gs\n\t"         \
        "pushl $0x23\n\t"           \
        "pushl %0\n\t"              \
        "pushfl\n\t"                \
        "pushl $0x1B\n\t"           \
        "pushl %1\n\t"              \
        "iret\n"                    \
        :                           \
        :"g" (_stack), "g"(_addr) :"ax")               


/*********ELF数据类型***********/
#define Elf32_Addr      u32                 //size 4 无符号程序地址
#define Elf32_Half      u16                 //size 2 无符号中等整数
#define Elf32_Off       u32                 //size 4 无符号文件偏移
#define Elf32_SWord     s32                 //size 4 有符号大整数
#define Elf32_Word      u32                 //size 4 无符号大整数
#define Elf32_Uchar     u8
#define EI_NIDENT       16
typedef struct{ 
    Elf32_Uchar         e_ident[EI_NIDENT];
    Elf32_Half          e_type;
    Elf32_Half          e_machine;
    Elf32_Word          e_version;
    Elf32_Addr          e_entry;
    Elf32_Off           e_phoff;
    Elf32_Off           e_shoff;
    Elf32_Word          e_flags;
    Elf32_Half          e_ehsize;
    Elf32_Half          e_phentsize;
    Elf32_Half          e_phnum;
    Elf32_Half          e_shentsize;
    Elf32_Half          e_shnum;
    Elf32_Half          e_shstrndx; 
} __attribute__((packed)) Elf32_Ehdr;
typedef struct{
    Elf32_Word          p_type;
    Elf32_Off           p_offset;
    Elf32_Addr          p_vaddr;
    Elf32_Addr          p_paddr;
    Elf32_Word          p_filesz;
    Elf32_Word          p_memsz;
    Elf32_Word          p_flags;
    Elf32_Word          p_align;
} __attribute__((packed)) Elf32_Phdr;

/*
 * 所有新任务的入口代码
 *
 * 此函数的主要功能是解析执行体elf格式，并将文件内容读入该任务内存
 * 进入此函数时，新任务的0号句柄已经指向该任务执行体文件
 * 故，此处直接对0号句柄fself操作，即可解析执行文件
 * 此文件不需open，也不需close，也不能close
 * 加载完毕之后，将通过iret_to_user直接进入用户态
 */
static void new_task_entry()
{
    u32 fself = 0;
    u32 fsize;
    u32 i;
    Elf32_Ehdr selfhead;
    Elf32_Phdr selfphdr;

    fsize = sys_seek(fself, 0, SEEK_END);
    sys_seek(fself, 0, SEEK_SET);
    sys_read(fself, (u8 *)&selfhead, sizeof(Elf32_Ehdr));

    if (selfhead.e_phnum*selfhead.e_phentsize + selfhead.e_phoff > fsize)
    {
        warning("Task_entry: not a valid elf. Task %d.", current);
        sys_exit();
    }

    for (i=0; i<selfhead.e_phnum; i++)
    {
        sys_seek(fself, selfhead.e_phoff+i*selfhead.e_phentsize, SEEK_SET);
        sys_read(fself, (u8 *)&selfphdr, sizeof(Elf32_Phdr));
        
        /*
         *  如果需要的p_memsz正好页对齐，则按照需要申请即可
         *  否则需要给出p_memsz的向上对齐值
         */
        if (selfphdr.p_memsz % PAGE_SIZE != 0)
        {
            u_get_pages(selfphdr.p_memsz/PAGE_SIZE+1, 
                selfphdr.p_vaddr, 
                selfphdr.p_type);
        } else {
            u_get_pages(selfphdr.p_memsz/PAGE_SIZE, 
                selfphdr.p_vaddr, 
                selfphdr.p_type);
        }

        sys_seek(fself, selfphdr.p_offset, SEEK_SET);
        sys_read(fself, (u8 *)selfphdr.p_vaddr, selfphdr.p_filesz);
    }

    /*
     *  在PAGE_OFFSET向下两页位置取两个页
     *  作为该任务的用户态栈
     */
    u_get_pages(2, PAGE_OFFSET-2*PAGE_SIZE, 1);

    /* iret to user */
    iret_to_user(selfhead.e_entry, PAGE_OFFSET-4);

}

/*
 * elf文件格式的一个初步校验
 */
static s32 verify_elf(u32 fhandle)
{
    Elf32_Ehdr fhead;
    sys_seek(fhandle, 0, SEEK_SET);
    sys_read(fhandle, (u8 *)&fhead, sizeof(Elf32_Ehdr));

    if (fhead.e_ident[0] != 0x7F || \
        fhead.e_ident[1] != 'E'  || \
        fhead.e_ident[2] != 'L'  || \
        fhead.e_ident[3] != 'F')
    {
        return -1;
    }

    if (fhead.e_ident[4] != 1)  return -1;          /* 标识文件的类别，或者说，容量。必须为1 */
    if (fhead.e_ident[5] != 1)  return -1;          /* 数据编码方式。必须为1，高位在前 */
    if (fhead.e_ident[6] != 1)  return -1;          /* 版本 必须为1，高位在前 */

    if (fhead.e_type != 2)      return -1;          /* 文件类型，2为可执行文件 */
    if (fhead.e_machine != 3)   return -1;          /* 机器类型，3是386 */
    if (fhead.e_version != 1)   return -1;          /* 版本 1为当前版本 */
    if (fhead.e_flags != 0)     return -1;          /* IA-32必须为 0 */
    if (fhead.e_ehsize != 0x34) return -1;          /* 头部大小必须为 0x34 */
    if (fhead.e_phentsize != 0x20) return -1;       /* Program header 条目大小 */
    if (fhead.e_shentsize != 0x28) return -1;       /* Section header 条目大小 */
    if (fhead.e_entry >= PAGE_OFFSET) return -1;    /* 非法入口点 */

    return 0;
}

s32 sys_exec(u8 * path)
{
    s32 fhandle,f_exe_id;
    fhandle = sys_open(path);
    if (fhandle < 0) 
    {
        warning("Sys_exec: file not found. file:%s", path);
        return -1;
    }
    if (verify_elf(fhandle))
    {
        warning("Sys_exec: not a elf file. file:%s", path);
        sys_close(fhandle);
        return -1;
    }

    f_exe_id = task_list[current].fid[fhandle];

    /* 有时候执行体就是自己，不需要关闭 */
    if(fhandle != 0)
        sys_close(fhandle);
    return task_add_task(f_exe_id, (void *)&new_task_entry);
}
