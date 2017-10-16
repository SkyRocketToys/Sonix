#ifndef _PRINTLOG_H_
#define _PRINTLOG_H_

#if 0
typedef struct sys_log_data {
	int counter;
	unsigned int ts_sec;	// Timestamp for second
	unsigned int ts_usec;	// Time stamp for micro second
	unsigned int type;
	unsigned int num;		// Information of error number
	struct sys_log_data *next;
	struct sys_log_data *prev;
	unsigned int reserved;
}sys_log_data_t;
#endif

typedef struct sys_log_data {
	long long ts_sec;	// Timestamp for second
	unsigned int ts_usec;	// Time stamp for micro second
	unsigned char type;
	char reserved1;
	unsigned short num;		// Information of error number
}sys_log_data_t;

typedef struct index_log_data {
	int start_counter;
	int last_counter;
	unsigned int reserved1;
	unsigned int reserved2;
}index_log_data_t;

void Init_PrintLog_Task(void);
void init_systemlog_task(void);
void system_log(char type, unsigned short num);
sys_log_data_t *get_system_log(void);
void show_system_log(void);

#endif
