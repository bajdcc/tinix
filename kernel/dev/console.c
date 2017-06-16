
#include <include/types.h>
#include <include/io.h>
#include <include/system.h>
#include <include/stdarg.h>
#include <include/string.h>
#include <include/task.h>
#include <include/timer.h>
#include <dev/console.h>
#include <mm/memory.h>
#include <fs/fs.h>

/*
0,0 |-----------x
    |
    |
    |
    y
*/
#define BOOT_CURPOS             ((unsigned long *)0xC0090004)
static  u32 COLUMNS = 80;                   /* 行字符数 */
//static  u32 LINES   = 25;                   /* 每屏行数 */

static  u16 video_port_reg = 0x3d4;         /* 显卡寄存器选择端口   */
static  u16 video_port_val = 0x3d5;         /* 显卡寄存器写值端口   */

/************* 控制台文件的读写跳转表 *************/
/************* 控制台文件的读缓冲结构 *************/
u32 current_con;
u32 fcon0,fcon1,fcon2,fcon3;
file_operations con_ops = {
    (void *)con_seek, 
    (void *)con_read, 
    (void *)con_write
    };          
file_req con0_req,con1_req,con2_req,con3_req;   
/******************* 一个空行 *****************/
u8 tmpline[160];

/* 更新光标 */
static void set_cursor(file *condev)
{
    u32 eflags;
    if (&kfile_table[current_con] != condev) return;
    _local_irq_save(eflags);
    outb_p(14, video_port_reg);
    outb_p(0xff & ((condev->f_pos[0] + (condev->f_base - 0xC00B8000)) >> 9), video_port_val);
    outb_p(15, video_port_reg);
    outb_p(0xff & ((condev->f_pos[0] + (condev->f_base - 0xC00B8000)) >> 1), video_port_val);
    *(u8 *)(condev->f_base + condev->f_pos[0] + 1) = 0x07;
    _local_irq_restore(eflags);
}

/* 切换控制台 */
void switch_con(u32 con)
{
    if (current_con == con) return;
    u32 eflags;
    _local_irq_save(eflags);
    outb_p(12, video_port_reg);
    outb_p(0xff & ((kfile_table[con].f_base - 0xC00B8000) >> 9), video_port_val);
    outb_p(13, video_port_reg);
    outb_p(0xff & ((kfile_table[con].f_base - 0xC00B8000) >> 1), video_port_val);
    current_con = con;
    set_cursor(&(kfile_table[con]));
    _local_irq_restore(eflags);
}

/* 控制台写操作,buff带颜色 */
s32 con_write(file *condev, u8 *buff, u32 size)
{
    u32 eflags;
    s32 pos = condev->f_pos[0];
    if (size == 0 || size%2 != 0) return -1;
    
    _local_irq_save(eflags);
    while (pos + size >= condev->f_size)
    {
        /* 第一行到倒数第一行向上滚动，最后一行清除 */
        memmove(
            (void *)condev->f_base, 
            (void *)(condev->f_base + COLUMNS*2), 
            condev->f_size - COLUMNS*2);
        
        memset(
            (void *)(condev->f_base + condev->f_size - COLUMNS*2), 
            0, 
            COLUMNS*2);
        
        pos -= COLUMNS*2;
    }
    
    if (pos < 0)
    {
        memcpy(
            (u8 *)(condev->f_base), 
            buff - pos, 
            size + pos);
    }
    else
    {
        memcpy(
            (u8 *)(condev->f_base + pos), 
            buff, 
            size);
        
        pos += size;
        condev->f_pos[0] = pos;
    }
    if (condev == &kfile_table[current_con])
    {
        set_cursor(condev);
    }
    _local_irq_restore(eflags);
    return size;
}

/* 控制台读操作 */
s32 con_read(file *condev, u8* buff, u32 size)
{
    if(size > MAX_IOBUFFER || size == 0) return -1;
    u32 eflags;
    u32 completed;
    
    _local_irq_save(eflags);
    if(condev->f_req->f_req_state == REQ_FREE) 
    {
        set_cursor(condev);
        condev->f_req->f_req_state = REQ_BUSY;
        condev->f_req->f_req_orgsize = size;
        condev->f_req->f_req_completed = 0;
        condev->f_req->f_req_task = current;
        task_list[current].state = TASK_IOBLOCK;
    }
    else
    {
        _local_irq_restore(eflags);
        return -1;
    }
    _local_irq_restore(eflags);

    task_ioblock();                 /* 当前任务将陷入io等待中 */

    completed = condev->f_req->f_req_completed;
    if (completed < size) buff[completed] = 0;
    
    strncpy(buff, condev->f_req->f_req_buffer, completed);
    condev->f_req->f_req_state = REQ_FREE;
    return completed;
}


