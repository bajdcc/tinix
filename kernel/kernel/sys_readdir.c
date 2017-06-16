#include <include/types.h>
#include <include/syscall.h>
#include <include/task.h>
#include <include/string.h>
#include <include/stdarg.h>
#include <include/timer.h>
#include <include/limits.h>
#include <include/system.h>
#include <mm/memory.h>
#include <fs/fs.h>
#include <dev/console.h>

typedef struct _file_stat {
    u32 findex;                                 //该文件在父目录中的序号，0为父目录自身
    u32 ftype;                                  //类型
    u32 ftime;                                  //最后修改时间
    u32 fsize;                                  //文件大小
    u8  fname[MAX_NAME];                        //文件名
}__attribute__((packed)) file_stat;

s32 sys_readdir(s32 handle, void *st_fstat, u32 size)
{
    s32 kfid,i,j;
    file_stat *pfstat = ((file_stat *)st_fstat);

    if (size != sizeof(file_stat)) return -1;
    if (handle < 0 || handle >= MAX_OPEN) return -1;
    kfid = task_list[current].fid[handle];
    if (kfid < 0) return -1;
    if (kfile_table[kfid].f_type != F_T_DIR) return -1;
    if (pfstat->findex > kfile_table[kfid].f_size) return -1;


    if (pfstat->findex == 0)
    {
        pfstat->ftype = F_T_DIR;
        pfstat->ftime = kfile_table[kfid].f_time;
        pfstat->fsize = kfile_table[kfid].f_size;
        strcpy(pfstat->fname, kfile_table[kfid].f_name);
        return pfstat->findex;
    }
    for (i = 1,j = 0; i < MAX_FILE; i++)
    {
        if (kfile_table[i].f_father == kfid)
        {
            j++;
            if (pfstat->findex == j)
            {
                pfstat->ftype = kfile_table[i].f_type;
                pfstat->ftime = kfile_table[i].f_time;
                pfstat->fsize = kfile_table[i].f_size;
                strcpy(pfstat->fname, kfile_table[i].f_name);
                return pfstat->findex;
            }
        }
    }

    panic("kernel file table error, kfid %d", kfid);
}
