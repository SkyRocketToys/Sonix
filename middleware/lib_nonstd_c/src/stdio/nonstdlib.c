#include <stddef.h>
#include <stdarg.h>
#include <uart/uart.h>
#include <string.h>
#include <ctype.h>
#include <nonstdlib.h>
#include <sys/time.h>

// Start ctype.h
#define _U  0x01    /* upper */
#define _L  0x02    /* lower */
#define _D  0x04    /* digit */
#define _C  0x08    /* cntrl */
#define _P  0x10    /* punct */
#define _S  0x20    /* white space (space/lf/tab) */
#define _X  0x40    /* hex digit */
#define _SP 0x80    /* hard space (0x20) */

#define SYSTEM_DEFAULT_DBG_LEVEL 	SYS_INFO

static int early_stage = 1;
static int system_dbg_level = SYSTEM_DEFAULT_DBG_LEVEL;

const unsigned char _ctype[] = {
_C,_C,_C,_C,_C,_C,_C,_C,                                                                /* 0-7 */
_C,_C|_S,_C|_S,_C|_S,_C|_S,_C|_S,_C,_C,                                 /* 8-15 */
_C,_C,_C,_C,_C,_C,_C,_C,                                                                /* 16-23 */
_C,_C,_C,_C,_C,_C,_C,_C,                                                                /* 24-31 */
_S|_SP,_P,_P,_P,_P,_P,_P,_P,                                                    /* 32-39 */
_P,_P,_P,_P,_P,_P,_P,_P,                                                                /* 40-47 */
_D,_D,_D,_D,_D,_D,_D,_D,                                                                /* 48-55 */
_D,_D,_P,_P,_P,_P,_P,_P,                                                                /* 56-63 */
_P,_U|_X,_U|_X,_U|_X,_U|_X,_U|_X,_U|_X,_U,                              /* 64-71 */
_U,_U,_U,_U,_U,_U,_U,_U,                                                                /* 72-79 */
_U,_U,_U,_U,_U,_U,_U,_U,                                                                /* 80-87 */
_U,_U,_U,_P,_P,_P,_P,_P,                                                                /* 88-95 */
_P,_L|_X,_L|_X,_L|_X,_L|_X,_L|_X,_L|_X,_L,                              /* 96-103 */
_L,_L,_L,_L,_L,_L,_L,_L,                                                                /* 104-111 */
_L,_L,_L,_L,_L,_L,_L,_L,                                                                /* 112-119 */
_L,_L,_L,_P,_P,_P,_P,_C,                                                                /* 120-127 */
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,                                                /* 128-143 */
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,                                                /* 144-159 */
_S|_SP,_P,_P,_P,_P,_P,_P,_P,_P,_P,_P,_P,_P,_P,_P,_P,    /* 160-175 */
_P,_P,_P,_P,_P,_P,_P,_P,_P,_P,_P,_P,_P,_P,_P,_P,        /* 176-191 */
_U,_U,_U,_U,_U,_U,_U,_U,_U,_U,_U,_U,_U,_U,_U,_U,        /* 192-207 */
_U,_U,_U,_U,_U,_U,_U,_P,_U,_U,_U,_U,_U,_U,_U,_L,        /* 208-223 */
_L,_L,_L,_L,_L,_L,_L,_L,_L,_L,_L,_L,_L,_L,_L,_L,        /* 224-239 */
_L,_L,_L,_L,_L,_L,_L,_P,_L,_L,_L,_L,_L,_L,_L,_L};       /* 240-255 */

#if 0
#define __ismask(x) (_ctype[(int)(unsigned char)(x)])

#define isalnum(c)  ((__ismask(c)&(_U|_L|_D)) != 0)
#define isalpha(c)  ((__ismask(c)&(_U|_L)) != 0)
#define iscntrl(c)  ((__ismask(c)&(_C)) != 0)
#define isdigit(c)  ((__ismask(c)&(_D)) != 0)
#define isgraph(c)  ((__ismask(c)&(_P|_U|_L|_D)) != 0)
#define islower(c)  ((__ismask(c)&(_L)) != 0)
#define isprint(c)  ((__ismask(c)&(_P|_U|_L|_D|_SP)) != 0)
#define ispunct(c)  ((__ismask(c)&(_P)) != 0)
#define isspace(c)  ((__ismask(c)&(_S)) != 0)
#define isupper(c)  ((__ismask(c)&(_U)) != 0)
#define isxdigit(c) ((__ismask(c)&(_D|_X)) != 0)
#endif
#define isascii(c) (((unsigned char)(c))<=0x7f)
#define toascii(c) (((unsigned char)(c))&0x7f)

