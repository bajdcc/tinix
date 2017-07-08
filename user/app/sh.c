#include <tinix.h>

/* 
 * banner
 */
const char * banner = 
    "                       _____ _       _         ___  ____                        "
    "                      |_   _(_)_ __ (_)_  __  / _ \\/ ___|                       "
    "                        | | | | '_ \\| \\ \\/ / | | | \\___ \\                       "
    "                        | | | | | | | |>  <  | |_| |___) |                      "
    "                        |_| |_|_| |_|_/_/\\_\\  \\___/|____/                       "
    "                                                                                ";

/******内建指令表********/
int buildin_help();
int buildin_id();
int buildin_pwd();
int buildin_ls();
int buildin_cd();
int buildin_ps();
int buildin_cat();
int buildin_exit();
int buildin_tick();
int buildin_kill();
int buildin_clear();
int buildin_date();
int buildin_tree();
int buildin_reboot();
int buildin_sleep();

typedef int (*fun_ptr)(void);
typedef struct _st_cmd
{
    char    *cmd;
    char    *help;
    fun_ptr function;
}st_cmd;
st_cmd buildin_table[] = {
    {"help",    "Print help infomation.",       buildin_help},
    {"id",      "Print current task id.",       buildin_id},
    {"pwd",     "Print current work dir.",      buildin_pwd},
    {"ls",      "List directory.",              buildin_ls},
    {"dir",     "An alias of 'ls'.",            buildin_ls},
    {"ll",      "An alias of 'ls'.",            buildin_ls},
    {"cd",      "Change work dir.",             buildin_cd},
    {"ps",      "Print task list.",             buildin_ps},
    {"cat",     "Print file content.",          buildin_cat},
    {"exit",    "Shell exit.",                  buildin_exit},
    {"tick",    "Print kernel tick count.",     buildin_tick},
    {"kill",    "Kill a task by id.",           buildin_kill},
    {"clear",   "Clear current console.",       buildin_clear},
    {"date",    "Print real-time clock time.",  buildin_date},
    {"tree",    "Print directory tree view.",   buildin_tree},
    {"reboot",  "Reboot system.",               buildin_reboot},
    {"sleep",   "Shell sleep.",                 buildin_sleep},
    {0,0,0}
};

/********全局变量********/
#define MAX_CMDLEN  128
#define MAX_DIR     128
#define MAX_CUT     64
char workdir[MAX_DIR]           = {'/',0};                      // 当前工作路径
char prompt[MAX_DIR]            = {0};                          // 提示符
char shell[8]                   = {0};                          // 当前的shell名称
char full_cmdline[MAX_CMDLEN]   = {0};                          // 完整命令行
char* cutdir[MAX_CUT];                                          // 切割路径用的

/*****其他功能函数********/
void    update_prompt();                                        // 重新计算提示符
char*   paser_dir(char *dir);                                   // 解析目录 消除路径回溯
void    print_dir(int depth, char *name);                       // 根据深度递归打印某个目录
void    print_banner();                                         // 显示一个操作系统banner
void    print_prompt();                                         // 将提示符显示到控制台上
int     dispatch_cmd(char *cmd);                                // 分派cmd命令

/* 将一个带有.或..的路径原地转换为绝对路径 */
char* paser_dir(char *dir) 
{
    int     i,j;
    char    tmpdir[MAX_DIR];
    char    *tmp;
    int     mode = dir[strlen(dir)-1]=='/'?0:1;

    for (i = 0; i <MAX_CUT; i++) cutdir[i] = 0;

    // 路径解析
    i = 0;
    tmp = strtok(dir, "/");
    while(tmp)
    {
        cutdir[i++] = tmp;
        tmp = strtok(0, "/");
    }

    for (i = 0; i <MAX_CUT; i++)
    {
        if (!cutdir[i]) break;

        // 处理..和.
        if (!strcmp(cutdir[i], ".."))
        {
            cutdir[i] = 0;

            for (j = i; j >= 0; j--)
            {
                if (cutdir[j] != 0)
                {
                    cutdir[j] = 0;
                    break;
                }
            }
        }
        else if (!strcmp(cutdir[i], ".")) 
        {
            cutdir[i] = 0;
        }
    }

    strcpy(tmpdir, "/");
    for (i = 0; i <MAX_CUT; i++)
    {
        if (cutdir[i] != 0) 
        {
            strcat(tmpdir, cutdir[i]);
            strcat(tmpdir, "/");
        }
    }

    // 如果要解析的是文件目录，把最后一个/消掉
    if (mode)
    {
        tmpdir[strlen(tmpdir)-1] = 0;
    }
    strcpy(dir, tmpdir);
    return dir;
}

