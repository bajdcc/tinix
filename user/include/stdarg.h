#ifndef _STDARG_H
#define _STDARG_H

#include <types.h>
typedef char *va_list;

/* Amount of space required in an argument list for an arg of type TYPE.
   TYPE may alternatively be an expression whose type is used.  */

#define __va_rounded_size(TYPE)  \
    (((sizeof (TYPE) + sizeof (int) - 1) / sizeof (int)) * sizeof (int))

#ifndef __sparc__
#define va_start(AP, LASTARG)                       \
    (AP = ((char *) &(LASTARG) + __va_rounded_size (LASTARG)))
#else
#define va_start(AP, LASTARG)                       \
    (__builtin_saveregs (),                        \
     AP = ((char *) &(LASTARG) + __va_rounded_size (LASTARG)))
#endif

void va_end(va_list);       /* Defined in gnulib */
#define va_end(AP)

#define va_arg(AP, TYPE)                        \
    (AP += __va_rounded_size (TYPE),                   \
     *((TYPE *) (AP - __va_rounded_size (TYPE))))

#define isdigit(ch)         ((ch>='0'&&ch<='9'))
#define isxdigit(ch)        ((ch>='A'&&ch<='F')||(ch>='A'&&ch<='F')||(ch>='0'&&ch<='9'))
#define isspace(ch)         ((ch=='\t')||(ch=='\r')||(ch=='\n')||(ch=='\v')||(ch=='\f')||(ch==' '))

extern int vsprintf(char *buf, const char *fmt, va_list args);
extern int sprintf(char * buffer, const char *fmt, ...);
extern int printf(const char *fmt, ...);
extern int scanf(const char *format, ...);
extern int sscanf(char * buff, const char *format, ...);
extern int _doscan(register void *x, const char *format, va_list ap);
extern unsigned long int strtoul(register const char *nptr, char **endptr, int base);
extern long int strtol(register const char *nptr, char **endptr, int base);

extern char _char_color;
#endif /* _STDARG_H */