s32 con_seek(file *condev, s32 offset, u32 mode)
{
    if (offset%2 != 0) return -1;
    switch(mode)
    {
        case SEEK_SET:
        {
            if (offset >= 0 && offset <= condev->f_size)
            {
                condev->f_pos[0] = offset;
                set_cursor(condev);
                return condev->f_pos[0];
            }
            return -1;
        }
        case SEEK_CUR:
        {
            if (offset+condev->f_pos[0] >= 0 && \
                offset+condev->f_pos[0] <= condev->f_size)
            {
                condev->f_pos[0] += offset;
                set_cursor(condev);
                return condev->f_pos[0];
            }
            return -1;
        }
        case SEEK_END:
        {
            if (offset+condev->f_size >= 0 && offset <= 0)
            {
                condev->f_pos[0] = offset+condev->f_size;
                set_cursor(condev);
                return condev->f_pos[0];
            }
            return -1;
        }    

    }
    return -1;
}

/* 此函数用于键盘输入时回显 */
static void con_echo(ch)
{
    u8 tmpchar[2] ={0,7};
    if (kfile_table[current_con].f_pos[0] != kfile_table[current_con].f_size)
    {
        if (ch == 127)
        {
            con_seek(&kfile_table[current_con], -2, SEEK_CUR);
            con_write(&kfile_table[current_con], "\x00\x00", 2);
            con_seek(&kfile_table[current_con], -2, SEEK_CUR);
        }
        else if (ch == 10)
        {
            con_write(&kfile_table[current_con], 
                    tmpline, 
                    COLUMNS*2 - (kfile_table[current_con].f_pos[0] % (COLUMNS*2))
                    );
        }
        else
        {
            tmpchar[0] = ch;
            con_write(&kfile_table[current_con], tmpchar, 2);
        }
    }
}

/* 此函数由键盘中断调用 */
void con_putch(u8 ch)
{
    if (kfile_table[current_con].f_req->f_req_state != REQ_BUSY) return;

    u32 *completed = &(kfile_table[current_con].f_req->f_req_completed);
    u8  *io_buffer = kfile_table[current_con].f_req->f_req_buffer;
    u32 io_task    = kfile_table[current_con].f_req->f_req_task;

    if ((ch >= 0x20 && ch <= 0x7e) || ch == 10)
    {
        io_buffer[*completed] = ch;
        (*completed)++;
        con_echo(ch);
    }
    else if (ch == 127 && *completed > 0)
    {
        (*completed)--;
        con_echo(ch);
    }

    if (ch == 10 || \
        *completed == kfile_table[current_con].f_req->f_req_orgsize)
    {
        task_iowakeup(io_task);
        kfile_table[current_con].f_req->f_req_state = REQ_COMP;
    }
}

/* 初始化控制台 */
void init_con()
{
    con0_req.f_req_state = REQ_FREE;
    con1_req.f_req_state = REQ_FREE;
    con2_req.f_req_state = REQ_FREE;
    con3_req.f_req_state = REQ_FREE;
    fcon0 = fs_add_file(DIR_DEV, 0, 0xC00B8000+0xFA0*3 + 0x30, 0xFA0, &con_ops, &con0_req, "con0");
    fcon1 = fs_add_file(DIR_DEV, 0, 0xC00B8000+0xFA0*2 + 0x20, 0xFA0, &con_ops, &con1_req, "con1");
    fcon2 = fs_add_file(DIR_DEV, 0, 0xC00B8000+0xFA0*1 + 0x10, 0xFA0, &con_ops, &con2_req, "con2");
    fcon3 = fs_add_file(DIR_DEV, 0, 0xC00B8000+0xFA0*0 + 0x00, 0xFA0, &con_ops, &con3_req, "con3");
    kfile_table[fcon3].f_pos[0] = *BOOT_CURPOS;
    switch_con(fcon3);
    memset(tmpline, 0, COLUMNS*2);

    dmesg("Init console driver completed!");
}
