#ifndef _STDARG_H
#define _STDARG_H

#include <include/types.h>
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
    (__builtin_saveregs (),                         \
     AP = ((char *) &(LASTARG) + __va_rounded_size (LASTARG)))
#endif

void va_end(va_list);       /* Defined in gnulib */
#define va_end(AP)

#define va_arg(AP, TYPE)                            \
    (AP += __va_rounded_size (TYPE),                \
     *((TYPE *) (AP - __va_rounded_size (TYPE))))

extern int vsprintf(char *buf, const char *fmt, va_list args);
extern int printk (u8 attr, const char *format, ...);

#define dmesg(format,args...) printk(10,"[%7d] %s:"format, jiffies, __FILE__, ##args);
#define warning(format,args...) printk(14,"[%7d] %s:"format, jiffies, __FILE__, ##args);
#define panic(format,args...) do{ \
        printk(12,"[%7d] %s:"format, jiffies, __FUNCTION__, ##args);\
        switch_con(fcon3);  \
        sti();              \
        while(1){}          \
        }while(0)

#endif /* _STDARG_H */

