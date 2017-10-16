#ifndef _TIMER_H_
#define _TIMER_H_

void timer_init_prepare(void);

void timer_start_prepare(void);

void timer_enable_intr_prepare(void);

void timer_clear_intr_prepare(void);

void timer_set_load_prepare(uint32_t counter);


#endif  /* _TIMER_H_*/
