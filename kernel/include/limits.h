/*
 * 一些操作系统限制
 */

#ifndef _LIMITS_H
#define _LIMITS_H

#define MAX_NAME            16      /* 文件名最大长度 */


#define MAX_TIMER           64      /* 定时器和任务队列长度 */
#define MAX_TASK            64
#define MAX_OPEN            16      /* 一个进程打开的最大文件数，即句柄表长度 */
#define MAX_TASK_COUNTER    20      /* 进程拥有的初始时间片 */


#define MAX_FILE            256     /* 操作系统支持的文件数量 */
#define MAX_IOBUFFER        512     /* IO 缓冲区大小 */
#endif

