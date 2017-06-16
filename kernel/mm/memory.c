#include <include/types.h>
#include <include/stdarg.h>
#include <include/timer.h>
#include <include/system.h>
#include <include/string.h>
#include <include/task.h>
#include <include/syscall.h>
#include <mm/memory.h>
#include <dev/console.h>

#define BOOT_MEMSIZE        ((u32 *)0xC0090000)


static u8 * mem_map = (u8*)PAGE_MAP;
static u32  page_max;
static u32  start_mm_index = 0;

/*
 * 取得一个用户态空闲页索引
 *
 * 为方便内核态线性地址和物理地址之间的相互转换
 * 内核态映射关系必须为 线性地址 - 物理地址 = PAGE_OFFSET
 * 因此内核态物理内存的查找是从头开始的
 * 为了减少冲突，用户态是倒着从尾部找的
 */
static u32 u_get_page_index()
{
    u32 page_index;
    u32 eflags;
    _local_irq_save(eflags);

    for (page_index = page_max-1; page_index > start_mm_index; page_index--) {
        if (! mem_map[page_index]) 
        {
            mem_map[page_index] = 1;
            _local_irq_restore(eflags);
            return page_index;
        }
    }
    panic("Memory exhausted!!");
}


/*
 * 将指定页page_index映射至当前任务pgd的put_addr处
 */
static void u_put_page(u32 page_index, u32 put_addr, u32 type)
{
    u32 pte;

    /*
     * 没有对应的pte,建立之
     */
    u32 * current_pgd = (u32 *)(task_list[current].pgd+PAGE_OFFSET);
    if (current_pgd[put_addr>>22] == 0 )   
    {
        pte = k_get_page();
        memset((void *)pte, 0, PAGE_SIZE);
        current_pgd[put_addr>>22] = (pte-PAGE_OFFSET) | PAGE_USER | PAGE_RW | PAGE_PRESENT;

        __flush_tlb();
    }
    else
    {
        pte = current_pgd[put_addr>>22];
        pte = (pte&0xFFFFF000) + PAGE_OFFSET;
    }

    /*
     * 修改pte 建立映射
     * 如果pte中的该项非0，则内核内存管理必然出错
     */
    if (((u32 *)pte)[(put_addr&0x3FF000)>>12] != 0)
    {
        warning("Task %d paging error at 0x%08X, pte 0x%08X!!",
            current, put_addr, pte);
        sys_exit();
    }
    
    ((u32 *)pte)[(put_addr&0x3FF000)>>12] = 
        (page_index<<12) | PAGE_USER | PAGE_RW | PAGE_PRESENT;

    __flush_tlb();
}

/*
 * 取多个连续的用户态空闲页， 并建立映射
 */
u32 u_get_pages(u32 pages, u32 put_addr, u32 type)
{
    u32 i;
    for (i = 0; i < pages; i++)
    {
        u_put_page(u_get_page_index(), put_addr+i*PAGE_SIZE, type);
    }

    memset((void *)put_addr, 0, pages*PAGE_SIZE);
    return put_addr;
}

/*
 * 取得一个内核态空闲页，
 * 并复制k_page_dir内容，作为新任务的pgd
 */
u32 u_get_page_table()
{
    u32 i;
    u32 page = k_get_page();
    for (i = 0; i<1024; i++)
        ((u32 *)page)[i] = ((u32 *)K_PAGE_DIR)[i];
    return page;
}


/*
 * 释放当前任务pgd 中的用户态相关的所有内容
 */
void u_free_page_table(u32 pgd)
{
    u32 i,j;
    u32 vpgd = pgd;
    u32 vpte;
    u32 tmp_index;

    /*
     * 遍历用户态pte入口
     */
    for (i = 0; i<768; i++)
    {
        if (((u32 *)vpgd)[i] & PAGE_PRESENT)
        {
            /*
             * 找到一个已提交的pte 遍历之
             */
            vpte = (((u32 *)vpgd)[i]&0xFFFFF000) + PAGE_OFFSET;
            for (j = 0; j<1024; j++)
            {
                if (((u32 *)vpte)[j] & PAGE_PRESENT)
                {
                    /*
                     * pte中发现一个已映射的页，释放物理内存
                     */
                    tmp_index = ((u32 *)vpte)[j]>>12;
                    mem_map[tmp_index] = 0;
                }
            }

            /*
             * pte中所有入口释放完之后，把pte本身也释放掉
             */
            k_free_page(vpte);
        }
    }
    /*
     * 至此，用户态所有内存已释放完毕
     */
}



