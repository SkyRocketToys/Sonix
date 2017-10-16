#pragma once

#include <stdarg.h>

char *print_vprintf(void *ctx, const char *fmt, va_list ap);
void *print_printf(void *ctx, const char *fmt, ...);