#if 0
static inline unsigned char __tolower(unsigned char c)
{
	    if (isupper(c))
		            c -= 'A'-'a';
	        return c;
}
#endif

static inline unsigned char __toupper(unsigned char c)
{
	    if (islower(c))
		            c -= 'a'-'A';
	        return c;
}

//#define tolower(c) __tolower(c)
#define toupper(c) __toupper(c)
// End of ctype.h

/* A convenience macro that defines the upper limit of 'size_t' */
#define SIZE_T_MAX     ( (size_t) (-1) )


/*
 * @param x - first value
 * @param y - second value
 *
 * @return smaller of both input values
 */
static inline size_t minval(size_t x, size_t y)
{
    return ( x<=y ? x : y );
}

void raise (int sig_nr)
{
}

//========================================

unsigned long simple_strtoul(const char *cp,char **endp,unsigned int base)
{
    unsigned long result = 0,value;

    if (*cp == '0') {
        cp++;
        if ((*cp == 'x') && isxdigit(cp[1])) {
            base = 16;
            cp++;
        }
        if (!base) {
            base = 8;
        }
    }
    if (!base) {
        base = 10;
    }
    while (isxdigit(*cp) && (value = isdigit(*cp) ? *cp-'0' : (islower(*cp)
        ? toupper(*cp) : *cp)-'A'+10) < base) {
        result = result*base + value;
        cp++;
    }
    if (endp)
        *endp = (char *)cp;
    return result;
}

long simple_strtol(const char *cp,char **endp,unsigned int base)
{
    if(*cp=='-')
        return -simple_strtoul(cp+1,endp,base);
    return simple_strtoul(cp,endp,base);
}

/* we use this so that we can do without the ctype library */
#define is_digit(c) ((c) >= '0' && (c) <= '9')

static int skip_atoi(const char **s)
{
    int i=0;

    while (is_digit(**s))
        i = i*10 + *((*s)++) - '0';
    return i;
}

#define ZEROPAD 1       /* pad with zero */
#define SIGN    2       /* unsigned/signed long */
#define PLUS    4       /* show plus */
#define SPACE   8       /* space if plus */
#define LEFT    16      /* left justified */
#define SPECIAL 32      /* 0x */
#define LARGE   64      /* use 'ABCDEF' instead of 'abcdef' */

#if 0
#define do_div(n,base) ({ \
int __res; \
__res = ((unsigned long) n) % (unsigned) base; \
n = ((unsigned long) n) / (unsigned) base; \
__res; })
#endif

#define do_div(n,base) ({ \
	unsigned int __res; \
	__res = ((unsigned long long) n) % base; \
	n = ((unsigned long long) n) / base; \
	__res; \
})


static char * number(char * str, long long num, int base, int size, int precision
    ,int type)
{
	char c,sign,tmp[66];
	const char *digits="0123456789abcdefghijklmnopqrstuvwxyz";
	int i;

	if (type & LARGE)
		digits = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	if (type & LEFT)
		type &= ~ZEROPAD;
	if (base < 2 || base > 36)
		return 0;

	c = (type & ZEROPAD) ? '0' : ' ';
	sign = 0;
	if (type & SIGN) {
		if (num < 0) {
			sign = '-';
			num = -num;
			size--;
		} else if (type & PLUS) {
			sign = '+';
			size--;
		} else if (type & SPACE) {
			sign = ' ';
			size--;
		}
	}
	if (type & SPECIAL) {
		if (base == 16)
			size -= 2;
		else if (base == 8)
			size--;
	}
	i = 0;
	if (num == 0)
		tmp[i++]='0';
	else while (num != 0)
		tmp[i++] = digits[do_div(num,base)];
	if (i > precision && precision == -1)
		precision = i;

	size -= precision;
	if (!(type&(ZEROPAD+LEFT)))
		while(size-->0)
			*str++ = ' ';
	if (sign)
		*str++ = sign;
	if (type & SPECIAL) {
		if (base==8)
			*str++ = '0';
		else if (base==16) {
			*str++ = '0';
			*str++ = digits[33];
		}
	}
	if (!(type & LEFT))
		while (size-- > 0)
			*str++ = c;
    //while (i < precision--)
    //    *str++ = '0';

	i--;
	for (; precision > 0 ; precision--, i--) {
    	*str++ = tmp[i];
    }

    while (size-- > 0)
        *str++ = ' ';
    return str;
}

