#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <stddef.h>
#include <FreeRTOS.h>
#include <bsp.h>
#include <nonstdlib.h>
#include <string.h>
#include <stdlib.h>

#define calloc snx_calloc
#define realloc snx_realloc
#define strtod	snx_strtod
#define isspace snx_isspace
#define HAVE_STDARG_H 1


#ifndef json_min
#define json_min(a,b) ((a) < (b) ? (a) : (b))
#endif

#ifndef json_max
#define json_max(a,b) ((a) > (b) ? (a) : (b))
#endif

#define PRId64 "d"//"I64d"
#define SCNd64 "d"//"I64d"

#define JSON_FILE_BUF_SIZE 4096

void *snx_calloc(size_t nmemb, size_t size);
void *snx_realloc(void *ptr, size_t new_size);
double snx_strtod(const char *nptr, char **endptr);
int snx_isspace(int c);

int json_parse_int64(const char *buf, int64_t *retval);
int json_parse_double(const char *buf, double *retval);

#endif
