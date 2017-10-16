#include <FreeRTOS.h>
#include <queue.h>
#include <task.h>

//#include "app_config.h"
#include <bsp.h>
#include <uart/uart.h>
#include <interrupt.h>
#include <nonstdlib.h>
#include <string.h>
#include "rtoscli.h"
#include "printlog.h"

#define RECV_QUEUE_SIZE		10
#define RECV_QUEUE_STACK_SIZE	1024

/* A queue for received characters, not processed yet */
static QueueHandle_t recvQueue;


/* forward declaration of an ISR handler: */
//static void recvIsrHandler(int irq);

xTaskHandle xHandle_recv;

/**
 * Initializes all receive related tasks and synchronization primitives.
 * This function must be called before anything is attempted to be received!
 *
 * @param uart_nr - number of the UART
 *
 * @return pdPASS if initialization is successful, pdFAIL otherwise
 */
int16_t recvInit()
{
    /* Create and assert a queue for received characters */
    recvQueue = xQueueCreate(RECV_QUEUE_SIZE, sizeof(portCHAR));
    if ( 0 == recvQueue )
    {
        return pdFAIL;
    }

    return pdPASS;
}

void rtoscli_recv(char ch)
{
	xQueueSendToBackFromISR(recvQueue, (void*) &ch, 0);
}

#define MAX_CMD_LENGTH		48

/**
 * A FreeRTOS task that processes received characters.
 * The task is waiting in blocked state until the ISR handler pushes something
 * into the queue. If the received character is valid, it will be appended to a
 * string buffer. When 'Enter' is pressed, the entire string will be sent to UART0.
 *
 * @param params - ignored
 */
void cli_recv_task(void* params)
{
	char ch;
	char cmdbuf[MAX_CMD_LENGTH];
	int ret = 0;
	int i = 0;
	init_rtos_cli();

	for ( ; ; ) {
		/* The task is blocked until something appears in the queue */
		if (xQueueReceive(recvQueue, (void*) &ch, portMAX_DELAY)) {
			if (!ispasswd()) {
				print_char(ch);
			} else {
				//print_char('*');
			}

			if (ch == 0xd) { /* enter key */
				if (i > 0) {
					print_char('\n');
					cmdbuf[i] = 0x0;

					//printf("cmd=%s\n", cmdbuf);
					ret = parse_cmd(cmdbuf, 0);
					memset(cmdbuf, 0, sizeof(cmdbuf));
					i = 0;
				}

				show_cli_prompt();
				//printf(PROMPT, curr_lv[0].name);
			} else if (ch == 0x7f || ch == 0x08) { // Del key, Ctrl-? or Ctrl-H
				print_char(0x20);
				print_char(ch);
				if (i > 0) {
					i--;
					cmdbuf[i] = 0x0;
				}
			} else {
				//printf("%c", c);
				cmdbuf[i] = ch;
				i++;
				if (i >= MAX_CMD_LENGTH - 1) { // Over Max command length, Force excute command to avoid buffer overflow
					print_char('\n');
					cmdbuf[MAX_CMD_LENGTH - 1] = 0x0;

					ret = parse_cmd(cmdbuf, 0);
					memset(cmdbuf, 0, sizeof(cmdbuf));
					i = 0;

					show_cli_prompt();
				}
			}
		}
	}

	(void) params;
}

void init_recv_task()
{
	//RBK the stack size was 5k, increate to 5k*3 due to stack overflow
	xTaskCreate(cli_recv_task, "cli_recv", STACK_SIZE_5K*3, NULL, PRIORITY_TASK_CLI_RX, &xHandle_recv);
}