int vsprintf(char *buf, const char *fmt, va_list args)
{
	int len;
	unsigned long num;
	int i, base;
	char * str;
	const char *s;
	int flags;      /* flags to number() */
	int field_width;    /* width of output field */
	int precision;      /* min. # of digits for integers; max
                   number of chars for from string */
	int qualifier;      /* 'h', 'l', or 'L' for integer fields */

	for (str=buf ; *fmt ; ++fmt) {
		if (*fmt != '%') {
			*str++ = *fmt;
			continue;
		}

		/* process flags */
		flags = 0;
repeat:
		++fmt;      /* this also skips first '%' */
		switch (*fmt) {
			case '-': flags |= LEFT; goto repeat;
			case '+': flags |= PLUS; goto repeat;
			case ' ': flags |= SPACE; goto repeat;
			case '#': flags |= SPECIAL; goto repeat;
			case '0': flags |= ZEROPAD; goto repeat;
		}

		/* get field width */
		field_width = -1;
		if (is_digit(*fmt))
			field_width = skip_atoi(&fmt);
		else if (*fmt == '*') {
			++fmt;
			/* it's the next argument */
			field_width = va_arg(args, int);
			if (field_width < 0) {
				field_width = -field_width;
				flags |= LEFT;
			}
		}

		/* get the precision */
		precision = -1;
		if (*fmt == '.') {
			++fmt;
			if (is_digit(*fmt))
				precision = skip_atoi(&fmt);
			else if (*fmt == '*') {
				++fmt;
				/* it's the next argument */
				precision = va_arg(args, int);
			}
			if (precision < 0)
				precision = 0;
		}

		/* get the conversion qualifier */
		qualifier = -1;

		if (*fmt == 'h' || *fmt == 'l' || *fmt == 'L') {
			qualifier = *fmt;
			++fmt;
		}

		/* default base */
		base = 10;

		switch (*fmt) {
		case 'c':
			if (!(flags & LEFT))
				while (--field_width > 0)
					*str++ = ' ';
			*str++ = (unsigned char) va_arg(args, int);
			while (--field_width > 0)
				*str++ = ' ';
			continue;

		case 's':
			s = va_arg(args, char *);
			if (!s)
				s = "<NULL>";

			len = strnlen(s, precision);

			if (!(flags & LEFT))
				while (len < field_width--)
					*str++ = ' ';
			for (i = 0; i < len; ++i)
				*str++ = *s++;
			while (len < field_width--)
				*str++ = ' ';
			continue;

		case 'p':
			if (field_width == -1) {
				field_width = 2*sizeof(void *);
				flags |= ZEROPAD;
			}
			str = number(str,
					(unsigned long) va_arg(args, void *), 16,
					field_width, precision, flags);
			continue;

		case 'n':
			if (qualifier == 'l') {
				long * ip = va_arg(args, long *);
				*ip = (str - buf);
			} else {
				int * ip = va_arg(args, int *);
				*ip = (str - buf);
			}
			continue;

		case '%':
			*str++ = '%';
			continue;

		/* integer number formats - set up the flags and "break" */
		case 'o':
			base = 8;
			break;

		case 'X':
			flags |= LARGE;
		case 'x':
			base = 16;
			break;

		case 'd':
		case 'i':
			flags |= SIGN;
		case 'u':
			break;

		default:
			*str++ = '%';
			if (*fmt)
				*str++ = *fmt;
			else
				--fmt;
			continue;
		}

		if (qualifier == 'l')
			num = va_arg(args, unsigned long);
		else if (qualifier == 'h') {
			num = (unsigned short) va_arg(args, int);
			if (flags & SIGN)
				num = (short) num;
		} else if (flags & SIGN)
			num = va_arg(args, int);
		else
			num = va_arg(args, unsigned int);

		str = number(str, num, base, field_width, precision, flags);
	}
	*str = '\0';

	return str-buf;
}

