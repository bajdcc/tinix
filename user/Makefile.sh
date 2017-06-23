############################################################
#                  Makefile for Tinix shell                #
############################################################

ENTRYPOINT      = 0x80000000
DATASEG         = 0x80005000
CC              = gcc
LD              = ld -melf_i386
CFLAGS          = -O0 -Wall -m32 -I ./include -c -fleading-underscore -nostdinc -fno-builtin \
				-fno-common -W -Wall -D FAST_SYS_CALL -fno-stack-protector
LDFLAGS         = -x -s  -Ttext $(ENTRYPOINT) -Tdata $(DATASEG) -e _tinix_main

ELF_KERNEL      = sh
PE_KERNEL       = objs/sh.pe
OBJ             = objs/sh.o 
LIB_OBJS        = objs/syscall.o objs/string.o objs/vsprintf.o objs/printf.o \
	objs/_doscan.o objs/sscanf.o objs/sprintf.o

$(ELF_KERNEL) : $(PE_KERNEL)
	objcopy -I pe-i386 -O elf32-i386 $^ $@

$(PE_KERNEL) : $(OBJ) $(LIB_OBJS)
	$(LD) $(LDFLAGS) -o $@ $^

$(OBJ) :app/sh.c
	$(CC) $(CFLAGS) $^
	mv *.o objs/

$(LIB_OBJS) :lib/syscall.c lib/string.c lib/vsprintf.c lib/printf.c \
	lib/_doscan.c lib/sscanf.c lib/sprintf.c
	$(CC) $(CFLAGS) $^
	mv *.o objs/

clean:
	-rm $(OBJ) $(PE_KERNEL) $(ELF_KERNEL)