/*
 * 取得内核态一个空闲页，并建立映射
 *
 * 为方便内核态线性地址和物理地址之间的相互转换
 * 内核态映射关系必须为 线性地址 - 物理地址 = PAGE_OFFSET
 * 因此内核态物理内存的查找是从头开始的
 * 同时，因为只为内核态保留了4个页表
 * 内核态可用的物理内存总计为16M，超过即出错
 */
u32 k_get_page()
{
    u32 page_index,vaddr,vpte;          /* 页号,页线性地址,对应pte线性地址*/
    u32 eflags;
    _local_irq_save(eflags);

    for (page_index = start_mm_index; page_index < page_max; page_index++) {
        if (! mem_map[page_index]) 
        {
            mem_map[page_index] = 1;
            goto paging;
        }
    }
    panic("Memory exhausted!!");

paging:
    _local_irq_restore(eflags);

    vaddr = page_index*PAGE_SIZE+PAGE_OFFSET;           /* 先计算出页号对应的线性地址 */
    if ( ((u32 *)K_PAGE_DIR)[vaddr>>22] == 0 )          /* 没有对应的pte,则内核态内存不足 */
        panic("Memory for kernel exhausted!!");

    vpte = (((u32 *)K_PAGE_DIR)[vaddr>>22] &0xFFFFF000) + PAGE_OFFSET;  /* 计算出pte的线性地址 */
    if (((u32 *)vpte)[(vaddr&0x3FF000)>>12] != 0)       /* 如果pte对应项非0 则内核内存管理必然出错 */
        panic("Paging error at 0x%08X, pte 0x%08X!!", vaddr, vpte);

    /* 修改pte的对应项 将page_index 映射至3G高位 */
    ((u32 *)vpte)[(vaddr&0x3FF000)>>12] = (page_index<<12)  | PAGE_RW | PAGE_PRESENT;

    __flush_tlb();
    return vaddr;
}

/*
 * 释放内核态一个页，并取消映射
 */
void k_free_page(u32 vaddr)
{
    u32 vpte;                                           /* 页号,对应pte线性地址*/

    if ( ((u32 *)K_PAGE_DIR)[vaddr>>22] == 0 )          /* 没有对应的pte,则内核内存管理必然出错 */
        panic("Unpaging pgd error, vaddr 0x%08X!!", vaddr);

    vpte = (((u32 *)K_PAGE_DIR)[vaddr>>22] &0xFFFFF000) + PAGE_OFFSET;  /* 计算出pte的线性地址 */
    if (((u32 *)vpte)[(vaddr&0x3FF000)>>12] == 0)       /* 如果pte对应项为0 则内核内存管理必然出错 */
        panic("Unpaging pte error, vaddr 0x%08X, pte 0x%08X!!", vaddr, vpte);

    /* 修改pte的对应项 取消映射 */
    ((u32 *)vpte)[(vaddr&0x3FF000)>>12] = 0;
    /* 释放物理内存 */
    if (mem_map[(vaddr-PAGE_OFFSET)>>12] != 1)
        panic("Free phymem error, page index %d!!", (vaddr-PAGE_OFFSET)>>12);

    mem_map[(vaddr-PAGE_OFFSET)>>12] = 0;
    //dmesg("kernel free page %d", (vaddr-PAGE_OFFSET)>>12);

    __flush_tlb();
    return;
}


/*
 * 计算任务使用的内存
 * 此函数只计算用户态，比较简单
 */
