#include <include/types.h>
#include <include/syscall.h>
#include <include/task.h>
#include <include/string.h>
#include <include/stdarg.h>
#include <include/timer.h>
#include <include/limits.h>
#include <include/system.h>
#include <fs/fs.h>

s32 sys_open(u8 * path)
{
    s32 dir = DIR_TOP;
    u8 * p  = path;
    s32 tmp = -1;
    u32 i;

    if (*p != '/') return -1;

    /* 解析目录 */
    while (1)
    {
        tmp = fs_match_dir(dir, p);
        if (tmp < 0) break;
        dir = tmp;
        p += strlen(kfile_table[dir].f_name);
    }

    /* 寻找文件 */
    tmp = fs_match_file(dir , p);
    if (tmp < 0) return -1;

    /* 如果找到的文件已经在句柄表中，返回错误 */
    for (i = 0; i < MAX_OPEN; i++)
    {
        if( task_list[current].fid[i] == tmp)
        {
            warning("Reopen file %s. Handle %d. Task %d.", 
                kfile_table[tmp].f_name, i, current);
            return i;
        }
    }

    /* 将找到的文件插入句柄表 */
    for (i = 0; i < MAX_OPEN; i++)
    {
        if( task_list[current].fid[i] == -1)
        {
            task_list[current].fid[i] = tmp;
            kfile_table[tmp].f_pos[current] = 0;
            return i;
        }
    }
    return -1;
}
