#include <FreeRTOS.h>
#include <queue.h>
#include <task.h>
#include <bsp.h>
#include <stdarg.h>
#include <nonstdlib.h>
#include <stdio.h>
#include <sys_clock.h>
#include "printlog.h"

#define PRINT_QUEUE_DELAY		50

static QueueHandle_t PrintLogQueue;
static QueueHandle_t system_log_queue;

void PrintLogTask(void* params)
{
	portCHAR message[CFG_PBSIZE];

	for ( ; ; ) {
		/* The task is blocked until something appears in the queue */
		xQueueReceive(PrintLogQueue, (void*) &message, portMAX_DELAY);

		print_msg("%s", message);
	}

	/* if it ever breaks out of the infinite loop... */
	vTaskDelete(NULL);

	(void) params;
}

void Init_PrintLog_Task()
{
	PrintLogQueue = xQueueCreate(8, CFG_PBSIZE);

	xTaskCreate(PrintLogTask, "printlog", STACK_SIZE_1K, NULL, PRIORITY_TASK_CLI_TX, NULL);

}

int32_t vSendMsgToQueue(char ch[])
{
	return xQueueSendToBack(PrintLogQueue, (void*) ch, PRINT_QUEUE_DELAY);
}

#define MAX_LOG_NUM		127

static sys_log_data_t *curr_log_data = NULL;
static index_log_data_t *log_index = NULL;
static int wrap = 0;

void system_log_task(void* params)
{
	for (; ;) {
		/* The task is blocked until something appears in the queue */
		curr_log_data = (sys_log_data_t *)log_index + log_index->last_counter;

		xQueueReceive(system_log_queue, (void*) curr_log_data, portMAX_DELAY);


		if (log_index->last_counter < MAX_LOG_NUM) {
			log_index->last_counter++;

			if (wrap > 0) {
				log_index->start_counter = log_index->last_counter;
			}

		} else {
			log_index->last_counter = 1;
			wrap++;
		}
	}

	/* if it ever breaks out of the infinite loop... */
	vTaskDelete(NULL);

	(void) params;
}

void init_systemlog_task()
{
	system_log_queue = xQueueCreate(10, sizeof(sys_log_data_t));

	log_index = pvPortMalloc((MAX_LOG_NUM + 1) * sizeof(sys_log_data_t), GFP_KERNEL, MODULE_CLI);

	print_msg("log addr = 0x%x\n", log_index);

	log_index->last_counter = 1;
	log_index->start_counter = 1;

	xTaskCreate(system_log_task, "system_log", STACK_SIZE_1K, NULL, PRIORITY_TASK_SYS_LOG, NULL);
}

void system_log(char type, unsigned short num)
{
	sys_log_data_t log_data;

	log_data.type = type;
	log_data.num = num;
	log_data.ts_sec = get_sys_seconds();

	xQueueSendToBack(system_log_queue, (void *) &log_data, PRINT_QUEUE_DELAY);
}

sys_log_data_t *get_system_log()
{
	//return top_log_data;
	return 0;
}

void show_system_log()
{
	int i = 0, end = 0;
	sys_log_data_t *log = NULL;
	unsigned int date_h, date_l;

	log = (sys_log_data_t *)log_index;

	if (log_index->start_counter != 1 || log_index->last_counter == 1) {
		end = MAX_LOG_NUM;
	} else {
		end = log_index->last_counter - 1;
	}

	for (i = log_index->start_counter ; i <= end ; i++) {
		date_h = (((log + i)->ts_sec) >> 32) & 0xffffffff;
		date_l = ((log + i)->ts_sec) & 0xffffffff;
		print_msg_queue("sec = %d%d", date_h, date_l);
		print_msg_queue(" type = %c", (log + i)->type);
		print_msg_queue(" num = %d\n", (log + i)->num);

	}

	if (log_index->start_counter != 1) {
		for (i = 1 ; i < log_index->start_counter ; i++) {
			date_h = (((log + i)->ts_sec) >> 32) & 0xffffffff;
			date_l = ((log + i)->ts_sec) & 0xffffffff;
			print_msg_queue("sec = %d%d", date_h, date_l);
			print_msg_queue(" type = %c", (log + i)->type);
			print_msg_queue(" num = %d\n", (log + i)->num);
		}
	}

}
