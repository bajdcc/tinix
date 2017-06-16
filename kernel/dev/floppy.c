#include <include/types.h>
#include <include/dma.h>
#include <include/io.h>
#include <include/system.h>
#include <include/stdarg.h>
#include <include/string.h>
#include <include/timer.h>
#include <dev/floppy.h>
#include <dev/console.h>
#include <fs/fs.h>

static u8 floppy_buffer[512];                       /* 软盘高速缓冲区地址指针 */

#define MAX_REPLIES 7                                                             
static u8 floppy_reply_buffer[MAX_REPLIES];         /* 软驱回应缓冲区 */
#define ST0 (floppy_reply_buffer[0])                /* 软驱回应0号字节 */
#define ST1 (floppy_reply_buffer[1])                /* 软驱回应1号字节 */
#define ST2 (floppy_reply_buffer[2])                /* 软驱回应2号字节 */
#define ST3 (floppy_reply_buffer[3])                /* 软驱回应3号字节 */


static char *floppy_inc_name;                       /* 软驱型号名 */
static char *floppy_type;
static u32  floppy_motor=0;                         /* 软驱马达状态字节 */

/*************软驱设备句柄及读写跳转表********************/
u32 ffd;
file_req fd_req;
file_operations fd_ops = {
    (void *)fd_seek, 
    (void *)fd_read, 
    (void *)0
    };
/**********************功能函数***************************/
static void floppy_result(void);                    /* 获得软驱响应状态  */
static u32  floppy_sendbyte(u32);                   /* 向软驱控制寄存器发送一个控制字节  */
static u32  floppy_getbyte(void);                   /* 从软驱数据寄存器得到一个数据字节  */
static u32  floppy_get_info(void);                  /* 得到软驱信息  */
static void floppy_motorOn(void);                   /* 打开软驱马达  */
static void floppy_motorOff(void);                  /* 关闭软驱马达  */
static void floppy_setmode(void);                   /* 软驱模式设置  */
static void block_to_hts(u32, u32*, u32*, u32*);    /* 逻辑块转为磁盘头、磁道号和扇区号  */
static void floppy_setupDMA(void);                  /* 设置软驱DMA通道  */
static void floppy_read_cmd(u32 blk);               /* 从软盘上读取指定的逻辑块到缓冲区  */


void floppy_result(void)
{
    u8 stat, i,count;
    i=0;
    for(count=0; count<0xFF; count++)
    {
        stat = inb( FD_STATUS ) & (STATUS_READY|STATUS_DIR|STATUS_BUSY); //读取状态寄存器
        if (stat == STATUS_READY)
            return;
        if (stat == (STATUS_READY|STATUS_DIR|STATUS_BUSY))
        {
            if(i>7) break;
            floppy_reply_buffer[i++]=inb_p(FD_DATA);
        }
    }

    panic("Get floppy status times out !\n");
}


void fd_handler(void)
{
    if (fd_req.f_req_state == REQ_BUSY)
    {
        fd_req.f_req_state = REQ_COMP;
        task_iowakeup(fd_req.f_req_task);
    }
}


u32 floppy_sendbyte( u32 value )
{
    u8 stat, i;

    for ( i = 0; i < 128; i++ ) {
        stat = inb( FD_STATUS ) & (STATUS_READY|STATUS_DIR);    //读取状态寄存器
        if  ( stat  == STATUS_READY )
        {
            outb( value ,FD_DATA);                              //将参数写入数据寄存器
            return 1;
        }
        io_delay();                                             // 作一些延迟
    }
    return 0;
}


u32 floppy_getbyte(void)
{
    u8 stat, i;

    for ( i = 0; i < 128; i++ ) {
        stat = inb( FD_STATUS ) & (STATUS_READY|STATUS_DIR|STATUS_BUSY); //读取状态寄存器
        if (stat == STATUS_READY)
            return -1;
        if ( stat  == 0xD0 )
            return inb(FD_DATA);
        io_delay();
    }
    return 0;
}


u32 floppy_get_info(void)
{
    u32 i;
    u8 CmType, FdType;

    floppy_sendbyte(0x10);
    i = floppy_getbyte(); 

    switch (i)
    {
        case 0x80:   floppy_inc_name = "NEC765A controller"; break;
        case 0x90:   floppy_inc_name = "NEC765B controller"; break;
        default:     floppy_inc_name = "Enhanced controller"; break;
    }

    CmType = readcmos(0x10);        //read floppy type from cmos
    FdType   = (CmType>>4) & 0x07;

    if ( FdType == 0 )
        panic("Floppy driver not found!");

    switch( FdType )
    {
    case 0x02: // 1.2MB
        floppy_type = "1.2MB";
    break;

    case 0x04: // 1.44MB       标准软盘
        floppy_type = "1.44MB";
        break;

    case 0x05: // 2.88MB
        floppy_type = "2.88MB";
        break;
    }
    return 1;
}