int buildin_help()
{
    int i=0;
    while (buildin_table[i].cmd != 0)
    {
        printf("%-20s %s\n", buildin_table[i].cmd, buildin_table[i].help);
        i++;
    }
    return 0;
}

int buildin_id()
{
    printf("pid=%d\n", sys_getpid());
    return 0;
}

int buildin_pwd()
{
    printf("%s\n", workdir);
    return 0;
}

int buildin_ls()
{
    char dir[MAX_CMDLEN] = {0};
    char testdir[MAX_CMDLEN] = {0};
    int fdir,i,count;
    file_stat item;
    char *lsdir;

    sscanf(full_cmdline, "%*s %s", dir);

    
    /* 没有第二参数 ls当前工作路径 / */
    if (strlen(dir) == 0) lsdir = workdir;
    else 
    {
        if (dir[strlen(dir)-1] != '/') strcat(dir, "/");
        if (dir[0] == '/') lsdir = dir;
        else
        {
            strcpy(testdir, workdir);
            strcat(testdir, dir);
            lsdir = testdir;
        }
    }

    fdir = sys_opendir(paser_dir(lsdir));
    if (fdir >= 0)
    {
        item.findex = 0;
        sys_readdir(fdir, &item, sizeof(file_stat));
        count = item.fsize;
        for (i = 1; i <= count; i++)
        {
            item.findex = i;
            sys_readdir(fdir, &item, sizeof(file_stat));
            printf("%04d-%02d-%02d %02d:%02d    %-6s   %-8d  %-16s\n", 
                (item.ftime >> 25) + 1980,
                (item.ftime >> 21) & 0x0000000F,
                (item.ftime >> 16) & 0x0000001F,
                (item.ftime >> 11) & 0x0000001F,
                (item.ftime >> 5)  & 0x0000003F,
                item.ftype==F_T_FILE?"<FILE>":"<DIR>",
                item.fsize,
                item.fname
                );
        }
        sys_close(fdir);
        return 0;
    }
    
    printf("\"%s\" is not a valid dir.\n", dir);
    return 0;
}


int buildin_cd()
{
    char dir[MAX_CMDLEN] = {0};
    char testdir[MAX_CMDLEN] = {0};
    int fdir;
    char* cddir;

    sscanf(full_cmdline, "%*s %s", dir);

    
    /* 没有第二参数 cd / */
    if (strlen(dir) == 0) 
    {
        strcpy(workdir, "/");
        update_prompt();
        return 0;
    }
    else 
    {
        if (!strcmp(workdir, dir)) return 0;
        if (dir[strlen(dir)-1] != '/') strcat(dir, "/");
        if (dir[0] == '/') cddir = dir;
        else
        {
            strcpy(testdir, workdir);
            strcat(testdir, dir);
            cddir = testdir;
        }
    }

    fdir = sys_opendir(paser_dir(cddir));
    if (fdir >= 0)
    {
        sys_close(fdir);
        strcpy(workdir, cddir);
        update_prompt();   
        return 0;
    }
    
    printf("\"%s\" is not a valid dir.\n", cddir);
    return 0;
}

int buildin_ps()
{
    int i;
    task_stat task;
    char stat;

    printf("COMM        PID     PPID    STAT    UTIME   KTIME   STIME   MM(KB)  TTY\n");
    for (i = 0; i < 64; i++)
    {
        if (sys_pstat(i, &task, sizeof(task_stat)) < 0) 
            continue;

        switch (task.state){
            case TASK_RUNNING:  stat = 'R';break;
            case TASK_SLEEP:    stat = 'S';break;
            case TASK_IOBLOCK:  stat = 'B';break;
            case TASK_WAIT:     stat = 'W';break;
            case TASK_STOPED:   stat = 'E';break;
        }

        printf("%-12s%-8d%-8d%-8c%-8d%-8d%-8d%-8d%-8s\n",
            task.comm, i, task.father, stat, task.utime, task.ktime,
            task.stime, task.usedmm/1024, task.conname);
    }
    return 0;
}

