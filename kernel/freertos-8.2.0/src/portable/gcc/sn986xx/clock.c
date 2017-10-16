#include <stdint.h>
#include <bsp.h>
#include <FreeRTOS.h>
#include <task.h>
#include <sys_clock.h>

#define PLL800_DIV_OFFSET	15
#define PLL300_DIV_OFFSET	23

pll_clock_t pll_clk;
system_bus_freq_t bus_freq;

// change in sys_seconds from set_date()
static long long sys_seconds_offset;
static long long sys_seconds_tz_offset;

void init_freq_table()
{
	uint32_t glb_ctrl = 0;
	volatile global_setting_t *glb_rate = (global_setting_t *)(GLOBAL_RATE);

	glb_ctrl = inl(GLOBAL_CTRL);

	pll_clk.pll800_div = ((glb_ctrl >> PLL800_DIV_OFFSET) & 0x7f);
	pll_clk.pll300_div = ((glb_ctrl >> PLL300_DIV_OFFSET) & 0x3f);

	pll_clk.pll800_clk = 12 * pll_clk.pll800_div * 1000000;
	pll_clk.pll300_clk = 12 * pll_clk.pll300_div * 1000000;

	if (glb_rate->cpu_rate == 0) {
		bus_freq.fcpu = pll_clk.pll800_clk / 16;
	} else {
		bus_freq.fcpu = pll_clk.pll800_clk / glb_rate->cpu_rate;
	}

	if (glb_rate->ddr_rate == 0 || glb_rate->ddr_rate == 1) {
		bus_freq.fddr = pll_clk.pll800_clk / 16;
	} else {
		bus_freq.fddr = pll_clk.pll800_clk / glb_rate->ddr_rate;
	}

	if (glb_rate->ahb_rate == 0) {
		bus_freq.fahb = bus_freq.fcpu / 16;
	} else {
		bus_freq.fahb = bus_freq.fcpu / glb_rate->ahb_rate;
	}

	if (glb_rate->apb_rate == 0) {
		bus_freq.fapb = bus_freq.fahb / 8;
	} else {
		bus_freq.fapb = bus_freq.fahb / glb_rate->apb_rate;
	}

	if (glb_rate->ahb2_rate < 8) {
		bus_freq.fahb2 = bus_freq.fcpu / 32;
	} else {
		bus_freq.fahb2 = bus_freq.fcpu / glb_rate->ahb2_rate;
	}

	if (glb_rate->isp_rate < 4) {
		bus_freq.fisp = pll_clk.pll800_clk / 16;
	} else {
		bus_freq.fisp = pll_clk.pll800_clk / glb_rate->isp_rate;
	}

}

system_bus_freq_t *clk_get()
{
	return &bus_freq;
}

extern void _cpu_delay(unsigned int);
void cpu_udelay(int us)
{
	unsigned int cnt = 0;

	cnt = us * 100;

	_cpu_delay(cnt);
}

long long sys_seconds = 0;

unsigned int year_sec[4] = {31536000, 31536000, 31622400, 31536000};
unsigned int mon_sec[12] = {2678400,	// Jan
							2419200,	// Feb	28 days
							2678400,	// Mar
							2592000,	// Apr
							2678400,	// May
							2592000,	// Jun
							2678400,	// Jul
							2678400,	// Aug
							2592000,	// Sep
							2678400,	// Oct
							2592000,	// Nov
							2678400};	// Dec

int get_date(system_date_t *pDate)
{
    return break_date(pDate, get_sys_seconds());
}

int break_date(system_date_t *pDate, long long sys_seconds_tmp)
{
	unsigned short tmp_y = 0, tmp_m = 0, tmp_d = 0;
	int i = 0;
	int w = 0, m = 0, y = 0;

	if (sys_seconds_tmp >= 31536000) {
		goto calc_year;
	} else if (sys_seconds_tmp >= 2419200) {
		goto calc_month;
	} else if (sys_seconds_tmp >= 86400) {
		goto calc_day;
	} else {
		goto calc_time;
	}

calc_year:
	/* Calculate year */
	for (i = 0 ; i < 250 ; i++) {
		if (sys_seconds_tmp - year_sec[tmp_y % 4] >= 0) {
			sys_seconds_tmp = sys_seconds_tmp - year_sec[tmp_y % 4];
			tmp_y++;
		} else {
			break;
		}
	}

calc_month:
	/* Update Feb */
	if ((tmp_y + 1970) % 4 == 0) {
		mon_sec[1] = 2505600;	// Leap year
	} else {
		mon_sec[1] = 2419200;
	}

	/* Calculate Month */
	for (i = 0 ; i < 12 ; i++) {
		if (sys_seconds_tmp - mon_sec[i] >= 0) {
			sys_seconds_tmp -= mon_sec[i];
			tmp_m++;
		} else {
			break;
		}
	}

calc_day:
	/* calculate day */
	tmp_d = sys_seconds_tmp / 86400;
	sys_seconds_tmp = sys_seconds_tmp % 86400;

calc_time:
	pDate->hour = sys_seconds_tmp / 3600;
	pDate->minute = (sys_seconds_tmp % 3600) / 60;
	pDate->second =  (sys_seconds_tmp % 3600 % 60);

	pDate->year = 1970 + tmp_y;
	pDate->month = 1 + tmp_m;
	pDate->day = 1 + tmp_d;

	/*
	 * Formula W= (d+2*m+3*(m+1)/5+y+y/4-y/100+y/400) mod 7
	 */
	if (pDate->month == 1 || pDate->month == 2) {
		m = pDate->month + 12;
		y = (pDate->year - 1) % 100;
	} else {
		m = pDate->month;
		y = pDate->year % 100;
	}

	w = (pDate->day + 2*m + 3*(m+1)/5 + y + y/4 - y/100 + y/400);
	pDate->week = (w % 7) + 1;

	return 0;
}