void floppy_motorOn( void )
{
    u32 eflags;
    if (!floppy_motor)
    {
        _local_irq_save(eflags);
        outb(28,FD_DOR);
        floppy_motor = 1;
        _local_irq_restore(eflags);
    }
    return;
}


void floppy_motorOff( void )
{
    u32 eflags;
    if (floppy_motor)
    {
        _local_irq_save(eflags);
        outb(12,FD_DOR);
        floppy_motor = 0;
        _local_irq_restore(eflags);

    }
    return;
}


void floppy_setmode(void)
{   
    floppy_sendbyte (FD_SPECIFY);
    floppy_sendbyte (0xcf);
    floppy_sendbyte (0x06);
    outb (0,FD_DCR);
}


void block_to_hts(u32 block, u32 *head, u32 *track, u32 *sector )
{
    *head = ( block % ( 18 * 2 ) ) /18;
    *track =  block / ( 18 * 2 );
    *sector = block % 18 + 1;
}


void floppy_setupDMA(void)
{  
    u32 eflags;
    _local_irq_save(eflags);
    DisableDma(2);
    ClearDmaFF(2);
    SetDmaMode(2,DMA_MODE_READ);
    SetDmaAddr(2,(unsigned long)floppy_buffer);
    SetDmaCount(2,512);
    EnableDma(2);
    _local_irq_restore(eflags);
}


void floppy_read_cmd(u32 blk)
{
    u32 head;
    u32 track;
    u32 sector;

    block_to_hts(blk,&head,&track,&sector);

    floppy_motorOn();
    io_delay();

    floppy_setupDMA();
    io_delay();

    floppy_setmode();
    io_delay();
    floppy_sendbyte (FD_READ);                  //send read command
    floppy_sendbyte (head*4 + 0);
    floppy_sendbyte (track);                    /*  Cylinder  */
    floppy_sendbyte (head);                     /*  Head  */
    floppy_sendbyte (sector);                   /*  Sector  */
    floppy_sendbyte (2);                        /*  0=128, 1=256, 2=512, 3=1024, ...  */
    floppy_sendbyte (18);
    //floppy_sendbyte (sector+secs-1);          /*  Last sector in track:here are  sectors count */
    floppy_sendbyte (0x1B);
    floppy_sendbyte (0xff);                      
    return;
}


s32 fd_read(file *fddev, u8 *buff, u32 size)
{
    if(size > MAX_IOBUFFER || size == 0 || fddev->f_pos[current] >= fddev->f_size) return -1;
    u32 eflags;

    _local_irq_save(eflags);
    if(fddev->f_req->f_req_state == REQ_FREE) 
    {
        fddev->f_req->f_req_state = REQ_BUSY;
        fddev->f_req->f_req_orgsize = size;
        fddev->f_req->f_req_task = current;
        task_list[current].state = TASK_IOBLOCK;
    }
    else
    {
        _local_irq_restore(eflags);
        return -1;
    }
    floppy_read_cmd(fddev->f_pos[current] / 512);
    _local_irq_restore(eflags);

    
    task_ioblock();                         /* 当前任务将陷入io等待中 */

    /* --------结果检查-------- */
    _local_irq_save(eflags);
    floppy_result();
    io_delay();

    if(ST1 != 0 || ST2 != 0)
    {
        panic("ST0 %d ST1 %d ST2 %d\n",ST0,ST1,ST2);
    }
    /* --------结果检查完成-------- */
    
    memcpy(buff, floppy_buffer, size);
    fddev->f_req->f_req_state = REQ_FREE;
    fddev->f_pos[current] += 512;

    floppy_motorOff();
    io_delay();

    _local_irq_restore(eflags);
    
    return size;
}


s32 fd_seek(file *fddev, s32 offset, u32 mode)
{
    switch(mode)
    {
        case SEEK_SET:
        {
            if (offset >= 0 && offset < fddev->f_size)
            {
                fddev->f_pos[current] = offset - offset%512;
                return fddev->f_pos[current];
            }
            return -1;
        }
        case SEEK_CUR:
        {
            if (offset+fddev->f_pos[current] >= 0 && \
                offset+fddev->f_pos[current] < fddev->f_size)
            {
                fddev->f_pos[current] += offset;
                fddev->f_pos[current] -= fddev->f_pos[current]%512;
                return fddev->f_pos[current];
            }
            return -1;
        }
        case SEEK_END:
        {
            if (offset+fddev->f_size >= 0 && offset <= 0)
            {
                fddev->f_pos[current] = offset+fddev->f_size;
                fddev->f_pos[current] -= fddev->f_pos[current]%512;
                return fddev->f_pos[current];
            }
            return -1;
        }    

    }
    return -1;
}


void init_fd(void)
{
    //floppy_get_info();
    //dmesg("Floppy Inc : %s  Floppy Type : %s",floppy_inc_name,floppy_type);

    fd_req.f_req_state = REQ_FREE;
    ffd = fs_add_file(DIR_DEV, 0, 0, 0x168000, &fd_ops, &fd_req, "fd");
    dmesg("Init floppy driver completed!");
}

