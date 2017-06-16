/* head.S - kernel entry */

#define     PAGE_OFFSET     0xC0000000
#define     PAGE_DIR        0x00100000              /* PDE */
#define     PAGE_PTE_1      0x00101000              /* PTE 1 */
#define     KERNEL_CS       0x08
#define     KERNEL_DS       0x10
#define     USER_CS         0x18
#define     USER_DS         0x23

.text
.globl  start, _gdt_pdesc, _idt_pdesc, _idt, _gdt
start:
    movl    $0x90000, %esp                          /* 重设esp */
    call    init_page                               /* 初始化页表 */
    call    init_gdt                                /* 开完分页后更新gdt */
    
    call    init_8259a                              /* 初始化中断控制器 */
    call    init_idt                                /* 初始化并加载idt */
    call    _k_main                                 /* C语言内核入口,再见at&t汇编! */
init_gdt:
    lgdtl   (_gdt_pdesc)
    movw    $KERNEL_DS, %ax                         /* 更新gdt 更新段寄存器 */
    movw    %ax, %fs
    movw    %ax, %ds
    movw    %ax, %es
    movw    %ax, %ss
    movw    %ax, %gs
    popl    %eax
    pushl   $KERNEL_CS
    pushl   %eax
    retf
/*****************************************************************/
/*                                                               */
/*                 初始化页目录页表                              */
/*                                                               */
/*****************************************************************/
init_page:
    xorl    %ecx, %ecx
    movl    $PAGE_DIR, %edi
clear_pde:
    movl    %ecx, (%edi)                            /* 清空页目录 */
    addl    $4, %edi
    cmpl    $(PAGE_DIR + 0x1000), %edi
    jne     clear_pde
    
    movl    $PAGE_DIR, %edi                         /* 初始化pde */
                                                    /* 将线性地址0~0x400000和0xC0000000~0xC0400000 */
                                                    /* 映射到相同的物理地址0~4m, 防止开分页后异常  */
    movl    $(PAGE_PTE_1 + 3), (%edi)               /* PDE 第1项 4m, 加1表示内存已映射  */
    addl    $0xc00, %edi
    movl    $(PAGE_PTE_1 + 3), (%edi)               /* PDE 第0xc00项 4m, 加1表示内存已映射 */   
    
    movl    $1, %ecx
    movl    $PAGE_PTE_1, %edi
init_pte:
    movl    %ecx, (%edi)                            /* 设置页表1 */
    addl    $4, %edi
    addl    $0x1000, %ecx
    cmpl    $(PAGE_PTE_1 + 0x1000), %edi
    jne     init_pte
    
    movl    $PAGE_DIR, %eax                         /* 开分页 */
    movl    %eax, %cr3
    movl    %cr0, %eax
    or      $0x80000000, %eax
    movl    %eax, %cr0
    
    addl    $PAGE_OFFSET, %esp                       /* 调整esp和返回地址 */
    popl    %eax
    addl    $PAGE_OFFSET, %eax
    pushl   %eax
    ret
    
/*****************************************************************/
/*                                                               */
/*                 重新初始化8259a中断控制                       */
/*                                                               */
/*****************************************************************/
init_8259a:
    movb  $0x11, %al                                /* 重新初始化8259a中断控制器 */
    out   %al, $0x20
    call  _delay
    out   %al, $0xA0
    call  _delay
    movb  $0x20, %al                                /* 设置硬件中断开始处 (0x20) */
    out   %al, $0x21
    call  _delay
    movb  $0x28, %al                                /* 设置硬件中断开始处 (0x28) */
    out   %al, $0xA1
    call  _delay
    movb  $0x04, %al                                /* 设置主片 */
    out   %al, $0x21
    call  _delay
    movb  $0x02, %al                                /* 设置从片 */
    out   %al, $0xA1
    call  _delay
    movb  $0x01, %al                                /* 8086 mode */
    out   %al, $0x21
    call  _delay
    out   %al, $0xA1
    call  _delay                                    /* 屏蔽所有 */
    movb  $0xfb, %al
    out   %al, $0x21
    call  _delay
    movb  $0xff, %al
    out   %al, $0xA1
    ret
_delay:                                             /* 延迟 */
    ret

/*****************************************************************/
/*                                                               */
/*                 初始化并加载IDT                               */
/*                                                               */
/*****************************************************************/
init_idt:
    movl    $ignore_int, %edx
    movl    $0x00100000, %eax
    movw    %dx, %ax
    movw    $0x8e00, %dx
    movl    $_idt, %edi
    movl    $256, %ecx
rp_sidt:
    movl    %eax, (%edi)
    movl    %edx, 4(%edi)
    addl    $8, %edi
    decl    %ecx
    jne     rp_sidt
    lidtl   (_idt_pdesc)
    ret
ignore_int:                                         /* 默认中断处理程序，什么也不做 */
    iret
    
/*****************************************************************/
/*                                                               */
/*                 idt gdt idt描述符 gdt描述符                   */
/*                                                               */
/*****************************************************************/

.data
    .align  2
_idt_pdesc:                                         /* idt伪描述符 */
    .short  256*8-1                                 /* idt contains 256 entries */    
    .long   _idt

    .align  2
_gdt_pdesc:                                         /* gdt伪描述符 */
    .short  256*8-1                                 /* But I think we must large it.    */
    .long   _gdt                                    /* Or only use one ldt and tss      */
                                                    /* in our system.                   */
    .align  8
_idt:
    .fill   256,8,0                                 /* 中断门暂时为空 */
    
    .align  2 
_gdt:                                               /* 四个段 核心代码段 核心数据段 用户代码段 用户数据段 */

    .short  0                                       /* 不使用 */
    .short  0
    .short  0
    .short  0
    
    .short  0xFFFF                                  /* kernel cs 0x08 (1) */
    .short  0x0000                                  /* base: 0, limit: 4G */
    .short  0x9A00                                  /* Type: present,ring0,data/stack,read/write (10010010) */
    .short  0x00CF                                  /* Limit(1):0xF | Flags:4Kb inc,32bit (11001111)    F --->段限高4位 */

    .short  0xFFFF                                  /* kernel ds 0x10 (2) */
    .short  0x0000                                  /* base: 0, limit: 4G */
    .short  0x9200
    .short  0x00CF

    .short  0xFFFF                                  /* user  cs 0x1B  (3)   11    0-在GDT中 11 特权级为 3 */
    .short  0x0000                                  /* base: 0, limit: 4G */
    .short  0xFA00     
    .short  0x00CF

    .short  0xFFFF                                  /* user ds 0x23   (4)    100  0-在GDT中 11 特权级为 3 */
    .short  0x0000                                  /* base: 0, limit: 4G */
    .short  0xF200
    .short  0x00CF

    .fill   251,8,0
