/*文件名: string.c
  说明:   提供一些内存处理函数
  作者:   Linus Torvalds
  日期:   2005/12/20
*/

#include <types.h>
#include <string.h>


/*
 * This string-include defines all string functions as inline
 * functions. Use gcc. It also assumes ds=es=data space, this should be
 * normal. Most of the string-functions are rather heavily hand-optimized,
 * see especially strtok,strstr,str[c]spn. They should work, but are not
 * very easy to understand. Everything is done entirely within the register
 * set, making the functions fast and clean. String instructions have been
 * used through-out, making for "slightly" unclear code :-)
 *
 *      Copyright (C) 1991, 1992 Linus Torvalds
 */

char * strcpy(char *dest, char *src)
{
    __asm__("cld\n"
            "1:\tlodsb\n\t"
            "stosb\n\t"
            "testb %%al,%%al\n\t"
            "jne 1b" 
            : : "S" (src), "D" (dest)
            :"%eax"
            );
    return dest;
}

char * strncpy(char *dest, char *src, u32 count)
{
    __asm__("cld\n"
            "1:\tdecl %2\n\t"
            "js 2f\n\t"
            "lodsb\n\t"
            "stosb\n\t"
            "testb %%al,%%al\n\t"
            "jne 1b\n\t"
            "rep\n\t"
            "stosb\n"
            "2:" 
            ::"S" (src), "D" (dest), "c" (count) 
            :"%eax"
            );
    return dest;
}

char * strcat(char *dest, char *src)
{
    __asm__("cld\n\t"
            "repne\n\t"
            "scasb\n\t"
            "decl %1\n"
            "1:\tlodsb\n\t"
            "stosb\n\t"
            "testb %%al,%%al\n\t"
            "jne 1b" 
            ::"S" (src), "D" (dest), "a" (0), "c" (0xffffffff) 
            );
    return dest;
}

char * strncat(char *dest, char *src, u32 count)
{
    __asm__ __volatile__ ("cld\n\t"
            "repne\n\t"
            "scasb\n\t"
            "decl %1\n\t"
            "movl %4,%3\n"
            "1:\tdecl %3\n\t"
            "js 2f\n\t"
            "lodsb\n\t"
            "stosb\n\t"
            "testb %%al,%%al\n\t"
            "jne 1b\n"
            "2:\txorl %2,%2\n\t"
            "stosb" 
            :
            :"S" (src), "D" (dest), "a" (0), "c" (0xffffffff), "g" (count) 
            );
    return dest;
}

s32 strcmp(char *cs, char *ct)
{
    s32 __res;
    __asm__ __volatile__ ("cld\n"
            "1:\tlodsb\n\t"
            "scasb\n\t"
            "jne 2f\n\t"
            "testb %%al,%%al\n\t"
            "jne 1b\n\t"
            "xorl %%eax,%%eax\n\t"
            "jmp 3f\n"
            "2:\tmovl $1,%%eax\n\t"
            "jl 3f\n\t"
            "negl %%eax\n"
            "3:" 
            :"=a" (__res) 
            : "D" (cs), "S" (ct) 
            );
    return __res;
}

s32 strncmp(char *cs, char *ct, u32 count)
{
    //register s32 __res __asm__("%eax");
    s32 __res;
    __asm__("cld\n"
            "1:\tdecl %3\n\t"
            "js 2f\n\t"
            "lodsb\n\t"
            "scasb\n\t"
            "jne 3f\n\t"
            "testb %%al,%%al\n\t"
            "jne 1b\n"
            "2:\txorl %%eax,%%eax\n\t"
            "jmp 4f\n"
            "3:\tmovl $1,%%eax\n\t"
            "jl 4f\n\t"
            "negl %%eax\n"
            "4:" 
            :"=a" (__res) 
            :"D" (cs), "S" (ct), "c" (count) 
            );
    return __res;
}

char * strchr(char *s, char c)
{
    register char * __res __asm__("%eax");
    __asm__("cld\n\t"
            "movb %%al,%%ah\n"
            "1:\tlodsb\n\t"
            "cmpb %%ah,%%al\n\t"
            "je 2f\n\t"
            "testb %%al,%%al\n\t"
            "jne 1b\n\t"
            "movl $1,%1\n"
            "2:\tmovl %1,%0\n\t"
            "decl %0" 
            :"=a" (__res) 
            :"S" (s), "0" (c) 
            );
    return __res;
}