int vsnprintf(char *buf, size_t size, const char *fmt, va_list args)
{
	int len;
	unsigned long long num, num_tmp;
	int i, base;
	char * str;
	const char *s;
	int flags;      /* flags to number() */
	int field_width;    /* width of output field */
	int precision;      /* min. # of digits for integers; max
                   number of chars for from string */
	int qualifier;      /* 'h', 'l', or 'L' for integer fields */
	char *end = buf + size;

	for (str=buf ; *fmt ; ++fmt) {
		if (str >= end)
			break;

		if (*fmt != '%') {
			*str++ = *fmt;
			continue;
		}

		/* process flags */
		flags = 0;
repeat:
		++fmt;      /* this also skips first '%' */
		switch (*fmt) {
			case '-': flags |= LEFT; goto repeat;
			case '+': flags |= PLUS; goto repeat;
			case ' ': flags |= SPACE; goto repeat;
			case '#': flags |= SPECIAL; goto repeat;
			case '0': flags |= ZEROPAD; goto repeat;
		}

		/* get field width */
		field_width = -1;
		if (is_digit(*fmt))
			field_width = skip_atoi(&fmt);
		else if (*fmt == '*') {
			++fmt;
			/* it's the next argument */
			field_width = va_arg(args, int);
			if (field_width < 0) {
				field_width = -field_width;
				flags |= LEFT;
			}
		}

		/* get the precision */
		precision = -1;
		if (*fmt == '.') {
			++fmt;
			if (is_digit(*fmt))
				precision = skip_atoi(&fmt);
			else if (*fmt == '*') {
				++fmt;
				/* it's the next argument */
				precision = va_arg(args, int);
			}
			if (precision < 0)
				precision = 0;
		}

		/* get the conversion qualifier */
		qualifier = -1;
		if (*fmt == 'h' || *fmt == 'l' || *fmt == 'q') {
			qualifier = *fmt;
			if (qualifier == 'l' && *(fmt+1) == 'l') {
				qualifier = 'q';
				++fmt;
			}
			++fmt;
		}

		/* default base */
		base = 10;

		switch (*fmt) {
		case 'c':
			if (!(flags & LEFT))
				while (--field_width > 0)
					*str++ = ' ';
			*str++ = (unsigned char) va_arg(args, int);
			while (--field_width > 0)
				*str++ = ' ';
			continue;

		case 's':
			s = va_arg(args, char *);
			if (!s)
				s = "<NULL>";

			len = strnlen(s, precision);

			if (!(flags & LEFT))
				while (len < field_width--)
					*str++ = ' ';
			for (i = 0; i < len; ++i) {
				*str++ = *s++;
				if (str >= end)
					break;
			}
			while (len < field_width--)
				*str++ = ' ';
			continue;

		case 'p':
			if (field_width == -1) {
				field_width = 2*sizeof(void *);
				flags |= ZEROPAD;
			}
			str = number(str, (unsigned long) va_arg(args, void *), 16,
					field_width, precision, flags);
			continue;

		case 'n':
			if (qualifier == 'l') {
				long * ip = va_arg(args, long *);
				*ip = (str - buf);
			} else {
				int * ip = va_arg(args, int *);
				*ip = (str - buf);
			}
			continue;

		case '%':
			*str++ = '%';
			continue;

			/* integer number formats - set up the flags and "break" */
		case 'o':
			base = 8;
			break;

		case 'X':
			flags |= LARGE;
		case 'x':
			base = 16;
			break;

		case 'd':
		case 'i':
			flags |= SIGN;
		case 'u':
			break;

		default:
			*str++ = '%';
			if (*fmt)
				*str++ = *fmt;
			else
				--fmt;
			continue;
		}

		if (qualifier == 'q')  /* "quad" for 64 bit variables */
			num = va_arg(args, unsigned long long);
		else if (qualifier == 'l')
			num = va_arg(args, unsigned long);
		else if (qualifier == 'h') {
			num = (unsigned short) va_arg(args, int);

		if (flags & SIGN)
			num = (short) num;
		} else if (flags & SIGN)
			num = va_arg(args, int);
		else
			num = va_arg(args, unsigned int);

		num_tmp = num;

		if (field_width == -1 && precision == -1) {
			field_width = 1;
			while ((num_tmp = num_tmp / base) > 0) {
				field_width++;
			}

			if (field_width > (int)(end - str)) {
				field_width = (int)(end - str);
				precision = field_width;
			}
		}
		str = number(str, num, base, field_width, precision, flags);
	}

	*str = '\0';

	return str-buf;
}

/* Forward decl. needed for IP address printing stuff... */
int sprintf(char *buf, const char *fmt, ...)
{
    va_list args;
    int i;

    va_start(args, fmt);
    i = vsnprintf(buf, CFG_PBSIZE, fmt, args);
    va_end(args);
    return i;
}


#if 0
void panic(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    printf(fmt);
    putc('\n');
    va_end(args);
#if 1
    hang();
#else
    udelay (100000);    /* allow messages to go out */
    do_reset (NULL, NULL, 0, 0, NULL);
#endif
}
#endif

void set_eraly_stage_flag (int flag)
{
    early_stage = flag;
}

