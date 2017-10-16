#include <ctype.h>
//#include "libc.h"
#undef isdigit

int isdigit(int c)
{
	return (unsigned)c-'0' < 10;
}

int isxdigit(int c)
{
	return isdigit(c) || ((unsigned)c|32)-'a' < 6;
}


//int __isdigit_l(int c, locale_t l)
//{
//	return isdigit(c);
//}

//weak_alias(__isdigit_l, isdigit_l);
