#define     KERNEL_CS       0x08
#define     KERNEL_DS       0x10
#define     USER_CS         0x1B
#define     USER_DS         0x23


/* trap入口 */
.globl _divide_error,_debug,_nmi,_int3,_overflow,_bounds,_invalid_op
.globl _double_fault,_coprocessor_segment_overrun
.globl _invalid_TSS,_segment_not_present,_stack_segment
.globl _general_protection,_coprocessor_error,_reserved
.globl _device_not_available,_page_fault,_parallel_interrupt

/* intr入口 */
.globl _timer_interrupt,_kbd_interrupt,_fd_interrupt

/* syscall入口 */
.globl _system_call,_fast_sys_call

/**************************************/
/*          无错误码处理流程          */
/**************************************/
no_error_code:
    xchgl %eax,(%esp)
    pushl %ebx
    pushl %ecx
    pushl %edx
    pushl %edi
    pushl %esi
    pushl %ebp
    push %ds
    push %es
    push %fs
    pushl $0        # "error code"
    lea 44(%esp),%edx
    pushl %edx
    movl $0x10,%edx
    mov %dx,%ds
    mov %dx,%es
    mov %dx,%fs
    mov %dx,%gs
    call *%eax
    addl $8,%esp
    pop %fs
    pop %es
    pop %ds
    popl %ebp
    popl %esi
    popl %edi
    popl %edx
    popl %ecx
    popl %ebx
    popl %eax
    iret

.align  4
_divide_error:
    pushl $_do_divide_error
    jmp no_error_code

.align  4
_debug:
    pushl $_do_int3     # __do_debug
    jmp no_error_code

.align  4
_nmi:
    pushl $_do_nmi
    jmp no_error_code

.align  4
_int3:
    pushl $_do_int3
    jmp no_error_code

.align  4
_overflow:
    pushl $_do_overflow
    jmp no_error_code

.align  4
_bounds:
    pushl $_do_bounds
    jmp no_error_code

.align  4
_invalid_op:
    pushl $_do_invalid_op
    jmp no_error_code

.align  4
_coprocessor_segment_overrun:
    pushl $_do_coprocessor_segment_overrun
    jmp no_error_code

.align  4
_device_not_available:
    pushl $_do_device_not_available
    jmp no_error_code

.align  4
_coprocessor_error:
    pushl $_do_coprocessor_error
    jmp no_error_code

.align  4
_reserved:
    pushl $_do_reserved
    jmp no_error_code


.align  4
_parallel_interrupt:
    pushl %eax
    movb $0x20,%al
    outb %al,$0x20
    popl %eax
    iret


/**************************************/
/*          有错误码处理流程          */
/**************************************/
error_code:
    xchgl %eax,4(%esp)      # error code <-> %eax
    xchgl %ebx,(%esp)       # &function <-> %ebx
    pushl %ecx
    pushl %edx
    pushl %edi
    pushl %esi
    pushl %ebp
    push %ds
    push %es
    push %fs
    pushl %eax          # error code
    lea 44(%esp),%eax       # offset
    pushl %eax
    movl $0x10,%edx
    mov %dx,%ds
    mov %dx,%es
    mov %dx,%fs
    mov %dx,%gs
    call *%ebx
    addl $8,%esp
    pop %fs
    pop %es
    pop %ds
    popl %ebp
    popl %esi
    popl %edi
    popl %edx
    popl %ecx
    popl %ebx
    popl %eax
    iret

.align  4
_double_fault:
    pushl $_do_double_fault
    jmp error_code

.align  4
_invalid_TSS:
    pushl $_do_invalid_TSS
    jmp error_code

.align  4
_segment_not_present:
    pushl $_do_segment_not_present
    jmp error_code

.align  4
_stack_segment:
    pushl $_do_stack_segment
    jmp error_code

.align  4
_general_protection:
    pushl $_do_general_protection
    jmp error_code

.align  4
_page_fault:
    pushl $_do_page_fault
    jmp error_code

/**************************************/
/*          时钟中断                  */
/**************************************/

.align  4
_timer_interrupt:
#    popl (_k_intr_stacktop-20)          /* eip */
#    popl (_k_intr_stacktop-16)          /* cs */
#    popl (_k_intr_stacktop-12)          /* eflags */
#    je   __int_from_kernel
#    popl (_k_intr_stacktop-8)           /* esp 特权级提高时才会有 */
#    popl (_k_intr_stacktop-4)           /* ss  特权级提高时才会有 */
#    subl $0x8, %esp
#
#__int_from_kernel:
#    subl $0xC, %esp
#    movl %esp, (_k_intr_stacktop-24)
#    leal (_k_intr_stacktop-24), %esp    /* 切换esp到内核中断栈 */
    movl %esp, (_k_intr_stacktop-4)
    leal (_k_intr_stacktop-4), %esp     /* 切换esp到内核中断栈 */
    pushl $0                            /* error_code */
    push %ds
    push %es
    push %gs
    push %fs
    pushl %ebp
    pushl %edi
    pushl %esi
    pushl %edx 
    pushl %ecx
    pushl %ebx 
    pushl %eax 

    pushl %esp

    movb $0x20,%al                      # EOI to interrupt controller #1
    outb %al,$0x20

    movl  44(%esp), %eax                # 获取被中断的ds
    andl  $3, %eax                      # cpl
    pushl %eax
    movl $0x10,%edx
    mov   %dx,%ds
    mov   %dx,%es
    mov   %dx,%fs
    mov   %dx,%gs                       # 设置数据段寄存器的值  
    call  _timer_handler                # 调用时钟中断处理代码
    addl  $8,%esp

    popl  %eax
    popl  %ebx 
    popl  %ecx    
    popl  %edx
    popl  %esi
    popl  %edi
    popl  %ebp    
    pop   %fs
    pop   %gs
    pop   %es
    pop   %ds
    addl  $4, %esp
    popl  %esp
    iret

