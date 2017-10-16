#include <string.h>

char *__strchrnul(const char *, int);

char *strchr(const char *s, int c)
{
	char *r = __strchrnul(s, c);
	return *(unsigned char *)r == (unsigned char)c ? r : 0;
}

char * strrchr(const char * s, int c)
{
   const char *p = s + strlen(s);
   do {
   if (*p == (char)c)
	   return (char *)p;
   } while (--p >= s);
   return NULL;
}
