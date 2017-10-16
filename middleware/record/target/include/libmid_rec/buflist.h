/**
 * @file
 * this is buffer list header file, include this file before use
 * @author CJ
 */
 
#ifndef _BUFLIST_H_
#define _BUFLIST_H_

#include <stdlib.h>
#include <stdio.h>
#include <string.h>


#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE 1
#endif

#define USE_FIXED_MEM	1
#define AVBUF_LEN	(1<< 20) 

/**
 * @brief buffer list info
 */
typedef struct _buflist{
	unsigned char *data;		/**< data pointer */
	unsigned int size;			/**< data size */
	int read_mark;				/**< indicates that data is read finish */
	unsigned char iframe;		/**< iframe info of data */
	unsigned char action;		/**< action command info of data */
	unsigned char type;
	uint64_t time;			/**< time info of data */
	struct _buflist *next;		/**< next data buffer pointer */
}buflist_t, *pBuflist_t;

int buflist_init(pBuflist_t pList);
int buflist_uninit(pBuflist_t pList);
int buflist_is_empty(pBuflist_t pList);
int buflist_get_length(pBuflist_t pList);
int buflist_clear(pBuflist_t pList);
int buflist_get_mark(pBuflist_t pList, int position);
int buflist_set_mark(pBuflist_t pList, int position, int mark);
int buflist_reset_mark(pBuflist_t pList);
buflist_t *buflist_read_data(pBuflist_t pList, int position);
int buflist_insert(pBuflist_t pList, int beforeWhich, buflist_t dataInfo);
int buflist_delete(pBuflist_t pList, int position);
int buflist_get_node(pBuflist_t pList, int position, buflist_t **node);

#endif