int buildin_cat()
{
    char dir[MAX_CMDLEN] = {0};
    char testdir[MAX_CMDLEN] = {0};
    char ch;
    int fdir;
    char *catdir;

    sscanf(full_cmdline, "%*s %s", dir);

    /* 没有第二参数 cd / */
    if (strlen(dir) == 0) 
    {
        printf("Cat what?\n");
        return 0;
    }
    else 
    {
        if (dir[0] == '/') catdir = dir;
        else
        {
            strcpy(testdir, workdir);
            strcat(testdir, dir);
            catdir = testdir;
        }
    }


    fdir = sys_open(paser_dir(catdir));
    if (fdir >= 0)
    {
        while (sys_read(fdir, &ch, 1) == 1)
            printf("%c", ch);
        sys_close(fdir);
        return 0;
    }
    printf("\"%s\" not found.\n", catdir);
    return 0;
}

int buildin_exit()
{
    printf("exit\n");
    sys_exit();
    return 0;
}

int buildin_tick()
{
    printf("%d\n", sys_getticks());
    return 0;
}

int buildin_kill()
{
    char tmp[MAX_CMDLEN] = {0};
    int pid;
    char *endptr;

    sscanf(full_cmdline, "%*s %s", tmp);
    if (strlen(tmp) == 0)
    {
        printf("Kill what?\n");
        return 0;
    }

    pid = strtol(tmp, &endptr, 10);
    if (endptr == tmp)
    {
        printf("Bad task id \"%s\".\n", tmp);
        return 0;
    }

    if (sys_kill(pid) < 0)
    {
        printf("Kill task %d failed!\n", pid);
        return 0;
    }
    return 0;
}

int buildin_clear()
{
    char tmp[1000];
    memset(tmp, 0, 1000);
    sys_seek(1, 0, SEEK_SET);
    sys_write(1, tmp, 1000);
    sys_write(1, tmp, 1000);
    sys_write(1, tmp, 1000);
    sys_write(1, tmp, 1000);
    sys_seek(1, 0, SEEK_SET);
    return 0;
}

int buildin_date()
{
    rtc_date tmp;
    char *dayofweek;
    char *mon;

    sys_getdate(&tmp, sizeof(rtc_date));
    switch(tmp.dayofweek){
        case 1: dayofweek = "Mon";break;
        case 2: dayofweek = "Tue";break;
        case 3: dayofweek = "Wed";break;
        case 4: dayofweek = "Thu";break;
        case 5: dayofweek = "Fri";break;
        case 6: dayofweek = "Sat";break;
        case 7: dayofweek = "Sun";break;
        default:    dayofweek = "Error";
    }

    switch(tmp.month){
        case 1:     mon = "Jan";break;
        case 2:     mon = "Feb";break;
        case 3:     mon = "Mar";break;
        case 4:     mon = "Apr";break;
        case 5:     mon = "May";break;
        case 6:     mon = "Jun";break;
        case 7:     mon = "Jul";break;
        case 8:     mon = "Aug";break;
        case 9:     mon = "Sep";break;
        case 10:    mon = "Oct";break;
        case 11:    mon = "Nov";break;
        case 12:    mon = "Dec";break;
        default:    mon = "Error";
    }

    printf("%s, %d %s %d %02d:%02d:%02d +0800\n",
        dayofweek, tmp.day, mon, tmp.year,
        tmp.hour, tmp.min, tmp.sec);
    return 0;
}

void print_dir(int depth, char *name)
{
    int i,j;
    int fdir,count;
    file_stat item;
    char tmpdir[MAX_DIR];

    fdir = sys_opendir(name);
    item.findex = 0;
    sys_readdir(fdir, &item, sizeof(file_stat));
    count = item.fsize;

    for (i = 1; i<= count; i++)
    {
        item.findex = i;
        sys_readdir(fdir, &item, sizeof(file_stat));
        for (j = 0; j < depth*4; j++) printf(" ");

        if (item.ftype == F_T_DIR) 
            printf("\x1f---");
        else
            printf("\x10---");
        printf("%s\n", item.fname);
        if (item.ftype == F_T_DIR)
        {
            strcpy(tmpdir, name);
            strcat(tmpdir, (char *)item.fname);
            print_dir(depth+1, tmpdir);
        }
            
    }
    sys_close(fdir);
}

