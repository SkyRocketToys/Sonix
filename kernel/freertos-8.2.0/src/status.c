#include <FreeRTOS.h>
#include <task.h>
#include <status.h>


typedef struct status_list_chain {
	unsigned int total_items;
	struct status_list_t *table;
	struct status_list_chain_t *next_chain;
}status_list_chain_t;

static status_list_chain_t *list_chain;
static status_list_chain_t *list_top;

int status_list_register(struct status_list_t *table, unsigned int size)
{

	taskENTER_CRITICAL();
	{
		list_chain->table = table;
		list_chain->total_items = size / sizeof(status_list_t);

		list_chain->next_chain = pvPortMalloc(sizeof(status_list_chain_t), GFP_KERNEL, MODULE_KERNEL );

		list_chain = list_chain->next_chain;
		list_chain->next_chain = NULL;
		list_chain->table = NULL;
		list_chain->total_items = 0;
	}
	taskEXIT_CRITICAL();

	return 0;
}

int status_list_resign(struct status_list_t *table)
{
	status_list_chain_t *list;
	status_list_chain_t *next_list;
	int ret = 0;

	list = list_top;

	taskENTER_CRITICAL();
	{
		if (list_top->table == table) {
			list_top = list_top->next_chain;
			vPortFree(list);

			ret = pdPASS;
		} else {
			while (list->table != NULL) {
				next_list = list->next_chain;

				if (next_list->table == table) {
					list->next_chain = next_list->next_chain;
					vPortFree(next_list);

					ret = pdPASS;
					break;
				}

				list = list->next_chain;

			}

		}
	}
	taskEXIT_CRITICAL();

	return ret;
}

int show_status_value(char *name)
{
	status_list_chain_t *list;
	status_list_t *tbl;
	int i = 0;
	int len = 0;
	int value = 0;

	list = list_top;
	len = strlen(name);

	while (list->table != NULL) {
		tbl = list->table;

		for (i = 0 ; i < list->total_items ; i++) {
			if (strncmp(name, (tbl + i)->name, len) == 0) {
				switch ((tbl + i)->format) {
					case 1: // Dec
						print_msg("%s = %d\n", (tbl + i)->desc, (int)(tbl + i)->value((tbl + i)->id));
						break;
					case 2: // Hex
						print_msg("%s = 0x%x\n", (tbl + i)->desc, (unsigned int)(tbl + i)->value((tbl + i)->id) );
						break;
				}

				goto out;
			}

		}

		list = list->next_chain;
	}

out:
	return 0;

}

void dump_list()
{
	status_list_chain_t *list;
	status_list_t *tbl;
	int i = 0;

	list = list_top;

	while (list->table != NULL) {
		tbl = list->table;

		for (i = 0 ; i < list->total_items ; i++) {
			//print_msg("Name = %s\n", (tbl + i)->name);
			//print_msg("Desc = %s\n", (tbl + i)->desc);
			//print_msg("ID = %d\n", (tbl + i)->id);


			switch ((tbl + i)->format) {
				case 1:
					print_msg("%s = %d\n", (tbl + i)->desc, (int)(tbl + i)->value((tbl + i)->id));
					break;
				case 2: // Hex
					print_msg("%s 	= 0x%x\n", (tbl + i)->desc, (unsigned int)(tbl + i)->value((tbl + i)->id) );
					break;
			}

		}

		list = list->next_chain;
	}


}

void init_status_list()
{
	list_chain = pvPortMalloc(sizeof(status_list_chain_t), GFP_KERNEL, MODULE_KERNEL );

	list_top = list_chain;

}