u32  calc_taskmm(u32 tid)
{
    u32 used = 0;
    u32 vpgd = task_list[tid].pgd + PAGE_OFFSET;
    u32 vpte;
    u32 i,j;

    /* 遍历用户态pte入口 */
    for (i = 0; i<768; i++)
    {
        if (((u32 *)vpgd)[i] & PAGE_PRESENT)
        {
            /* 找到一个已提交的pte 遍历之 */
            vpte = (((u32 *)vpgd)[i]&0xFFFFF000) + PAGE_OFFSET;
            for (j = 0; j<1024; j++)
            {
                if (((u32 *)vpte)[j] & PAGE_PRESENT)
                {
                    /* pte中发现一个已映射的页，释放物理内存 */
                    used += PAGE_SIZE;
                }
            }
        }
    }
    return used;
}

/*
 * 计算物理内存使用率
 */
void calc_physical_mem(u32 *memtotal, u32 *used, u32 *k_use)
{
    u32 i;

    *used = 0;
    *k_use = 0;
    *memtotal = (page_max-1)*PAGE_SIZE;
    for (i = 0; i < page_max; i++)
    {
        if (mem_map[i]) *used += PAGE_SIZE;
    }

    /*
     * 计算四张内核pte的提交个数
     */
    for (i = 0; i < PAGE_SIZE/4; i++)
    {
        if (((u32 *)K_PAGE_PTE0)[i] & PAGE_PRESENT)
        {
            *k_use += PAGE_SIZE;
        }

        if (((u32 *)K_PAGE_PTE1)[i] & PAGE_PRESENT)
        {
            *k_use += PAGE_SIZE;
        }

        if (((u32 *)K_PAGE_PTE2)[i] & PAGE_PRESENT)
        {
            *k_use += PAGE_SIZE;
        }

        if (((u32 *)K_PAGE_PTE3)[i] & PAGE_PRESENT)
        {
            *k_use += PAGE_SIZE;
        }
    }

}

/*
 *  内存管理初始化，仅在init/main.c中调用
 */
void init_mem()
{
    u32 index;
    start_mm_index = 0;
    page_max = (*BOOT_MEMSIZE)/PAGE_SIZE;               /* 物理内存页数 */
    start_mm_index += (PAGE_MAP-PAGE_OFFSET)/PAGE_SIZE; /* 开始管理索引 + 内核占用页数 */
    start_mm_index += page_max/PAGE_SIZE + 1;           /* 开始管理索引 + mem_map 占用页数 */
    
    /* 初始化内存管理map */
    for (index = 0; index < page_max; index++)
    {
        if (index < start_mm_index) 
            mem_map[index] = 1;                         /* 内核已使用内存标记 */
        else mem_map[index] = 0;                        /* 物理内存空闲标记 */
    }

    /*
     * 初始化pte0 使用多少映射多少
     * 其余三个为内核保留的pte均清零
     */
    for (index = 0; index < PAGE_SIZE/4; index++)       /* PAGE_SIZE/4=pte地址个数 */
    {
        if (index < start_mm_index)
            ((u32 *)K_PAGE_PTE0)[index] = (index*PAGE_SIZE) | PAGE_RW | PAGE_PRESENT;
        else
            ((u32 *)K_PAGE_PTE0)[index] = 0;
            
        ((u32 *)K_PAGE_PTE1)[index] = 0;
        ((u32 *)K_PAGE_PTE2)[index] = 0;
        ((u32 *)K_PAGE_PTE3)[index] = 0;
    }

    /*
     * 建立4m - 16m的pte 
     * 内核内存使用不能超过这么多
     */
    ((u32 *)K_PAGE_DIR)[0x301] = (K_PAGE_PTE1 - PAGE_OFFSET) | PAGE_RW | PAGE_PRESENT;
    ((u32 *)K_PAGE_DIR)[0x302] = (K_PAGE_PTE2 - PAGE_OFFSET) | PAGE_RW | PAGE_PRESENT; 
    ((u32 *)K_PAGE_DIR)[0x303] = (K_PAGE_PTE3 - PAGE_OFFSET) | PAGE_RW | PAGE_PRESENT;  
    ((u32 *)K_PAGE_DIR)[0] = 0; /* 取消低4m映射关系 */
    __flush_tlb();
}

