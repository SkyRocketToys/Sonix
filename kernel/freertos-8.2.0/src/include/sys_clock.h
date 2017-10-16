#ifndef _SYS_CLOCK_H_
#define _SYS_CLOCK_H_

typedef struct system_date {
	unsigned short year;
	unsigned char month;
	unsigned char day;
	unsigned char hour;
	unsigned char minute;
	unsigned char second;
	unsigned char week;
} system_date_t;

int get_date(system_date_t *pDate);
int break_date(system_date_t *pDate, long long seconds);
void set_date(system_date_t *pDate, int offset);
void set_date_utc(unsigned long utc_seconds);
long long get_sys_seconds(void);
long long get_sys_seconds_utc(void);
long long get_sys_seconds_boot(void);
void time_to_tm(long long totalsecs, int offset, system_date_t *pDate);
long long tm_to_time(system_date_t *pDate, int offset);
void set_tz_offset(long seconds);

#endif
