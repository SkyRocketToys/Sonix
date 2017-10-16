#include <FreeRTOS.h>
#include <bsp.h>
#include <task.h>
#include <cmd_verify.h>

/** \defgroup cmd_verify_memtest Memory test commands
 *  \ingroup cmd_verify
 * @{
 */

typedef struct param {
	unsigned int *addr;
	int size;
	int counter;
}param_t;

/**
* @brief Memory test test
* @param [size] Allocate memory size to test
* @param [counter] How many times to test
* @details Example: memtest [size] [counter]
*/
void memory_test_task(void *pvParameters)
{
	param_t *params = (param_t *)pvParameters;
	int i = 0, j = 0;
	int size = 0;
	int counter = 0;
	unsigned int *addr;

	size = params->size;
	counter = params->counter;
	addr = params->addr;

	for (;;) {
		for (j = 0 ; j < counter ; j++) {
			for (i = 0 ; i < (size / 4) ; i++) {
				*(addr + i) = 0x5a5a5a5a;
			}

			for (i = 0 ; i < (size / 4) ; i++) {
				if (*(addr + i) != 0x5a5a5a5a ) {
					print_msg_queue("Memory test fail at 0x%x !!!\n", addr);
					goto out;
				}
			}
			vTaskDelay( 20 / portTICK_RATE_MS );
		}

		print_msg_queue("Memory test Pass\n");
		goto out;
	}

out:
	vPortFree(addr);
	vTaskDelete(NULL);

}

int cmd_verify_memtest(int argc, char* argv[])
{
	param_t mem_param;

	if (argc < 3) {
		print_msg_queue("Not enough arguments!!!\n");
		goto out;
	} else {
		mem_param.size = simple_strtoul(argv[1], NULL, 10);
		mem_param.counter = simple_strtoul(argv[2], NULL, 10);
	}

	mem_param.addr = pvPortMalloc(mem_param.size, GFP_DMA, MODULE_CLI);

	if (mem_param.addr == NULL) {
		print_msg_queue("Memory allocate fail !!!\n");
		goto out;
	}

	if (pdPASS != xTaskCreate(memory_test_task, "memory test", 128, (void*) &mem_param, 2, NULL)) {
		print_msg_queue("Could not create task memory_test_task\n");
	}


	//vPortFree(addr);
out:
	return 0;
}
/** @} */
