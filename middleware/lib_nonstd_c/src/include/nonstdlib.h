#ifndef __NONSTDLIB_H__
#define __NONSTDLIB_H__

#include <stddef.h>
#include <stdarg.h>

//#ifndef offsetof
//#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)
//#endif
#define CFG_PBSIZE          256

/* system debug print level */
#define SYS_ERR				0
#define SYS_WARN			1
#define SYS_INFO			2
#define SYS_DBG				3


#ifndef container_of
#define container_of(ptr, type, member) ({			\
	const typeof(((type *)0)->member) * __mptr = (ptr);	\
	(type *)((char *)__mptr - offsetof(type, member)); })
#endif

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

#ifndef max
#define max(x, y) ({				\
	typeof(x) _max1 = (x);			\
	typeof(y) _max2 = (y);			\
	(void) (&_max1 == &_max2);		\
	_max1 > _max2 ? _max1 : _max2; })
#endif

#ifndef min
#define min(x, y) ({				\
	typeof(x) _min1 = (x);			\
	typeof(y) _min2 = (y);			\
	(void) (&_min1 == &_min2);		\
	_min1 < _min2 ? _min1 : _min2; })
#endif


struct list_head {
	struct list_head *next, *prev;
};

static inline void INIT_LIST_HEAD(struct list_head *list)
{
	list->next = list;
	list->prev = list;
}

static inline int list_empty(const struct list_head *head)
{
	return head->next == head;
}

static inline void __list_add(struct list_head *new,
			      struct list_head *prev,
			      struct list_head *next)
{
	next->prev = new;
	new->next = next;
	new->prev = prev;
	prev->next = new;
}

static inline void list_add(struct list_head *new, struct list_head *head)
{
	__list_add(new, head, head->next);
}

static inline void __list_del(struct list_head * prev, struct list_head * next)
{
	next->prev = prev;
	prev->next = next;
}

static inline void list_del(struct list_head *entry)
{
	__list_del(entry->prev, entry->next);
//	entry->next = 0x00100100;
//	entry->prev = 0x00200200;
	entry->next = (struct list_head *)0x00100100;
	entry->prev = (struct list_head *)0x00200200;
}


#define list_entry(ptr, type, member) container_of(ptr, type, member)

static inline void list_del_init(struct list_head *entry)
{
	__list_del(entry->prev, entry->next);
	INIT_LIST_HEAD(entry);
}

static inline void list_add_tail(struct list_head *new, struct list_head *head)
{
	__list_add(new, head->prev, head);
}

#define list_for_each_safe(pos, n, head) \
	for (pos = (head)->next, n = pos->next; pos != (head); \
		pos = n, n = pos->next)

unsigned long simple_strtoul(const char *cp,char **endp,unsigned int base);
long simple_strtol(const char *cp,char **endp,unsigned int base);
int sprintf(char * buf, const char *fmt, ...);
void print_char(char ch);
void print_msg(const char *fmt, ...);
void print_msg_queue(const char *fmt, ...);
void vprint_msg_queue(const char *fmt, va_list args);
void set_eraly_stage_flag (int flag);
int set_system_dbg_level(int level);
void print_q(int level, const char *fmt, ...);
void print(int level, const char *fmt, ...);

typedef void (*print_msg_hook_t)(const char *);
print_msg_hook_t print_msg_queue_set_hook(print_msg_hook_t hook);

#endif
