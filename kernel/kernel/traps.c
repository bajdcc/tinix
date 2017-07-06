#include <include/types.h>
#include <include/system.h>
#include <include/traps.h>
#include <include/task.h>
#include <include/stdarg.h>
#include <include/timer.h>
#include <include/io.h>
#include <include/syscall.h>
#include <dev/console.h>

/* 
 * 本文件定义了所有异常入口
 * 系统调用入口、硬件中断入口也是在这里设置的
 *
 * 基本上所有异常均将导致产生异常的任务退出，只有以下两个特例
 *
 * #BP 调试陷阱 
 * 此异常会将产生异常的任务寄存器状态打印至内核消息输出终端
 *
 * #NM 设备不可用故障 
 * 此异常表示当前任务尝试使用浮点指令，但cr0的ts位置位
 * 因此此异常处理中将清除cr0的ts位，同时将该任务的标志位图中
 * 的TASK_FLAG_FPU置位，这将导致该任务在调度切换时需要保存和恢复fpu
 */

void divide_error(void);
void debug(void);
void nmi(void);
void int3(void);
void overflow(void);
void bounds(void);
void invalid_op(void);
void device_not_available(void);
void double_fault(void);
void coprocessor_segment_overrun(void);
void invalid_TSS(void);
void segment_not_present(void);
void stack_segment(void);
void general_protection(void);
void page_fault(void);
void coprocessor_error(void);
void reserved(void);

void parallel_interrupt(void);
void timer_interrupt(void);
void kbd_interrupt(void);
void fd_interrupt(void); 

void system_call(void);


static void do_exception_exit(char * str,long * esp,long error_code,
        long fs,long es,long ds,
        long ebp,long esi,long edi,
        long edx,long ecx,long ebx,long eax)
{
    warning("Task %d error. ", current);
    warning("Info: %s. Code: %08X", str, error_code);
    warning("EAX=%08X  EBX=%08X  ECX=%08X  EDX=%08X",
        eax,ebx,ecx,edx);
    warning("ESI=%08X  EDI=%08X  EBP=%08X  ESP=%08X",
        esi,edi,ebp,(long) esp);
    warning("DS=%04X  ES=%04X   FS=%04X  CS=%04X   EFLAGS: %08X",
        ds&0xffff, es&0xffff, fs&0xffff, esp[1]&0xffff, esp[2]);
    warning("EIP=%08X ", esp[0]);
    
    if ((esp[1]&0xffff) == KERNEL_CS)
    {
        panic("Kernel mode exception, panic!");
    }
    else
    {
        warning("User mode exception, Task %d exit.", current);
        sys_exit();
    }
}

void do_double_fault(long * esp, long error_code,
        long fs,long es,long ds,
        long ebp,long esi,long edi,
        long edx,long ecx,long ebx,long eax)
{
    do_exception_exit("double fault",esp,error_code,
        fs,es,ds,ebp,esi,edi,edx,ecx,ebx,eax);
}

void do_general_protection(long * esp, long error_code,
        long fs,long es,long ds,
        long ebp,long esi,long edi,
        long edx,long ecx,long ebx,long eax)
{
    do_exception_exit("general protection",esp,error_code,
        fs,es,ds,ebp,esi,edi,edx,ecx,ebx,eax);
}

void do_divide_error(long * esp, long error_code,
        long fs,long es,long ds,
        long ebp,long esi,long edi,
        long edx,long ecx,long ebx,long eax)
{
    do_exception_exit("divide error",esp,error_code,
        fs,es,ds,ebp,esi,edi,edx,ecx,ebx,eax);
}

void do_int3(long * esp, long error_code,
        long fs,long es,long ds,
        long ebp,long esi,long edi,
        long edx,long ecx,long ebx,long eax)
{
    warning("registers of task %d:", current);
    warning("EAX=%08X  EBX=%08X  ECX=%08X  EDX=%08X",
        eax,ebx,ecx,edx);
    warning("ESI=%08X  EDI=%08X  EBP=%08X  ESP=%08X",
        esi,edi,ebp,(long) esp);
    warning("DS=%04X  ES=%04X   FS=%04X  CS=%04X   EFLAGS: %08X",
        ds&0xffff, es&0xffff, fs&0xffff, esp[1]&0xffff, esp[2]);
    warning("EIP=%08X ", esp[0]);
}

void do_nmi(long * esp, long error_code,
        long fs,long es,long ds,
        long ebp,long esi,long edi,
        long edx,long ecx,long ebx,long eax)
{
    do_exception_exit("nmi",esp,error_code,
        fs,es,ds,ebp,esi,edi,edx,ecx,ebx,eax);
}

void do_debug(long * esp, long error_code,
        long fs,long es,long ds,
        long ebp,long esi,long edi,
        long edx,long ecx,long ebx,long eax)
{
    do_exception_exit("debug",esp,error_code,
        fs,es,ds,ebp,esi,edi,edx,ecx,ebx,eax);
}

void do_overflow(long * esp, long error_code,
        long fs,long es,long ds,
        long ebp,long esi,long edi,
        long edx,long ecx,long ebx,long eax)
{
    do_exception_exit("overflow",esp,error_code,
        fs,es,ds,ebp,esi,edi,edx,ecx,ebx,eax);
}

void do_bounds(long * esp, long error_code,
        long fs,long es,long ds,
        long ebp,long esi,long edi,
        long edx,long ecx,long ebx,long eax)
{
    do_exception_exit("bounds",esp,error_code,
        fs,es,ds,ebp,esi,edi,edx,ecx,ebx,eax);
}