void print_char(char ch)
{
    uart_print_char(ch);
}

#define CONFIG_PRINT_LOGLEVEL	4

#ifdef CONFIG_PRINT_LOGLEVEL
static int loglevel = CONFIG_PRINT_LOGLEVEL;	// CONFIG_PRINT_LOGLEVEL range from 0 to 8
#endif

static char *parser_loglevel(char *message)
{
	char *new_msg;
	int lv = 0;

	// Log level format <N>, Checking symbol near log level number
	if ((message[0] == '<') && (message[2] == '>')) {
		if (message[1] >= 0x30 && message[1] < 0x39) {
			lv = message[1] - 0x30;
		} else {
			// Log level not in the range.
			new_msg = message;
			//new_msg = NULL;
			goto out;
		}
	} else {
		// Message format not match.
		new_msg = message;
		//new_msg = NULL;
		goto out;
	}

	if (lv <= loglevel) {
		new_msg = message + 3;
		//print_msg("%s", (message + 2));
	} else {
		new_msg = NULL;
	}

out:
	return new_msg;
}

extern void isp_print_msg_hook(const char *);

void print_msg(const char *fmt, ...)
{
	va_list args;
	unsigned int i;
	char printbuffer[CFG_PBSIZE];
	char *pPrintBuf = NULL;

	va_start(args, fmt);

        /*
          this is a workaround for the lack of any other way to detect
          isp initialisation failure
         */
        if (strncmp(fmt, "[isp]", 5) == 0) {
            isp_print_msg_hook(fmt);
        }

	/* For this to work, printbuffer must be larger than
	 * anything we ever want to print.
	 */
	i = vsnprintf(printbuffer, CFG_PBSIZE, fmt, args);
	va_end(args);

	pPrintBuf = parser_loglevel(printbuffer);
	if (pPrintBuf != NULL) {
		uart_print(pPrintBuf);
	}
	/* Print the string */
	//uart_print(printbuffer);
}

static print_msg_hook_t print_hook = NULL;

/*
  allow print function to be intercepted for operation over WiFi
 */
print_msg_hook_t print_msg_queue_set_hook(print_msg_hook_t hook)
{
    print_msg_hook_t old_hook = print_hook;
    print_hook = hook;
    return old_hook;
}

void print_msg_queue(const char *fmt, ...)
{
	va_list args;

	va_start(args, fmt);
        vprint_msg_queue(fmt, args);
	va_end(args);
}

void vprint_msg_queue(const char *fmt, va_list args)
{
	unsigned int i;
	char printbuffer[CFG_PBSIZE];

	/* For this to work, printbuffer must be larger than
	 * anything we ever want to print.
	 */
	i = vsnprintf(printbuffer, CFG_PBSIZE, fmt, args);

	if (early_stage==1) {
		/* Print the string */
		uart_print(printbuffer);
	} else {
		/* Print the string */
		vSendMsgToQueue(printbuffer);
	}
        if (print_hook) {
            print_hook(printbuffer);
        }
}

#if 1
void *__memrchr(const void *m, int c, size_t n)
{
	const unsigned char *s = m;
	c = (unsigned char)c;
	while (n--) if (s[n]==c) return (void *)(s+n);

	return 0;
}
#endif

int set_system_dbg_level(int level)
{
	/* check level */
	if (level < SYS_ERR || level > SYS_DBG)
		return (-1);
	
	system_dbg_level = level;
}

int get_system_dbg_level(void)
{
	return system_dbg_level;
}

//void snx_dbg_print(int level, const char *fmt, ...)
void print(int level, const char *fmt, ...)
{
	va_list args;
	unsigned int i;
	char printbuffer[CFG_PBSIZE];
	
	do {
		if (level <= system_dbg_level)
			print_msg(fmt);
	} while(0);
}

//void snx_dbg_print_q(int level, const char *fmt, ...)
void print_q(int level, const char *fmt, ...)
{
	va_list args;
	unsigned int i;
	char printbuffer[CFG_PBSIZE];
	
	do {
		if (level <= system_dbg_level) {
		/* For this to work, printbuffer must be larger than
		 * anything we ever want to print.
		 */
		 	va_start(args, fmt);
			i = vsnprintf(printbuffer, CFG_PBSIZE, fmt, args);
			va_end(args);

			if (early_stage==1) {
				/* Print the string */
				uart_print(printbuffer);
			} else {
				/* Print the string */
				vSendMsgToQueue(printbuffer);
			}
		}
	} while(0);
}