char * strrchr(char *s, char c)
{
    //register char * __res __asm__("%edx");
    char * __res;
    __asm__("cld\n\t"
            "movb %%al,%%ah\n"
            "1:\tlodsb\n\t"
            "cmpb %%ah,%%al\n\t"
            "jne 2f\n\t"
            "movl %%esi,%0\n\t"
            "decl %0\n"
            "2:\ttestb %%al,%%al\n\t"
            "jne 1b" 
            :"=d" (__res) 
            :"0" (0), "S" (s), "a" (c) 
            );
    return __res;
}

u32 strspn(char *cs, char *ct)
{
    //register char * __res __asm__("%esi");
     char * __res;
    __asm__("cld\n\t"
            "movl %4,%%edi\n\t"
            "repne\n\t"
            "scasb\n\t"
            "notl %%ecx\n\t"
            "decl %%ecx\n\t"
            "movl %%ecx,%%edx\n"
            "1:\tlodsb\n\t"
            "testb %%al,%%al\n\t"
            "je 2f\n\t"
            "movl %4,%%edi\n\t"
            "movl %%edx,%%ecx\n\t"
            "repne\n\t"
            "scasb\n\t"
            "je 1b\n"
            "2:\tdecl %0" 
            :"=S" (__res) 
            :"a" (0),"c" (0xffffffff), "0" (cs), "g" (ct) 
            :"%edx", "%edi"
            );
    return __res - cs;
}

u32 strcspn(char *cs, char *ct)
{
    //register char * __res __asm__("%esi");
    char * __res;
    __asm__("cld\n\t"
            "movl %4, %%edi\n\t"
            "repne\n\t"
            "scasb\n\t"
            "notl %%ecx\n\t"
            "decl %%ecx\n\t"
            "movl %%ecx,%%edx\n"
            "1:\tlodsb\n\t"
            "testb %%al,%%al\n\t"
            "je 2f\n\t"
            "movl %4,%%edi\n\t"
            "movl %%edx,%%ecx\n\t"
            "repne\n\t"
            "scasb\n\t"
            "jne 1b\n"
            "2:\tdecl %0" 
            :"=S" (__res) 
            :"a" (0), "c" (0xffffffff), "0" (cs), "g" (ct) 
            :"%edx", "%edi"
            );
    return __res - cs;
}

char * strpbrk(char *cs, char *ct)
{
    //register char * __res __asm__("%esi");
    char * __res;
    __asm__("cld\n\t"
            "movl %4,%%edi\n\t"
            "repne\n\t"
            "scasb\n\t"
            "notl %%ecx\n\t"
            "decl %%ecx\n\t"
            "movl %%ecx,%%edx\n"
            "1:\tlodsb\n\t"
            "testb %%al,%%al\n\t"
            "je 2f\n\t"
            "movl %4,%%edi\n\t"
            "movl %%edx,%%ecx\n\t"
            "repne\n\t"
            "scasb\n\t"
            "jne 1b\n\t"
            "decl %0\n\t"
            "jmp 3f\n"
            "2:\txorl %0,%0\n"
            "3:" 
            :"=S" (__res) 
            :"a" (0), "c" (0xffffffff), "0" (cs), "g" (ct) 
            :"%edx", "%edi"
            );
    return __res;
}

char * strstr(char *cs, char *ct)
{
    //register char * __res __asm__("%eax");
    char * __res;
    __asm__("cld\n\t"
            "movl %4,%%edi\n\t"
            "repne\n\t"
            "scasb\n\t"
            "notl %%ecx\n\t"
            "decl %%ecx\n\t"   /* NOTE! This also sets Z if searchstring='' */
            "movl %%ecx,%%edx\n"
            "1:\tmovl %4,%%edi\n\t"
            "movl %%esi,%%eax\n\t"
            "movl %%edx,%%ecx\n\t"
            "repe\n\t"
            "cmpsb\n\t"
            "je 2f\n\t"       /* also works for empty string, see above */
                "xchgl %%eax,%%esi\n\t"
            "incl %%esi\n\t"
            "cmpb $0,-1(%%eax)\n\t"
            "jne 1b\n\t"
            "xorl %%eax,%%eax\n\t"
            "2:" 
            :"=a" (__res) 
            :"0" (0), "c" (0xffffffff), "S" (cs), "g" (ct) 
            :"%edx", "%edi"
            );
    return __res;
}

