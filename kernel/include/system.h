#ifndef _SYSTEM_H
#define _SYSTEM_H

#define     KERNEL_CS       0x08
#define     KERNEL_DS       0x10
#define     USER_CS         0x1B
#define     USER_DS         0x23

#define sti()   __asm__ ( "sti"::)      /* 开中断 */
#define cli()   __asm__ ( "cli"::)      /* 关中断 */
#define nop()   __asm__ ( "nop"::)      /* 空操作 */
#define iret()  __asm__ ( "iret"::)     /* 中断返回 */ 
#define idle() do{                                      \
        unsigned __eflags;                              \
        _get_eflags(__eflags);                          \
        if (__eflags & 0x200) __asm__ ( "hlt"::);       \
        else panic("Can not halt when IF not set!");    \
        }while(0)

#define _get_eflags(x)      \
__asm__ ("pushfl \n\t"      \
    "popl %0 \n\t"          \
    :"=g" (x)               \
    :                       \
    :"memory")

#define _local_irq_save(x) 	\
__asm__ ("pushfl \n\t" 		\
	"popl %0 \n\t" 			\
	"cli" 					\
	:"=g" (x) 				\
	:						\
	:"memory")

#define _local_irq_restore(x)  	\
__asm__ ("pushl %0 \n\t" 		\
	"popfl" 					\
	:							\
	:"g" (x) 					\
	:"memory")

#define _set_gate(gate_addr,type,dpl,addr) 	\
__asm__ ("movw %%dx,%%ax\n\t" 				\
	"movw %0,%%dx\n\t" 						\
	"movl %%eax,%1\n\t" 					\
	"movl %%edx,%2" 						\
	: 										\
	: "i" ((short) (0x8000+(dpl<<13)+(type<<8))), \
	"o" (*((char *) (gate_addr))), 			\
	"o" (*(4+(char *) (gate_addr))), 		\
	"d" ((char *) (addr)),"a" (0x00080000))

#define set_intr_gate(n,addr) \
	_set_gate(&idt[n],14,0,addr)

#define set_trap_gate(n,addr) \
	_set_gate(&idt[n],15,0,addr)

#define set_system_gate(n,addr) \
	_set_gate(&idt[n],15,3,addr)

#define lidt(_idt)          \
    __asm__ __volatile__ (  \
        "lidtl  (%%ebx)"    \
        :                   \
        :"b"(_idt)          \
        )


#define lldt(value)         \
    __asm__ __volatile__(   \
        "lldtw     %%ax\n\t" \
        ::"a" (value)       \
        )

#define lgdt(_idt)          \
    __asm__ __volatile__ (  \
        "lgdtl  (%%ebx)"    \
        :                   \
        :"b"(_idt)          \
        )

#define ltr(value)          \
    __asm__ __volatile__(   \
        "ltrw     %%ax\n\t" \
        ::"a" (value)       \
        )
/*
 * 修改cr3寄存器
 */
#define set_cr3(_pgd)       \
    __asm__ __volatile__(   \
        "movl %0, %%cr3"    \
        ::"r"(_pgd))
/*
 * msr寄存器的几个宏
 */
#define SYSENTER_CS_MSR     0x174
#define SYSENTER_ESP_MSR    0x175
#define SYSENTER_EIP_MSR    0x176

#define wrmsr(_msr, _value) \
    __asm__ __volatile__ (  \
        "wrmsr"             \
        :                   \
        :"a"(_value),"c"(_msr),"d"(0))

/*
 *  修改cr0的ts位的宏
 *  用于检测任务是否使用了fpu寄存器
 */
#define set_cr0_ts()                \
     __asm__ __volatile__ (         \
            "movl %%cr0, %%eax\n\t" \
            "orl  $8, %%eax\n\t"    \
            "movl %%eax, %%cr0"     \
            :::"eax")

#define clear_cr0_ts()                      \
     __asm__ __volatile__ (                 \
            "movl %%cr0, %%eax\n\t"         \
            "andl $0xFFFFFFF7, %%eax\n\t"   \
            "movl %%eax, %%cr0"             \
            :::"eax")

/*
 *  保存与恢复fpu寄存器的宏
 */
#define save_fpu(_save_to)          \
     __asm__ __volatile__ (         \
            "fsave (%0)\n\t"        \
            :                       \
            :"g"(_save_to)          \
            :"memory")
#define restore_fpu(_restore_from)  \
     __asm__ __volatile__ (         \
            "frstor (%0)\n\t"       \
            :                       \
            :"g"(_restore_from))

/* 访问标志 */
#define ACS_PRESENT     0x80            				/* present segment */
#define ACS_CSEG        0x18            				/* 代码段 */
#define ACS_DSEG        0x10            				/* 数据段 */
#define ACS_CONFORM     0x04            				/* conforming segment */
#define ACS_READ        0x02            				/* 可读段 */
#define ACS_WRITE       0x02            				/* 可写段 */
#define ACS_IDT         ACS_DSEG        				/* segment type is the same type */
#define ACS_INT_GATE    0x0E            				/* int gate for 386 */
#define ACS_TRAP_GATE   0x0F            				/* TRAP gate for 386 */
#define ACS_INT         (ACS_PRESENT | ACS_INT_GATE) 	/* present int gate */
#define ACS_TRAP 		(ACS_PRESENT| ACS_TRAP_GATE) 	/* present int gate */
#define ACS_TSS_GATE    0x09
#define ACS_TSS         (ACS_PRESENT | ACS_TSS_GATE) 	/* present tss gate */
#define ACS_DPL_0       0x00            				/* descriptor privilege level #0 */
#define ACS_DPL_1       0x20            				/* descriptor privilege level #1 */
#define ACS_DPL_2       0x40            				/* descriptor privilege level #2 */
#define ACS_DPL_3       0x60            				/* descriptor privilege level #3 */
#define ACS_LDT         0x02            				/* ldt descriptor */

/* Ready-made values */
#define ACS_CODE        (ACS_PRESENT | ACS_CSEG | ACS_READ)
#define ACS_DATA        (ACS_PRESENT | ACS_DSEG | ACS_WRITE)
#define ACS_STACK       (ACS_PRESENT | ACS_DSEG | ACS_WRITE)

/* Attributes (in turms of size) */
#define ATTR_GRANULARITY 0x80          				 	/* segment limit is given in 4KB pages rather than in bytes */
#define ATTR_BIG         0x40           				/* ESP is used rather than SP */
#define ATTR_DEFAULT     0x40           				/* 32-bit code segment rather than 16-bit */

typedef struct desc_struct {
	unsigned long a,b;
}__attribute__((packed)) desc_table[256];

/* 段描述符定义 */
typedef struct
{
    u16 limit;
    u16 base_l;
    u8  base_m;
    u8  access;
    u8  attribs;
    u8  base_h;
} __attribute__((packed)) seg_desc;
//上面这个结构使用至代码中可能会导致优化错误

extern desc_table idt,gdt;

#endif
