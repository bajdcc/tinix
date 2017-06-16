#ifndef _INCLUDE_SYSCALL_H
#define _INCLUDE_SYSCALL_H

#include <types.h>

/*****系统调用数据结构***/
#define MAX_NAME        16      /* 路径最大长度 */

/* SEEK*/
#define SEEK_SET                0
#define SEEK_CUR                1
#define SEEK_END                2

#define TASK_RUNNING            1   /* 运行 */
#define TASK_SLEEP              2   /* 睡眠 */
#define TASK_IOBLOCK            3   /* IO阻塞 */
#define TASK_WAIT               4   /* 等待 */
#define TASK_STOPED             5   /* 已终止 */

/* 文件类型 */
#define F_T_FILE                0
#define F_T_DIR                 1
#define F_T_DEV                 2

/* 日期结构 */
typedef struct _rtc_date{
    u32 sec;
    u32 min;
    u32 hour;
    u32 dayofweek;
    u32 day;
    u32 month;
    u32 year;
}__attribute__((packed)) rtc_date;

typedef struct _task_stat
{
    /* 任务状态 */
    u32     state;                              /* 进程状态 */
    u32     father;                             /* 父进程id */
    u32     utime;                              /* 用户态运行时间(滴答数) */
    u32     ktime;                              /* 内核态运行时间(滴答数) */
    u32     stime;                              /* 进程开始的时间 */
    u32     usedmm;                             /* 进程使用的内存 */
    u8      conname[MAX_NAME];                  /* 控制台名称 */
    u8      comm[MAX_NAME];                     /* 执行体名称 */
} __attribute__((packed)) task_stat;

typedef struct _file_stat {
    u32 findex;                                 //该文件在父目录中的序号，0为父目录自身
    u32 ftype;                                  //类型
    u32 ftime;                                  //最后修改时间
    u32 fsize;                                  //文件大小
    u8  fname[MAX_NAME];                        //文件名
}__attribute__((packed)) file_stat;


#define     _NR_sys_open        0       //s32 sys_open(u8 *);
#define     _NR_sys_close       1       //s32 sys_close(s32);
#define     _NR_sys_write       2       //s32 sys_write(s32 , u8* , u32 );
#define     _NR_sys_read        3       //s32 sys_read(s32 , u8* , u32 );
#define     _NR_sys_seek        4       //s32 sys_seek(s32 , s32 , u32 );
#define     _NR_sys_wait        5       //s32 sys_wait(u32);
#define     _NR_sys_exit        6       //s32 sys_exit();
#define     _NR_sys_exec        7       //s32 sys_exec(u8 *);
#define     _NR_sys_kill        8       //s32 sys_kill(u32);
#define     _NR_sys_getpid      9       //s32 sys_getpid(u32);
#define     _NR_sys_getticks    10      //s32 sys_getticks();
#define     _NR_sys_pstat       11      //s32 sys_pstat(u32 pid, void *st_pstat, u32 size);
#define     _NR_sys_opendir     12      //s32 sys_opendir(u8 * path);
#define     _NR_sys_readdir     13      //s32 sys_readdir(s32 handle, void *st_fstat, u32 size);
#define     _NR_sys_getdate     14      //s32 sys_getdate(void *, u32);
#define     _NR_sys_reboot      15      //s32 sys_reboot(u32);
#define     _NR_sys_sleep       16      //s32 sys_sleep(u32);

extern int sys_open(char *_filename);
extern int sys_close(unsigned int _handle);
extern int sys_write(int _handle,char * _buffer,int _size);
extern int sys_read(int _handle,char * _buffer,int _size);
extern int sys_seek(int _handle, int _offset,unsigned int _mode);
extern int sys_wait(unsigned int _pid);
extern int sys_exit();
extern int sys_exec(char * _path);
extern int sys_kill(unsigned int _tid);
extern int sys_getpid();
extern int sys_getticks();
extern int sys_pstat(unsigned int pid, struct _task_stat *st_pstat, unsigned int size);
extern int sys_opendir(char * path);
extern int sys_readdir(int handle, struct _file_stat *st_fstat, unsigned int size);
extern int sys_getdate(struct _rtc_date *st_rtcdate, unsigned int size);
extern int sys_reboot(unsigned int mode);
extern int sys_sleep(unsigned int stime);


#endif