int buildin_tree()
{
    char dir[MAX_CMDLEN] = {0};
    char testdir[MAX_CMDLEN] = {0};
    int fdir;
    char *lsdir;

    sscanf(full_cmdline, "%*s %s", dir);

    
    /* 没有第二参数 ls当前工作路径 / */
    if (strlen(dir) == 0) lsdir = workdir;
    else 
    {
        if (dir[strlen(dir)-1] != '/') strcat(dir, "/");
        if (dir[0] == '/') lsdir = dir;
        else
        {
            strcpy(testdir, workdir);
            strcat(testdir, dir);
            lsdir = testdir;
        }
    }


    fdir = sys_opendir(paser_dir(lsdir));
    if (fdir >= 0)
    {
        printf("%s\n", lsdir);
        sys_close(fdir);
        print_dir(0, lsdir);
        return 0;
    }
    
    printf("\"%s\" is not a valid dir.\n", lsdir);
    return 0;
}

int buildin_reboot()
{
    return sys_reboot(0);
}

int buildin_sleep()
{
    char tmp[MAX_CMDLEN] = {0};
    int stime;
    char *endptr;

    sscanf(full_cmdline, "%*s %s", tmp);
    if (strlen(tmp) == 0)
    {
        return sys_sleep(1);
    }

    stime = strtol(tmp, &endptr, 10);
    if (endptr == tmp)
    {
        printf("Bad time \"%s\".\n", tmp);
        return 0;
    }

    return sys_sleep(stime);
}

void update_prompt()
{
    char tmp[MAX_DIR*2];
    unsigned i;
    strcpy(prompt, "\x5b\x0c");                     // '[' , H R 0 0

    memset(tmp, 0, MAX_DIR*2);
    sprintf(shell, "Tinix-%d ", sys_getpid());
    for (i = 0; i < strlen(shell); i++)
    {
        tmp[i*2]    = shell[i];
        tmp[i*2+1]  = 10;
    }

    strcat(prompt, tmp);                            // "sh-x", H 0 G 0
    memset(tmp, 0, MAX_DIR*2);

    for (i = 0; i < strlen(workdir); i++)
    {
        tmp[i*2]    = workdir[i];
        tmp[i*2+1]  = 14;
    }
    strcat(prompt, tmp);                            // "workdir", H R G 0
    strcat(prompt, "\x5d\x0c");                     // ']' , H R 0 0
    strcat(prompt, "\x23\x0f\x20\x07");             // '#'
}

void print_banner()
{
    unsigned i;
    unsigned char randcolor;
    rtc_date t;
    char tmp[strlen((char*)banner)*2];
    
    sys_getdate(&t, sizeof(rtc_date));
    
    randcolor = t.sec + sys_getticks();
    randcolor = randcolor%8 + 8;
    
    for (i = 0; i < strlen((char*)banner); i++) 
    {
        tmp[i*2]    = banner[i];
        tmp[i*2+1]  = randcolor;
    }
    sys_write(1, tmp, strlen((char*)banner)*2);
}

void print_prompt()
{
    sys_write(1, prompt, strlen(prompt));
}

int dispatch_cmd(char *cmd)
{
    int i=0;
    char tmp[MAX_CMDLEN*2] = {0};
    char *execmd;

    /* 作为内建命令执行 */
    while (buildin_table[i].cmd != 0)
    {
        if (!strcmp(buildin_table[i].cmd, cmd))
            return buildin_table[i].function();
        i++;
    }

    /* 作为绝对路径执行 */
    if (cmd[0] == '/')
    {
        execmd = cmd;
    }
    else
    {
        strcpy(tmp, workdir);       /* 作为相对路径执行 */
        strcat(tmp, cmd);
        execmd = tmp;
    }

    
    i = sys_exec(paser_dir(execmd));
    if (i > 0) 
    {
        sys_wait(i);
        return 0;
    }

    printf("Unknow command \"%s\". Try \"help\".\n", execmd);
    return -1;
}


void tinix_main()
{
    int readlen;
    char tmp[MAX_CMDLEN];

    print_banner();
    
    update_prompt();

    while(1)
    {
        print_prompt();

        readlen = sys_read(1, full_cmdline, MAX_CMDLEN);
        if (readlen <= 1)
        {
            continue;
        }

        if (sscanf(full_cmdline, "%s", tmp) > 0)
        {
            dispatch_cmd(tmp);
            continue;
        }
    }
}
