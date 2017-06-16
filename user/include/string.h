#ifndef _LINUX_STRING_H_
#define _LINUX_STRING_H_

#include <types.h>

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

extern  char *  strcpy(char *dest, char *src);
extern  char *  strncpy(char *dest, char *src, u32 count);
extern  char *  strcat(char *dest, char *src);
extern  char *  strncat(char *dest, char *src, u32 count);
extern  s32     strcmp(char *cs, char *ct);
extern  s32     strncmp(char *cs, char *ct, u32 count);
extern  char *  strchr(char *s, char c);
extern  char *  strrchr(char *s, char c);
extern  u32     strspn(char *cs, char *ct);
extern  u32     strcspn(char *cs, char *ct);
extern  char *  strpbrk(char *cs, char *ct);
extern  char *  strstr(char *cs, char *ct);
extern  u32     strlen(char *s);
extern  char *  ___strtok;
extern  char *  strtok(char *s, char *ct);
extern  void *  memcpy(void *dest, void *src, u32 n);
extern  void *  memmove(void *dest, void *src, u32 n);
extern  s32     memcmp(void *cs, void *ct, u32 count);
extern  void *  memchr(void *cs, char c, u32 count);
extern  void *  memset(void *s, char c, u32 count);

#endif

