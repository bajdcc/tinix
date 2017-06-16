#ifndef _FAT12_H
#define _FAT12_H

#include <include/types.h>


/*
FAT12 软盘结构示意图
    +----------+--------+--------+--------+--------        ......           ------+
    |          |        |        |        |                                       |
    | 引导扇区 |  FAT1  |  FAT2  | 根目录 |    数据区(保存文件的簇从这里开始)     |
    |          |        |        |        |簇2                                    |
    +----------+--------+--------+--------+--------        ......           ------+
 扇区0          1        10       19       33
*/


/*
 *  软盘引导扇区中的 BPB 结构体
 */
typedef struct _fat_bpb
{
    u8      JMP[3];                 // 跳转指令
    u8      OEM[8];                 // 8 字节的 OEM 信息字符串。
    u16     SectorSize;             // 扇区的字节大小，一般为 512。
    u8      SectorsPerClustor;      // 每个簇的扇区数（软盘上的 FAT12 系统一般为 1）。
    u16     ReservedSectors;        // 保留扇区数（就是引导扇区数量，一般为 1）。
    u8      FatCount;               // FAT表的数量（一般为 2）。
    u16     RootEntryCount;         // 根目录的文件条目数量（软盘上一般为 224）。
    u16     TotalSectors16;         // 软盘的扇区总数（1.44M 软盘为 2880）。
    u8      Media;                  // 介质描述，0xF0 表示软盘。
    u16     SectorsPerFat;          // 每个 FAT 表占用的扇区数（1.44M 软盘为 9）。
    u16     SectorsPerTrack;        // 每磁道扇区数（1.44M 软盘为 18）。
    u16     HeadCount;              // 磁头数量（软盘为 2）。
    u32     HiddenSectors;          // 隐藏扇区数。
    u32     TotalSectors32;         // 扇区总数(如果 TotalSectors16 为 0，就使用此变量)。
    u8      DriveNumber;            // 中断的驱动器号。
    u8      ReservedField;          // 保留未用。
    u8      BootSignature;          // 扩展引导标记。
    u32     VolumeID;               // 卷序列号。
    u8      ValumeLable[11];        // 卷标。
    u8      FileSystemType[8];      // 文件系统类型字符串。
}__attribute__((packed)) fat_bpb;

/*
 *  根目录中的文件条目结构体
 */
typedef struct _fat_file_entry
{
    u8  FileName[8];                // 8 字节文件名。
    u8  ExtName[3];                 // 3 字节扩展名。
    u8  Attribute;                  // 文件属性（隐藏、只读等）。
    u8  Reserved[10];               // 保留未用。
    u16 Time;                       // 文件最后访问时间。
    u16 Date;                       // 文件最后访问日期。
    u16 ClusterNumber;              // 文件的第一个簇号。
    u32 FileSize;                   // 文件的字节大小。
}__attribute__((packed)) fat_file_entry;


extern void init_fat12fs();

extern s32 fat12_seek(file *, s32, u32);
extern s32 fat12_read(file *, u8*, u32);
#endif
