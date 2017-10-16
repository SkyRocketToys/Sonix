/*-------------------------------------------*/
/* Integer type definitions for FatFs module */
/*-------------------------------------------*/

#ifndef _FF_INTEGER
#define _FF_INTEGER

#ifdef _WIN32	/* FatFs development platform */

#include <windows.h>
#include <tchar.h>

#else			/* Embedded platform */

/* This type MUST be 8 bit */
typedef unsigned char	BYTE;

/* These types MUST be 16 bit */
//typedef short			SHORT;
typedef unsigned short	WORD;
typedef unsigned short	WCHAR;

/* These types MUST be 16 bit or 32 bit */
//typedef int				INT;
//typedef unsigned int	UINT;

/* These types MUST be 32 bit */
//typedef long			LONG;
typedef unsigned long	DWORD;


#ifndef _BASE_TYPE_INTEGER_
#define _BASE_TYPE_INTEGER_	
	typedef signed int INT;	
	typedef unsigned int UINT; 
	typedef signed long LONG;  
	typedef unsigned long ULONG;
	typedef signed short SHORT;  
	typedef unsigned short USHORT;
#endif



#endif

#endif
