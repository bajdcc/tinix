#include <include/types.h>
#include <include/system.h>
#include <include/stdarg.h>
#include <include/timer.h>
#include <include/syscall.h>
#include <include/string.h>
#include <mm/memory.h>
#include <dev/floppy.h>
#include <dev/console.h>
#include <fs/fs.h>
#include <fs/fat12.h>

/*
 *  文件系统的读写跳转表
 */
file_operations fat12_ops = {
    (void *)fat12_seek,
    (void *)fat12_read,
    (void *)0
};

/*
 *  文件系统的几个关键指针
 *  目前写死了1.44M fat12
 *  以后有机会再兼容吧
 */
fat_bpb * _BPB;
void * _FAT_BUF;
void * _ROOT_ENTRY;
void * _DATA_ENTRY;

static void fat_verify_floppy();                            /* 确认软盘类型，初始化文件系统的几个关键指针 */
static void fat_load_floppy();                              /* 将软盘内容加载至内存，仅在init中调用 */
static void fat_convert_name(fat_file_entry *, u8 *);       /* 将dos文件名转换为小写可读文件名 */
static u32  fat_next_cluster(u32 );                         /* 根据簇号计算下一簇簇号 */
static u32  fat_cluster_to_addr(u32 );                      /* 根据簇号计算扇区数据指针 */
static void fat_traversal_subdir(u32 , u32 );               /* 递归遍历一个子目录，将找到的文件、目录加入内核文件表中 */
static void fat_traversal_root();                           /* 遍历根目录区 */


s32 fat12_seek(file *fat12dev, s32 offset, u32 mode)
{
    switch(mode)
    {
        case SEEK_SET:
        {
            if (offset >= 0 && offset < fat12dev->f_size)
            {
                fat12dev->f_pos[current] = offset;
                return fat12dev->f_pos[current];
            }
            return -1;
        }
        case SEEK_CUR:
        {
            if (offset+fat12dev->f_pos[current] >= 0 && \
                offset+fat12dev->f_pos[current] < fat12dev->f_size)
            {
                fat12dev->f_pos[current] += offset;
                return fat12dev->f_pos[current];
            }
            return -1;
        }
        case SEEK_END:
        {
            if (offset+fat12dev->f_size >= 0 && offset <= 0)
            {
                fat12dev->f_pos[current] = offset+fat12dev->f_size;
                return fat12dev->f_pos[current];
            }
            return -1;
        }    

    }
    return -1;
}


/*
 *  最烦的一个功能!!!!!!真是烦!!!
 */
s32 fat12_read(file *fat12dev, u8* buff, u32 size)
{
    if (size == 0 || fat12dev->f_pos[current]+size > fat12dev->f_size) 
        return -1;

    u32 cluster = fat12dev->f_base;
    u32 readsize = 0;
    u32 lastsize = size;
    u32 f_pos = fat12dev->f_pos[current];
    do
    {
        if (f_pos >= 512)
        {
            f_pos -= 512;   //当前簇无数据
        }
        else
        {
            if (lastsize < 512-f_pos)   //剩余读取的大小小于当前簇剩余的大小
            {
                memcpy(buff + readsize, (void *)(fat_cluster_to_addr(cluster)+ f_pos), lastsize );
                readsize += lastsize;
                goto LABLE_RET;
            }
            else
            {
                memcpy(buff + readsize, (void *)(fat_cluster_to_addr(cluster)+ f_pos), 512-f_pos);
                readsize += 512-f_pos;
                lastsize -= 512-f_pos;
                f_pos = 0;
            }
        }

        cluster = fat_next_cluster(cluster);
    }while(cluster < 0x0FF8 && readsize < size && lastsize >0);    // 如果是最后一个簇就结束

LABLE_RET:
    fat12dev->f_pos[current] += readsize;
    return readsize;
}

static void fat_verify_floppy()
{
    if (_BPB->SectorSize != 512 || _BPB->TotalSectors16 != 2880)
	{
		dmesg("Floppy BPB: sector size %d, totalsectors %d", _BPB->SectorSize, _BPB->TotalSectors16);
        panic("Floppy disk type is not supported!");
	}

    dmesg("Floppy BPB: sector size %d, totalsectors %d", _BPB->SectorSize, _BPB->TotalSectors16);
    _FAT_BUF = (void *)((u32)_BPB + 512);
    _ROOT_ENTRY = (void *)((u32)_BPB + 0x2600);
    _DATA_ENTRY = (void *)((u32)_BPB + 0x4200);
}

static void fat_load_floppy()
{
    u32 i;
    _BPB = (fat_bpb *)k_get_page();

    for (i = 0; i < 509; i++)   k_get_page();

    dmesg("Load floppy disk data to %08x", _BPB);
    con_seek(&kfile_table[fcon3], 160, SEEK_CUR);

    u32 fd = sys_open("/dev/fd");
    for (i = 0; i < 2880; i++)
    {
        sys_seek(fd, i*512, SEEK_SET);
        sys_read(fd, (u8 *)((u32)_BPB + i*512), 512);

        if((i+1)%80 == 0)
        {
            con_seek(&kfile_table[fcon3], -160, SEEK_CUR);
            dmesg("Loading floppy disk data, %d sectors completed.", i);
        }
    }

    con_seek(&kfile_table[fcon3], -160, SEEK_CUR);
    dmesg("Load floppy disk data completed!                    ");
    sys_close(fd);
}

static void fat_convert_name(fat_file_entry *pitem, u8 *buffer)
{
    u32 i;

    buffer[0] = 0;
    for (i = 0; i < 11; ++i)
    {
        if(pitem->FileName[i] == ' ') pitem->FileName[i] = 0;
        if(pitem->FileName[i] >= 'A' && pitem->FileName[i] <= 'Z') 
            pitem->FileName[i] += 0x20;
    }
    strncat(buffer, pitem->FileName, 8);

    if (pitem->ExtName[0] != 0)
    {
        strcat(buffer, ".");
        strncat(buffer, pitem->ExtName, 3);
    }
    if (pitem->Attribute & 0x10) strcat(buffer, "/");
    return;
}

static u32 fat_next_cluster(u32 cluster)
{
    u16 next_cluster;
    
    /*
     * 根据簇号取下一个簇号所在的字
     */
    next_cluster = *(u16 *)(_FAT_BUF + cluster * 3 / 2);
    
    /*
     * 如果簇号是偶数，字中的低 12 位就是下一个簇号
     * 如果簇号是奇数，字中的高 12 位就是下一个簇号
     */
    if(0 == (cluster & 0x1))
        next_cluster = next_cluster & 0x0FFF;
    else
        next_cluster = next_cluster >> 4;
        
    return next_cluster;
}

static u32 fat_cluster_to_addr(u32 cluster)
{
    return (cluster-2)*512 + (u32)_DATA_ENTRY;
}

static void fat_traversal_subdir(u32 father, u32 firstcluster)
{
    u8 tmpname[16];
    u32 dir;
    u32 cluster = firstcluster;
    u32 clusterbuf = fat_cluster_to_addr(cluster);

    fat_file_entry * pitem = (fat_file_entry *)clusterbuf;

    while (pitem->FileName[0] != 0)
    {
        /*
         *  略过已删除文件、长文件名、目录自身、父目录
         *  及标签
         */
        if (pitem->FileName[0] == 0xE5 ||   \
            pitem->Attribute   == 0x0F ||   \
            pitem->FileName[0] == 0x2E ||   \
            pitem->ClusterNumber== 0)  
        {
            pitem = (fat_file_entry *)((u32)pitem + sizeof(fat_file_entry));
            continue;
        }
        
        fat_convert_name(pitem, tmpname);

        /* 这是一个目录 */
        if (pitem->Attribute & 0x10)
        {
            dir = fs_add_dir(father, (pitem->Date << 16) | pitem->Time, tmpname);

            dmesg("Found a subdir [ %12s ] on [ %12s ]", 
                tmpname, 
                kfile_table[father].f_name);
            /* 遍历 该目录 */
            fat_traversal_subdir(dir, pitem->ClusterNumber);
        }
        else
        {
            fs_add_file(father, 
                (pitem->Date << 16) | pitem->Time,
                pitem->ClusterNumber, 
                pitem->FileSize, 
                &fat12_ops, 
                0, 
                tmpname);
            dmesg("Found a file   [ %12s ] on [ %12s ]", 
                tmpname, 
                kfile_table[father].f_name);
        }

        pitem = (fat_file_entry *)((u32)pitem + sizeof(fat_file_entry));

        if ((u32)pitem >= clusterbuf+512)
        {
            cluster = fat_next_cluster(cluster);
            if (cluster >= 0x0FF8) 
                return;               /* 最后一簇 */

            clusterbuf = fat_cluster_to_addr(cluster);
            pitem = (fat_file_entry *)clusterbuf;
        }
    }

}

static void fat_traversal_root()
{
    fat_file_entry * proot = (fat_file_entry *)_ROOT_ENTRY;
    u8 tmpname[16];
    u32 dir;

    /* 遍历根目录区 */
    while (proot->FileName[0] != 0)
    {
        /*
         *  略过已删除文件、长文件名、目录自身、父目录
         *  及标签
         */
        if (proot->FileName[0] == 0xE5 ||   \
            proot->Attribute   == 0x0F ||   \
            proot->FileName[0] == 0x2E ||   \
            proot->ClusterNumber== 0)  
        {
            proot = (fat_file_entry *)((u32)proot + sizeof(fat_file_entry));
            continue;
        }
        
        fat_convert_name(proot, tmpname);

        /* 这是一个目录 */
        if (proot->Attribute & 0x10)
        {
            dir = fs_add_dir(DIR_ROOT, (proot->Date << 16) | proot->Time, tmpname);

            dmesg("Found a subdir [ %12s ] on [ %12s ]", 
                tmpname, 
                kfile_table[DIR_ROOT].f_name);
            /* 遍历 该目录 */
            fat_traversal_subdir(dir, proot->ClusterNumber);
        }
        else
        {
            fs_add_file(DIR_ROOT, 
                (proot->Date << 16) | proot->Time,
                proot->ClusterNumber, 
                proot->FileSize, 
                &fat12_ops, 
                0, 
                tmpname);
            dmesg("Found a file   [ %12s ] on [ %12s ]", 
                tmpname, 
                kfile_table[DIR_ROOT].f_name);
        }

        proot = (fat_file_entry *)((u32)proot + sizeof(fat_file_entry));

        if ((u32)proot >= (u32)_DATA_ENTRY) break;
    }
}

void init_fat12fs()
{
    #ifndef NO_FLOPPY
    fat_load_floppy();
    fat_verify_floppy();
    #else
    _BPB = (fat_bpb *)0xC0105000;
    _FAT_BUF = (void *)((u32)_BPB + 512);
    _ROOT_ENTRY = (void *)((u32)_BPB + 0x2600);
    _DATA_ENTRY = (void *)((u32)_BPB + 0x4200);
    #endif
    fat_traversal_root();

    dmesg("Init fat12 file system completed!");
}