/**************************************/
/*          键盘中断                  */
/**************************************/

.align 4
_kbd_interrupt:
    movl %esp, (_k_intr_stacktop-4)
    leal (_k_intr_stacktop-4), %esp     /* 切换esp到内核中断栈 */
    pushl $0                            /* error_code */
    push %ds
    push %es
    push %gs
    push %fs
    pushl %ebp
    pushl %edi
    pushl %esi
    pushl %edx 
    pushl %ecx
    pushl %ebx 
    pushl %eax 
    
    movb $0x20,%al                      # EOI to interrupt controller #1
    outb %al,$0x20

    movl  $0x10,%edx
    mov   %dx,%ds
    mov   %dx,%es
    mov   %dx,%fs
    mov   %dx,%gs
    call  _kbd_handler

    popl  %eax
    popl  %ebx 
    popl  %ecx    
    popl  %edx
    popl  %esi
    popl  %edi
    popl  %ebp    
    pop   %fs
    pop   %gs
    pop   %es
    pop   %ds
    addl  $4, %esp
    popl  %esp                          /* 切回原始栈 */
    iret

/**************************************/
/*          软驱中断                  */
/**************************************/

.align 4
_fd_interrupt:
    movl %esp, (_k_intr_stacktop-4)
    leal (_k_intr_stacktop-4), %esp     /* 切换esp到内核中断栈 */
    pushl $0                            /* error_code */
    push %ds
    push %es
    push %gs
    push %fs
    pushl %ebp
    pushl %edi
    pushl %esi
    pushl %edx 
    pushl %ecx
    pushl %ebx 
    pushl %eax 
    
    movb $0x20,%al                      # EOI to interrupt controller #1
    outb %al,$0x20

    movl  $0x10,%edx
    mov   %dx,%ds
    mov   %dx,%es
    mov   %dx,%fs
    mov   %dx,%gs
    call  _fd_handler

    popl  %eax 
    popl  %ebx 
    popl  %ecx    
    popl  %edx
    popl  %esi
    popl  %edi
    popl  %ebp    
    pop   %fs
    pop   %gs
    pop   %es
    pop   %ds
    addl  $4, %esp
    popl  %esp                          /* 切回原始栈 */
    iret


/**************************************/
/*          系统调用                  */
/*使用eax作为调用号，ebx、ecx、edx传参*/
/**************************************/

.align 4
_system_call:
    pushl $0                            /* error_code */
    push %ds
    push %es                            # 保存段寄存器
    push %gs
    push %fs
    pushl %ebp
    pushl %edi
    pushl %esi
    pushl %edx 
    pushl %ecx
    pushl %ebx 
    pushl %eax 

    movl  $0x10,%edx
    mov   %dx,%ds
    mov   %dx,%es
    mov   %dx,%fs
    mov   %dx,%gs                       # 设置数据段寄存器的值 
    call    _system_call_entry

    addl  $4, %esp                      /* 不恢复eax eax作为返回值 */
    popl  %ebx 
    popl  %ecx    
    popl  %edx
    popl  %esi
    popl  %edi
    popl  %ebp    
    pop   %fs
    pop   %gs
    pop   %es
    pop   %ds
    addl  $4, %esp
    iret

/**************************************/
/*          快速系统调用              */
/*使用eax作为调用号，ebx、esi、edi传参*/
/**************************************/

.align 4
_fast_sys_call:
    sti
    pushl $0                            /* error_code */
    push %ds
    push %es                            # 保存段寄存器
    push %gs
    push %fs
    pushl %ebp
    pushl %edx 
    pushl %ecx
    pushl %edi
    pushl %esi
    pushl %ebx 
    pushl %eax 

    movl  $0x10,%edx
    mov   %dx,%ds
    mov   %dx,%es
    mov   %dx,%fs
    mov   %dx,%gs                       # 设置数据段寄存器的值 
    call    _system_call_entry

    addl  $4, %esp                      /* 不恢复eax eax作为返回值 */
    popl  %ebx 
    popl  %esi
    popl  %edi
    popl  %ecx    
    popl  %edx
    popl  %ebp    
    pop   %fs
    pop   %gs
    pop   %es
    pop   %ds
    addl  $4, %esp
    sysexit

/******************************************/
/*          为内核保留一个中断栈          */
/******************************************/

.bss
.align 4096
_k_intr_stack:
    .fill 4096,1,0
_k_intr_stacktop:
