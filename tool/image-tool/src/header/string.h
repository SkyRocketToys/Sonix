#ifndef _LINUX_STRING_H_
#define _LINUX_STRING_H_

#include "types.h"
#include "stddef.h"

#ifdef __cplusplus
extern "C" {
#endif

extern char * strcpy(char *,const char *);
extern char * strncpy(char *,const char *, size_t);
extern char * strcat(char *, const char *);
extern char * strncat(char *, const char *, size_t);
extern int    strcmp(const char *,const char *);
extern int    strncmp(const char *,const char *,size_t);
extern char * strchr(const char *,int);
extern char * strrchr(const char *,int);
extern char * strstr(const char *,const char *);
extern size_t strlen(const char *);
extern size_t strnlen(const char *,size_t);
extern char * strdup(const char *);
extern char * strswab(const char *);

extern void * memset(void *,int,size_t);
extern void * memcpy(void *,const void *,size_t);
extern void * memmove(void *,const void *,size_t);
extern void * memscan(void *,int,size_t);
extern int    memcmp(const void *,const void *,size_t);
extern void * memchr(const void *,int,size_t);

#ifdef __cplusplus
}
#endif

#endif /* _LINUX_STRING_H_ */
