/**/

#ifndef _CONSOLE_H
#define _CONSOLE_H
/*
#define dprint(format,args...) \
    printf("[%s-%s-L%d] "format, __FILE__, __FUNCTION__, __LINE__, ##args);
*/
#define COLUMNS                 80
#define LINES                   25
#define ATTR                    7
#define ATTR_OK                 10
#define ATTR_ERROR              12
#define VIDEO                   0xB8000

extern void dprint (int attr, const char *format, ...);
extern void cls (void);

extern int xpos;
extern int ypos;
#endif
/*
[blink][R][G][B]  [highlight][R][G][B]
   |    |  |  |      |      |  |  |
[  7  ][6][5][4]  [    3    ][2][1][0]
*/
