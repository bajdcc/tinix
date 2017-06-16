/**/

#ifndef _MEMORY_H
#define _MEMORY_H

#include <include/types.h>

#define PAGE_SIZE           4096
#define _1M_SIZE            0x00100000
#define PAGE_OFFSET         0xC0000000
#define K_PAGE_DIR          0xC0100000      /* 页目录地址 */
#define K_PAGE_PTE0         0xC0101000      /* 内核使用的第一个页表 */
#define K_PAGE_PTE1         0xC0102000      /* 内核使用的第二个页表 */
#define K_PAGE_PTE2         0xC0103000      /* 内核使用的第三个页表 */
#define K_PAGE_PTE3         0xC0104000      /* 内核使用的第四个页表 */

#ifdef  NO_FLOPPY
#define PAGE_MAP            0xC026D000
#else
#define PAGE_MAP            0xC0105000      /* 物理内存管理 */
#endif

#define PAGE_PRESENT    0x001
#define PAGE_RW         0x002
#define PAGE_USER       0x004
#define PAGE_PWT        0x008   /* 486 only - not used currently */
#define PAGE_PCD        0x010   /* 486 only - not used currently */
#define PAGE_ACCESSED   0x020
#define PAGE_DIRTY      0x040
#define PAGE_COW        0x200   /* implemented in software (one of the AVL bits) */

#define PAGE_PRIVATE    (PAGE_PRESENT | PAGE_RW | PAGE_USER | PAGE_ACCESSED | PAGE_COW)
#define PAGE_SHARED     (PAGE_PRESENT | PAGE_RW | PAGE_USER | PAGE_ACCESSED)
#define PAGE_COPY       (PAGE_PRESENT | PAGE_USER | PAGE_ACCESSED | PAGE_COW)
#define PAGE_READONLY   (PAGE_PRESENT | PAGE_USER | PAGE_ACCESSED)
#define PAGE_TABLE      (PAGE_PRESENT | PAGE_RW | PAGE_USER | PAGE_ACCESSED)

#define PAGE_ALIGN(_x)  ((_x)&0xFFFFF000)


/* 用刷新cr3的方法来刷新TLB */
#define __flush_tlb()                               \
    do {                                            \
        unsigned int tmpreg;                        \
        __asm__ __volatile__(                       \
                "movl %%cr3, %0;              \n"   \
                "movl %0, %%cr3;  # flush TLB \n"   \
                : "=r" (tmpreg)                     \
                :: "memory");                       \
    } while (0)


extern void init_mem();
extern void k_free_page(u32);
extern u32  k_get_page();
extern u32  u_get_page_table();
extern u32  u_get_pages(u32, u32, u32);
extern void u_free_page_table(u32);
extern u32  calc_taskmm(u32);
extern void calc_physical_mem(u32 *, u32 *, u32 *);
#endif
