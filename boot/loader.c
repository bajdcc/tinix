#include <multiboot.h>
#include <console.h>

typedef signed   char       s8;
typedef unsigned char       u8;
typedef signed   short      s16;
typedef unsigned short      u16;
typedef signed   int        s32;
typedef unsigned int        u32;
typedef signed   long long  s64;
typedef unsigned long long  u64;

#define CHECK_FLAG(flags,bit)   ((flags) & (1 << (bit)))
#define die()                   {while(1){}}

#define     MEM_SIZE            ((u32 *)0x90000)
#define     CUR_POS             ((u32 *)0x90004)
#define     APM_ADDR            ((u32 *)0x90008)

/*
 *  内核镜像的起始地址和结束地址
 *  在boot.S中定义
 */
extern u8 kernels[];
extern u8 kernele[];

#ifdef NO_FLOPPY
extern u8 floppy_s[];
extern u8 floppy_e[];
#endif


static void load_kernel()
{
	u32  ksize  =(u32)(kernele-kernels);
	u8 * kaddr  =(u8*)0;
	register u32  i;

    /*
     * 将低端内存清零
     */
    for(i = 0; i < 0x90000; i += 4)   
        *(u32 *)i = 0;

    dprint (ATTR_OK, "Kernel size %d bytes.", ksize);

    /*
     * 将内核拷贝至零地址
     */
	for(i = 0; i < ksize; i++)   
        *kaddr++ = kernels[i];
    
    /*
     * 低端内存可能不连续
     * 此处进行简单的内存校验
     */
    for(i = 0; i < ksize; i++)
    {
        if (*(u8 *)i != kernels[i])
        {
            dprint (ATTR_ERROR, "Loading kernel error at 0x%x", i);
        }
    }
    dprint (ATTR_OK, "Kernel loading complete!");

    #ifdef NO_FLOPPY
    for(i = 0; i < 0x168000; i++)    
        *(u8 *)(i + 0x105000) = floppy_s[i];

    dprint (ATTR_OK, "Load floppy data to 0x105000!");
    #endif
}

static void save_bootinfo(multiboot_info_t *bootinfo)
{

    if (CHECK_FLAG(bootinfo->flags, 0))
    {
        *MEM_SIZE = (bootinfo->mem_upper * 1024) & 0xfffff000;
        if (*MEM_SIZE < 0x2000000)
        {
            dprint (ATTR_ERROR, 
                "Total available memory %u MB. Memory is too small.", 
                (*MEM_SIZE)/(1024*1024)-1);
            die();
        }
        dprint (ATTR_OK, 
            "Total available memory %u MB.", 
            (*MEM_SIZE)/(1024*1024)-1);
    }
    if (CHECK_FLAG(bootinfo->flags, 10))
    {
        apm_info * papm = (apm_info *)bootinfo->apm_table;

        *APM_ADDR = (papm->cseg << 16) | papm->offset;
        dprint (ATTR_OK, "APM bios offset is %x.", *APM_ADDR);
    }
}


void loader_main (u32 magic, multiboot_info_t *bootinfo)
{
    cls ();
    
    if (magic == MULTIBOOT_BOOTLOADER_MAGIC)
    {
        dprint (ATTR_OK, "Boot from grub!");
        save_bootinfo(bootinfo);
        dprint (ATTR_OK, "Loading kernel...");
        load_kernel();
        dprint (ATTR_OK, "Jump to kernel!");
        *CUR_POS = (xpos + ypos * COLUMNS) * 2;
    }
    else
    {
        dprint (ATTR_ERROR, "Invalid magic number: 0x%x", (unsigned) magic);
        dprint (ATTR_ERROR, "Only can boot from grub!");
        die();
    }
}
