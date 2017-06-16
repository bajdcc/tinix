#include <include/types.h>
#include <include/syscall.h>
#include <include/task.h>
#include <include/string.h>
#include <include/stdarg.h>
#include <include/timer.h>
#include <include/limits.h>
#include <include/system.h>
#include <fs/fs.h>


s32 sys_read(s32 handle, u8* buffer, u32 size)
{
    if(handle < 0 || handle >= MAX_OPEN) 
    	return -1;

    s32 kfid;
    kfid = task_list[current].fid[handle];

    if (kfid < 0) 
    	return -1;


    if (!(kfile_table[kfid].f_op)) 
    	return -1;

    if (!(kfile_table[kfid].f_op->read)) 
    	return -1;
    
    return kfile_table[kfid].f_op->read(&(kfile_table[kfid]), buffer, size);
}