void set_date_utc(unsigned long utc_seconds)
{
    long long sys_seconds_new = utc_seconds;
    long err = sys_seconds_new - sys_seconds;
    if (labs(err) > 5) {
        // avoid changing too often
        //console_printf("Adjusting time by %ld\n", err);
        sys_seconds_offset += err;
        sys_seconds = sys_seconds_new;
    }
}

void set_date(system_date_t *pDate, int offset)
{
	long long sys_seconds_tmp = 0;
	unsigned short tmp_y = 0;
	int i = 0;

	/* Year */
	for (tmp_y = 1970 ; tmp_y < pDate->year ; tmp_y++) {
		sys_seconds_tmp += year_sec[i % 4];
		i++;
	}

	/* Month */
	/* Update Feb */
	if (pDate->year % 4 == 0) {
		mon_sec[1] = 2505600;	// Leap year
	} else {
		mon_sec[1] = 2419200;
	}

	for (i = 0 ; i < (pDate->month - 1) ; i++) {
		sys_seconds_tmp += mon_sec[i];
	}

	/* Day */
	sys_seconds_tmp += (pDate->day - 1) * 86400;

	/* Hour */
	sys_seconds_tmp += pDate->hour * 3600;

	/* Minute */
	sys_seconds_tmp += pDate->minute * 60;

	/* Second */
	sys_seconds_tmp += pDate->second;

        // remember offset we have applied, so we can report seconds since boot
        long long sys_seconds_new = sys_seconds_tmp + offset;
        sys_seconds_offset += sys_seconds_new - sys_seconds;
        sys_seconds = sys_seconds_new;
        
	return;
}

long long tm_to_time(system_date_t *pDate, int offset)
{
	long long sys_seconds_tmp = 0;
	unsigned short tmp_y = 0;
	int i = 0;

	/* Year */
	for (tmp_y = 1970 ; tmp_y < pDate->year ; tmp_y++) {
		sys_seconds_tmp += year_sec[i % 4];
		i++;
	}

	/* Month */
	/* Update Feb */
	if (pDate->year % 4 == 0) {
		mon_sec[1] = 2505600;	// Leap year
	} else {
		mon_sec[1] = 2419200;
	}

	for (i = 0 ; i < (pDate->month - 1) ; i++) {
		sys_seconds_tmp += mon_sec[i];
	}

	/* Day */
	sys_seconds_tmp += (pDate->day - 1) * 86400;

	/* Hour */
	sys_seconds_tmp += pDate->hour * 3600;

	/* Minute */
	sys_seconds_tmp += pDate->minute * 60;

	/* Second */
	sys_seconds_tmp += pDate->second;

	return (sys_seconds_tmp + offset);
}

void time_to_tm(long long totalsecs, int offset, system_date_t *pDate)
{
	unsigned short tmp_y = 0, tmp_m = 0, tmp_d = 0;
	long long sys_seconds_tmp = totalsecs;
	int i = 0;

	if (sys_seconds_tmp >= 31536000) {
		goto calc_year;
	} else if (sys_seconds_tmp >= 2419200) {
		goto calc_month;
	} else if (sys_seconds_tmp >= 86400) {
		goto calc_day;
	} else {
		goto calc_time;
	}

calc_year:
	/* Calculate year */
	for (i = 0 ; i < 250 ; i++) {
		if (sys_seconds_tmp - year_sec[tmp_y % 4] >= 0) {
			sys_seconds_tmp = sys_seconds_tmp - year_sec[tmp_y % 4];
			tmp_y++;
		} else {
			break;
		}
	}

calc_month:
	/* Update Feb */
	if ((tmp_y + 1970) % 4 == 0) {
		mon_sec[1] = 2505600;	// Leap year
	} else {
		mon_sec[1] = 2419200;
	}

	/* Calculate Month */
	for (i = 0 ; i < 12 ; i++) {
		if (sys_seconds_tmp - mon_sec[i] >= 0) {
			sys_seconds_tmp -= mon_sec[i];
			tmp_m++;
		} else {
			break;
		}
	}

calc_day:
	/* calculate day */
	tmp_d = sys_seconds_tmp / 86400;
	sys_seconds_tmp = sys_seconds_tmp % 86400;

calc_time:
	pDate->hour = sys_seconds_tmp / 3600;
	pDate->minute = (sys_seconds_tmp % 3600) / 60;
	pDate->second =  (sys_seconds_tmp % 3600 % 60);

	pDate->year = 1970 + tmp_y;
	pDate->month = 1 + tmp_m;
	pDate->day = 1 + tmp_d;

	return;
}

long long get_sys_seconds()
{
    return sys_seconds + sys_seconds_tz_offset;
}

long long get_sys_seconds_utc()
{
    return sys_seconds;
}

long long get_sys_seconds_boot(void)
{
    return sys_seconds - sys_seconds_offset;
}

void set_tz_offset(long seconds)
{
    sys_seconds_tz_offset = seconds;
}
