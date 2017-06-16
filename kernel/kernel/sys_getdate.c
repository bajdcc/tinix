#include <include/types.h>
#include <include/syscall.h>
#include <include/task.h>
#include <include/string.h>
#include <include/stdarg.h>
#include <include/timer.h>
#include <include/limits.h>
#include <include/system.h>
#include <include/io.h>
#include <mm/memory.h>
#include <dev/console.h>
#include <fs/fs.h>

s32 sys_getdate(void *st_date, u32 size)
{
    if (size != sizeof(rtc_date)) return -1;

    rtc_date *pdate = ((rtc_date *)st_date);

    do {
    pdate->sec          = readcmos(0);
    pdate->min          = readcmos(2);
    pdate->hour         = readcmos(4);
    pdate->dayofweek    = readcmos(6);
    pdate->day          = readcmos(7);
    pdate->month        = readcmos(8);
    pdate->year         = readcmos(9);
    } while (pdate->sec != readcmos(0));

    BCD_TO_BIN(pdate->sec);
    BCD_TO_BIN(pdate->min);
    BCD_TO_BIN(pdate->hour);
    BCD_TO_BIN(pdate->dayofweek);
    BCD_TO_BIN(pdate->day);
    BCD_TO_BIN(pdate->month);
    BCD_TO_BIN(pdate->year);

    pdate->year += 2000;

    return size;
}

