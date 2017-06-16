/* boot.S - bootstrap the kernel */

#define MULTIBOOT_HEADER_MAGIC          0x1BADB002
#define MULTIBOOT_HEADER_FLAGS          0x00000007  /* The flags for the Multiboot header. */
#define STACK_SIZE                      0x400      /* The size of our stack (1KB). */


.text
.globl  start, _kernels, _kernele ,stack

#ifdef NO_FLOPPY
.globl  _floppy_s, _floppy_e
#endif

start:
    jmp     multiboot_entry

    .align  4
multiboot_header:
    .long   MULTIBOOT_HEADER_MAGIC
    .long   MULTIBOOT_HEADER_FLAGS
    .long   -(MULTIBOOT_HEADER_MAGIC + MULTIBOOT_HEADER_FLAGS)
    .long   0
    .long   0
    .long   0
    .long   0
    .long   0
    .long   1                                       /* Text mode */
    .long   80                                      /* Columns */
    .long   25                                      /* Lines */
    .long   0                                       /* no use */
multiboot_entry:
    movl    $(stack + STACK_SIZE), %esp             /* Initialize the stack pointer. */
    pushl   $0                                      /* Reset EFLAGS. */
    popf
    pushl   %ebx                                    /* Push the pointer to the Multiboot information structure. */
    pushl   %eax                                    /* Push the magic value. */
    call    _loader_main                            /* Now enter the C main function... */
    xorl    %eax, %eax
    jmp     *%eax                                   /* 永远也不会回来 */

.data
stack:
    .fill   STACK_SIZE, 1, 0                        /* Our stack area. */
_kernels:
    .incbin "kernelz"
_kernele:
    .ascii  "kernel end."

#ifdef NO_FLOPPY
_floppy_s:
    .incbin "../../floppy.img"
_floppy_e:
    .ascii  "floppy end."
#endif
