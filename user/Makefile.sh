############################################################
#                  Makefile for Tinix shell                #
############################################################

ENTRYPOINT      = 0x80000000
CC              = gcc
LD              = ld -melf_i386
CFLAGS          = -O0 -Wall -m32 -I ./include -c -fleading-underscore -nostdinc -fno-builtin \
				-fno-common -W -Wall -D FAST_SYS_CALL -fno-stack-protector -fno-pic
LDFLAGS         = -x -s  -Ttext $(ENTRYPOINT) -e _tinix_main

EXECUTABLE      = sh
OBJ             = objs/sh.o 
LIB_OBJS        = objs/syscall.o objs/string.o objs/vsprintf.o objs/printf.o \
	objs/_doscan.o objs/sscanf.o objs/sprintf.o

$(EXECUTABLE) : $(OBJ) $(LIB_OBJS)
	$(LD) $(LDFLAGS) -o $@ $^

$(OBJ) :app/sh.c
	$(CC) $(CFLAGS) $^
	mv *.o objs/

$(LIB_OBJS) :lib/syscall.c lib/string.c lib/vsprintf.c lib/printf.c \
	lib/_doscan.c lib/sscanf.c lib/sprintf.c
	$(CC) $(CFLAGS) $^
	mv *.o objs/

clean:
	-rm $(OBJ) $(EXECUTABLE)