void do_invalid_op(long * esp, long error_code,
        long fs,long es,long ds,
        long ebp,long esi,long edi,
        long edx,long ecx,long ebx,long eax)
{
    do_exception_exit("invalid operand",esp,error_code,
        fs,es,ds,ebp,esi,edi,edx,ecx,ebx,eax);
}

void do_device_not_available(long * esp)
{
    /*
     * #NM异常表示此任务需要使用fpu
     */
    dmesg("Task %d need FPU, clear ts bit. Eip %08X",
        current, esp[0]);
    clear_cr0_ts();
    task_list[current].flags |= TASK_FLAG_FPU;
}

void do_coprocessor_segment_overrun(long * esp, long error_code,
        long fs,long es,long ds,
        long ebp,long esi,long edi,
        long edx,long ecx,long ebx,long eax)
{
    do_exception_exit("coprocessor segment overrun",esp,error_code,
        fs,es,ds,ebp,esi,edi,edx,ecx,ebx,eax);
}

void do_invalid_TSS(long * esp,long error_code,
        long fs,long es,long ds,
        long ebp,long esi,long edi,
        long edx,long ecx,long ebx,long eax)
{
    do_exception_exit("invalid TSS",esp,error_code,
        fs,es,ds,ebp,esi,edi,edx,ecx,ebx,eax);
}

void do_segment_not_present(long * esp,long error_code,
        long fs,long es,long ds,
        long ebp,long esi,long edi,
        long edx,long ecx,long ebx,long eax)
{
    do_exception_exit("segment not present",esp,error_code,
        fs,es,ds,ebp,esi,edi,edx,ecx,ebx,eax);
}

void do_stack_segment(long * esp,long error_code,
        long fs,long es,long ds,
        long ebp,long esi,long edi,
        long edx,long ecx,long ebx,long eax)
{
    do_exception_exit("stack segment",esp,error_code,
        fs,es,ds,ebp,esi,edi,edx,ecx,ebx,eax);
}

void do_coprocessor_error(long * esp, long error_code,
        long fs,long es,long ds,
        long ebp,long esi,long edi,
        long edx,long ecx,long ebx,long eax)
{
    do_exception_exit("coprocessor error",esp,error_code,
        fs,es,ds,ebp,esi,edi,edx,ecx,ebx,eax);
}

void do_page_fault(long * esp, long error_code,
        long fs,long es,long ds,
        long ebp,long esi,long edi,
        long edx,long ecx,long ebx,long eax)
{
    do_exception_exit("page fault",esp,error_code,
        fs,es,ds,ebp,esi,edi,edx,ecx,ebx,eax);
}

void do_reserved(long * esp, long error_code,
        long fs,long es,long ds,
        long ebp,long esi,long edi,
        long edx,long ecx,long ebx,long eax)
{
    do_exception_exit("reserved error",esp,error_code,
        fs,es,ds,ebp,esi,edi,edx,ecx,ebx,eax);
}


void init_traps()
{
    int i;

    set_trap_gate(0,  &divide_error);
    set_trap_gate(1,  &debug);
    set_trap_gate(2,  &nmi);
    set_system_gate(3, &int3);
    set_system_gate(4, &overflow);
    set_system_gate(5, &bounds);
    set_trap_gate(6,  &invalid_op);
    set_trap_gate(7,  &device_not_available);
    set_trap_gate(8,  &double_fault);                   /* 有错误码 */
    set_trap_gate(9,  &coprocessor_segment_overrun);  
    set_trap_gate(10, &invalid_TSS);                    /* 有错误码 */
    set_trap_gate(11, &segment_not_present);            /* 有错误码 */
    set_trap_gate(12, &stack_segment);                  /* 有错误码 */
    set_trap_gate(13, &general_protection);             /* 有错误码 */
    set_trap_gate(14, &page_fault);                     /* 有错误码 */
    set_trap_gate(15, &reserved);
    set_trap_gate(16, &coprocessor_error);

    for (i=17;i<48;i++)
        set_trap_gate(i, &reserved);
	
    set_intr_gate(0x20, &timer_interrupt);
    set_intr_gate(0x21, &kbd_interrupt);
    set_trap_gate(0x27, &parallel_interrupt);
    set_intr_gate(0x26, &fd_interrupt);

    /*
     *  允许 主8259A 芯片的IRQ0、1、2 、6中断请求。
     *  暂不允许 从8259A 芯片的中断请求。
     */
    outb_p(inb_p(0x21)&0xB8,0x21);
    outb(inb_p(0xA1)&0xff,0xA1);

    dmesg("Init traps completed!");
}

// irq编号    设备名称            用途
// irq0         time            电脑系统计时器
// irq1         keyboard        键盘
// irq2         redirect irq9   与irq9相接，mpu-401 mdi使用该irq
// irq3         com2            串口设备
// irq4         com1            串口设备
// irq5         lpt2            建议声卡使用该irq
// irq6         fdd             软驱传输控制用
// irq7         lpt1            打印机传输控制用
// irq8         cmos alert      即时时钟
// irq9         redirect irq2   与irq2相接；可设定给其他硬件使用
// irq10        reversed        建议网卡使用该irq
// irq11        reversed        与irq10相同，都是保留给pci硬件使用，建议分配给显卡
// irq12        ps/2mouse       接ps/2鼠标，若无，也可设定给其他硬件使用
// irq13        fpu             协处理器用，例如fpu（浮点运算器）
// irq14        primary ide     主硬盘传输控制器（1号）
// irq15        secondary ide   从硬盘传输控制器（2号）
