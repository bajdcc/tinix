#include <include/types.h>
#include <include/syscall.h>
#include <include/task.h>
#include <include/string.h>
#include <include/stdarg.h>
#include <include/timer.h>
#include <include/limits.h>
#include <include/system.h>
#include <fs/fs.h>


s32 sys_close(s32 handle)
{
    if(handle < 2 || handle >= MAX_OPEN) return -1;
    if( task_list[current].fid[handle] == -1)
    {
        return -1;
    }
    task_list[current].fid[handle] = -1;
    return 0;
}
