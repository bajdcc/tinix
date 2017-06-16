/*文件名: string.h
  说明:   提供一些内存处理函数
  作者:   Linus Torvalds
  日期:   2005/12/20
*/

#ifndef _LINUX_STRING_H_
#define _LINUX_STRING_H_

#include <include/types.h>

#ifndef NULL
#define NULL ((void *) 0)
#endif

/*
 * This string-include defines all string functions as 
 * functions. Use gcc. It also assumes ds=es=data space, this should be
 * normal. Most of the string-functions are rather heavily hand-optimized,
 * see especially strtok,strstr,str[c]spn. They should work, but are not
 * very easy to understand. Everything is done entirely within the register
 * set, making the functions fast and clean. String instructions have been
 * used through-out, making for "slightly" unclear code :-)
 *
 *      Copyright (C) 1991, 1992 Linus Torvalds
 */

extern  u8 *  	strcpy(u8 *dest, u8 *src);
extern  u8 *  	strncpy(u8 *dest, u8 *src, u32 count);
extern  u8 *  	strcat(u8 *dest, u8 *src);
extern  u8 *  	strncat(u8 *dest, u8 *src, u32 count);
extern  s32     strcmp(u8 *cs, u8 *ct);
extern  s32     strncmp(u8 *cs, u8 *ct, u32 count);
extern  u8 *  	strchr(u8 *s, char c);
extern  u8 *  	strrchr(u8 *s, char c);
extern  u32     strspn(u8 *cs, u8 *ct);
extern  u32     strcspn(u8 *cs, u8 *ct);
extern  u8 *  	strpbrk(u8 *cs, u8 *ct);
extern  u8 *  	strstr(u8 *cs, u8 *ct);
extern  u32     strlen(u8 *s);
extern  u8 *  	___strtok;
extern  u8 *  	strtok(u8 *s, u8 *ct);
extern  void *  memcpy(void *dest, void *src, u32 n);
extern  void *  memmove(void *dest, void *src, u32 n);
extern  s32     memcmp(void *cs, void *ct, u32 count);
extern  void *  memchr(void *cs, char c, u32 count);
extern  void *  memset(void *s, char c, u32 count);

#endif

