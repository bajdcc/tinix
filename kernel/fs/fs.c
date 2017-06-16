#include <include/types.h>
#include <include/string.h>
#include <include/system.h>
#include <fs/sysfs.h>
#include <fs/fs.h>

file kfile_table[MAX_FILE];

static u32 kfile_lastid = 0;

/*
 *  向内核文件表中添加一个目录
 *  如果是个伪文件，ftime置0即可
 */
s32 fs_add_dir(u32 father, u32 ftime, u8 *name)
{
    u32 eflags;
    _local_irq_save(eflags);
    if (kfile_lastid < MAX_FILE)
    {
        kfile_table[kfile_lastid].f_type = F_T_DIR;
        kfile_table[kfile_lastid].f_father = father;
        kfile_table[kfile_lastid].f_time = ftime;
        kfile_table[kfile_lastid].f_base = 0;
        kfile_table[kfile_lastid].f_size = 0;
        kfile_table[kfile_lastid].f_op = 0;
        kfile_table[kfile_lastid].f_req = 0;

        strcpy(kfile_table[kfile_lastid].f_name, name);

        kfile_table[father].f_size++;
        kfile_lastid++;

        _local_irq_restore(eflags);
        return kfile_lastid-1;
    }
    else
    {
        _local_irq_restore(eflags);
        return -1;
    }

}

/*
 *  向内核文件表中添加一个文件
 *  如果是个伪文件，ftime置0即可
 */
s32 fs_add_file(u32 father, u32 ftime, u32 base, u32 size, 
    struct _file_operations *f_op,
    struct _file_req *f_req,
    u8 *name)
{
    u32 i,eflags;
    _local_irq_save(eflags);
    if (kfile_lastid < MAX_FILE)
    {

        kfile_table[kfile_lastid].f_type = F_T_FILE;
        kfile_table[kfile_lastid].f_father = father;
        kfile_table[kfile_lastid].f_time = ftime;
        kfile_table[kfile_lastid].f_base = base;
        kfile_table[kfile_lastid].f_size = size;
        kfile_table[kfile_lastid].f_op = f_op;
        kfile_table[kfile_lastid].f_req = f_req;
        for (i=0;i<MAX_TASK;i++) kfile_table[kfile_lastid].f_pos[i] = 0;
        strcpy(kfile_table[kfile_lastid].f_name, name);

        kfile_table[father].f_size++;
        kfile_lastid++;
        _local_irq_restore(eflags);
        return kfile_lastid-1;
    }
    else
    {
        _local_irq_restore(eflags);
        return -1;
    }
}


s32  fs_match_dir(u32 father, u8 *path)
{
    u32 i;
    for (i = 1; i < kfile_lastid; i++)
    {
        if( kfile_table[i].f_type   == F_T_DIR && \
            kfile_table[i].f_father == father && \
            0 == strncmp(path, kfile_table[i].f_name, strlen(kfile_table[i].f_name)))
        {
            return i;
        }
    }
    return -1;
}


s32  fs_match_file(u32 father, u8 *name)
{
    u32 i;
    for (i = 1; i < kfile_lastid; i++)
    {
        if( kfile_table[i].f_type   == F_T_FILE && \
            kfile_table[i].f_father == father && \
            0 == strcmp(name, kfile_table[i].f_name))
        {
            return i;
        }
    }
    return -1;
}

void init_fs()
{
    kfile_lastid = 0;
    fs_add_dir(0, 0, "*");
    fs_add_dir(0, 0, "/");
    fs_add_dir(1, 0, "dev/");
    fs_add_dir(1, 0, "sys/");
}
