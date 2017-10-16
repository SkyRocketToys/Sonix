#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include <stdlib.h>
//#include <sys/types.h>


//#define isdigit(c) ((unsigned) ((c) - '0') < 10U)

int
sscanf(const char *str, const char *format, ...)
{
	int tmp = 0, i = 0;
	int value;
	int *valp;
	const char *start = str;
	char *a;
	char buf[64] = {'\0'};
	char tok;
	va_list args;

	va_start(args, format);
	for ( ; *format != '\0'; format++) {
		if (*format == '%' && format[1] == 'd') {
			int positive;

			if (*str == '-') {
				positive = 0;
				str++;
			} else
				positive = 1;
			if (!isdigit(*str))
				break;
			value = 0;
			do {
				value = (value * 10) - (*str - '0');
				str++;
			} while (isdigit(*str));
			if (positive)
				value = -value;
			valp = va_arg(args, int *);
			*valp = value;
			format++;
		} else if (*format == '%' && format[1] == 'x') {
			format++;
			//if (!isxdigit(*str))
				//break;
			value = 0;
			while (1) {
				if ((*str >= 0x30) && (*str <= 0x39))
					tmp = *str - 0x30;
				else if ((*str >= 0x41) && (*str <= 0x46))
					tmp = *str - 0x37;
				else if ((*str >= 0x61) && (*str <= 0x66))
					tmp = *str - 0x57;

				value = (value * 16) + tmp;
				str++;

				if (!((*str >= 0x30) && (*str <= 0x39)) && !((*str >= 0x41) && (*str <= 0x46)) && !((*str >= 0x61) && (*str <= 0x66)))
					break;
			}

			valp = va_arg(args, int *);
			*valp = value;
		}else if (*format == '%' && format[1] == 's') {
			tok = format[2];
			do {
				buf[i] = *str;
				str++;
				i++;
			} while (*str != tok);

			a = va_arg(args, char *);
			strncpy ( a, buf, strlen ( buf ) + 1 );
			i = 0;
			memset(buf, 0, sizeof(buf));
			format++;
		} else if (*format == *str) {
			str++;
		} else
			break;
	}
	va_end(args);
	return str - start;
}