u32 strlen(char *s)
{
    register s32 __res __asm__("%ecx");
    __asm__("cld\n\t"
        "repne\n\t" 
        "scasb\n\t" 
        "notl %0\n\t" 
        "decl %0" 
        : "=c" (__res) 
        : "D" (s), "a" (0), "0" (0xffffffff)
        );

    return __res;
}

char *___strtok;

char * strtok(char *s, char *ct)
{
    register char * __res;
    __asm__("testl %1,%1\n\t"
            "jne 1f\n\t"
            "testl %0,%0\n\t"
            "je 8f\n\t"
            "movl %0,%1\n"
            "1:\txorl %0,%0\n\t"
            "movl $-1,%%ecx\n\t"
            "xorl %%eax,%%eax\n\t"
            "cld\n\t"
            "movl %4,%%edi\n\t"
            "repne\n\t"
            "scasb\n\t"
            "notl %%ecx\n\t"
            "decl %%ecx\n\t"
            "je 7f\n\t"          /* empty delimeter-string */
            "movl %%ecx,%%edx\n"
            "2:\tlodsb\n\t"
            "testb %%al,%%al\n\t"
            "je 7f\n\t"
            "movl %4,%%edi\n\t"
            "movl %%edx,%%ecx\n\t"
            "repne\n\t"
            "scasb\n\t"
            "je 2b\n\t"
            "decl %1\n\t"
            "cmpb $0,(%1)\n\t"
            "je 7f\n\t"
            "movl %1,%0\n"
            "3:\tlodsb\n\t"
            "testb %%al,%%al\n\t"
            "je 5f\n\t"
            "movl %4,%%edi\n\t"
            "movl %%edx,%%ecx\n\t"
            "repne\n\t"
            "scasb\n\t"
            "jne 3b\n\t"
            "decl %1\n\t"
            "cmpb $0,(%1)\n\t"
            "je 5f\n\t"
            "movb $0,(%1)\n\t"
            "incl %1\n\t"
            "jmp 6f\n"
            "5:\txorl %1,%1\n"
            "6:\tcmpb $0,(%0)\n\t"
            "jne 7f\n\t"
            "xorl %0,%0\n"
            "7:\ttestl %0,%0\n\t"
            "jne 8f\n\t"
            "movl %0,%1\n"
            "8:" 
            :"=b" (__res), "=S" (___strtok) 
            :"0" (___strtok), "1" (s), "g" (ct) 
            :"%eax", "%ecx", "%edx", "%edi"
            );
    return __res;
}

void * memcpy(void *dest, void *src, u32 n)
{
    __asm__("cld\n\t"
           "rep\n\t" 
        "movsb" 
        : 
        :"c" (n), "S" (src), "D" (dest) 
        );
    return dest;
}

void * memmove(void *dest, void *src, u32 n)
{
    if (dest < src) {
        __asm__("cld\n\t"
            "rep\n\t" 
            "movsb" 
            : 
            : "c" (n), "S" (src), "D" (dest) 
        );
    } else {
        __asm__("std\n\t"
            "rep\n\t" 
            "movsb\n\t" 
            "cld" 
            : 
            : "c" (n), "S" (src + n - 1), "D" (dest + n - 1)              
            );
    }
    return dest;
}

s32 memcmp(void *cs, void *ct, u32 count)
{
    //register s32 __res __asm__("%eax");
     s32 __res;
    __asm__("cld\n\t"
            "repe\n\t"
            "cmpsb\n\t"
            "je 1f\n\t"
            "movl $1,%%eax\n\t"
            "jl 1f\n\t"
            "negl %%eax\n"
            "1:" 
            :"=a" (__res) 
            :"0" (0), "D" (cs), "S" (ct), "c" (count) 
            );
    return __res;
}

void * memchr(void *cs, char c, u32 count)
{
    //register void * __res __asm__("%edi");
    void * __res;
    if (!count) {
        return NULL;
    }
    __asm__("cld\n\t"
            "repne\n\t"
            "scasb\n\t"
            "je 1f\n\t"
            "movl $1,%0\n"
            "1:\tdecl %0" 
            :"=D" (__res) 
            :"a" (c), "D" (cs), "c" (count) 
            );
    return __res;
}

void * memset(void *s, char c, u32 count)
{
    __asm__ __volatile__ ("cld\n\t"
        "rep\n\t" 
        "stosb" 
        : 
        :"a" (c), "D" (s), "c" (count)
    );
    return s;
}

