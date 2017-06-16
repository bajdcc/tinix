#include <include/types.h>
#include <include/syscall.h>
#include <include/task.h>
#include <include/string.h>
#include <include/stdarg.h>
#include <include/timer.h>
#include <include/limits.h>
#include <include/system.h>
#include <fs/fs.h>

s32 sys_opendir(u8 * path)
{
    s32 dir = DIR_TOP;
    u8 * p  = path;
    u32 i;
    s32 tmp;

    if (*p != '/' || p[strlen(p)-1] != '/') return -1;

    /* 解析目录 */
    while (1)
    {
        tmp = fs_match_dir(dir, p);
        if (tmp < 0) break;
        dir = tmp;
        p += strlen(kfile_table[dir].f_name);
    }

    /* 路径没有完 */
    if (*p != 0) return -1;

    /* 如果找到的目录已经在句柄表中，返回错误 */
    for (i = 0; i < MAX_OPEN; i++)
    {
        if( task_list[current].fid[i] == dir)
        {
            warning("Reopen dir %s. Handle %d. Task %d.", 
                kfile_table[dir].f_name, i, current);
            return i;
        }
    }

    /* 将找到的文件插入句柄表 */
    for (i = 0; i < MAX_OPEN; i++)
    {
        if( task_list[current].fid[i] == -1)
        {
            task_list[current].fid[i] = dir;
            kfile_table[dir].f_pos[current] = 0;
            return i;
        }
    }
    return -1;
}

