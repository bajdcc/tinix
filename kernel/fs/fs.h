#ifndef _FS_H
#define _FS_H

#include <include/types.h>
#include <include/limits.h>

/* 文件类型 */
#define F_T_FILE                0
#define F_T_DIR                 1
#define F_T_DEV                 2

/* 文件设备请求结构状态 */
#define REQ_FREE                0
#define REQ_BUSY                1
#define REQ_COMP                2

/* 基本目录 */
#define DIR_TOP                 0
#define DIR_ROOT                1
#define DIR_DEV                 2
#define DIR_SYS                 3

/* SEEK*/
#define SEEK_SET                0
#define SEEK_CUR                1
#define SEEK_END                2

//文件操作跳转表结构
typedef struct _file_operations {
    s32 (*seek) (void *, s32, u32);             //定位文件当前的读写位置
    s32 (*read) (void *, u8 *, u32);            //读文件
    s32 (*write) (void *, u8 *, u32);           //写文件
}__attribute__((packed)) file_operations;

typedef struct _file {
    u32 f_type;
    u32 f_father;                               //父目录索引
    u32 f_time;                                 //最后修改时间
    u32 f_base;                                 //文件基址，不同的文件含义不同
    s32 f_size;                                 //文件大小
    s32 f_pos[MAX_TASK];                        //读写位置,每个任务有自己的读写位置
    struct _file_operations *f_op;              //操作文件跳转指针 不能空！！！     
    struct _file_req *f_req;                    //操作请求结构，可以空
    u8  f_name[MAX_NAME];
}__attribute__((packed)) file;

typedef struct _file_req
{
    u8  f_req_state;                            //设备状态
    u32 f_req_orgsize;                          //原始请求大小
    u32 f_req_completed;                        //已完成请求大小
    u32 f_req_task;                             //发起请求的任务id
    u8  f_req_buffer[MAX_IOBUFFER];
}__attribute__((packed)) file_req;



extern void init_fs();
extern s32  fs_add_dir(u32 father, u32 ftime, u8 *name);
extern s32  fs_add_file(
            u32 father, 
            u32 ftime,
            u32 base, 
            u32 size, 
            struct _file_operations *f_op,
            struct _file_req *f_req_read,
            u8 *name);

extern s32  fs_match_dir(u32 father, u8 *path);
extern s32  fs_match_file(u32 father, u8 *name);

extern file kfile_table[MAX_FILE];
#endif
