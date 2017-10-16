#include <FreeRTOS.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cmd_nvram.h"
#include <libmid_nvram/snx_mid_nvram.h>
#include <nonstdlib.h>

#define DS5_DEBUG			1

/** \defgroup cmd_nvram NVRAM commands
 *  \ingroup system_cli
 * @{
 */

int cmd_nvram_pkg_list(void)
{
	nvram_pkg_name_info_t *pkg_name_info = NULL;
	int pkg_cnt = 0;

	pkg_cnt = snx_nvram_get_all_pkg_count();
	pkg_name_info = (nvram_pkg_name_info_t *) pvPortMalloc(sizeof(nvram_pkg_name_info_t)*pkg_cnt, GFP_KERNEL, MODULE_CLI);
	memset(pkg_name_info, 0x00, sizeof(nvram_pkg_name_info_t)*pkg_cnt);
	snx_nvram_get_pkg_name_list(pkg_name_info);

	print_msg_queue("All pack name list: (total count: %d)\n", pkg_cnt);
	print_msg_queue("List format: [pack name]\n");
	while(pkg_cnt--) {
		print_msg_queue("[%s]\n", (pkg_name_info+pkg_cnt)->name);
	}

	vPortFree(pkg_name_info);

	return 0;
}

int cmd_nvram_cfg_list(char *pkg_name)
{
	char *cfg_name = NULL;
	nvram_cfg_name_data_info_t *cfg_name_info = NULL;
	int cfg_cnt = 0;
	void *cfg_data = NULL;

	cfg_cnt = snx_nvram_get_all_cfg_count(pkg_name);
	cfg_name_info = (nvram_cfg_name_data_info_t *) pvPortMalloc(sizeof(nvram_cfg_name_data_info_t)*cfg_cnt, GFP_KERNEL, MODULE_CLI);
	memset(cfg_name_info, 0x00, sizeof(nvram_cfg_name_data_info_t)*cfg_cnt);
	if (snx_nvram_get_cfg_name_list(pkg_name, cfg_name_info) == NVRAM_E_PKGNOEXIST) {
		print_msg_queue("Search pack name: [%s], This pack name not exist\n", pkg_name, cfg_cnt);
		return 0;
	}

	print_msg_queue("Pack name: %s, All config list: (total count: %d)\n", pkg_name, cfg_cnt);
	print_msg_queue("List format: [config name] [data length] [data type] [data]\n");

	while(cfg_cnt--) {
		cfg_name = (cfg_name_info+cfg_cnt)->name;
		print_msg_queue("[%s] [%u Byte(s)] ", cfg_name, (cfg_name_info+cfg_cnt)->data_info.data_len);
		cfg_data = (void *) pvPortMalloc((cfg_name_info+cfg_cnt)->data_info.data_len, GFP_KERNEL, MODULE_CLI);
		(cfg_name_info+cfg_cnt)->data_info.data = cfg_data;
		snx_nvram_get_immediately(pkg_name, cfg_name, &(cfg_name_info+cfg_cnt)->data_info);
		switch((cfg_name_info+cfg_cnt)->data_info.data_type) {
		case NVRAM_DT_STRING:
			print_msg_queue("[string] [%s]\n", (cfg_name_info+cfg_cnt)->data_info.data);
			break;

		case NVRAM_DT_BIN_RAW:
			print_msg_queue("[file] [Not support show bin data]\n");
			break;

		case NVRAM_DT_INT:
			print_msg_queue("[int] [%d]\n", *((int *)(cfg_name_info+cfg_cnt)->data_info.data));
			break;

		case NVRAM_DT_FLOAT:
			print_msg_queue("[float] [%f]\n", *((float *)(cfg_name_info+cfg_cnt)->data_info.data));
			break;

		case NVRAM_DT_UINT:
			print_msg_queue("[unsigned int] [%u]\n", *((unsigned int *)(cfg_name_info+cfg_cnt)->data_info.data));
			break;

		case NVRAM_DT_UCHAR:
			print_msg_queue("[unsigned char] [Not support show uchar data]\n");
			//print_msg_queue("[unsigned char] [%bx]\n", (unsigned char *)(cfg_name_info+cfg_cnt)->data_info.data);
			break;

		default:
			print_msg_queue("[unknown type] [unknown data]\n");
			break;
		}
		vPortFree(cfg_data);
		cfg_data = NULL;
	}

	vPortFree(cfg_name_info);

	return 0;
}

int cmd_nvram_list_all_name(int argc, char* argv[])
{
	if ((argc == 2) && (strncmp(argv[1], "-p", 2) == 0)) {
			cmd_nvram_pkg_list();
	} else if ((argc == 3) && (strncmp(argv[1], "-c", 2) == 0)) {
		cmd_nvram_cfg_list(argv[2]);
	} else {
		print_msg_queue("Usage: %s [Option]\n", argv[0]);
		print_msg_queue("Available options are\n");
		print_msg_queue("-p                List all of pack infoprmation\n");
		print_msg_queue("-c pack_name      List pack_name all of config information\n");
	}
	return 0;
}

int cmd_nvram_set(int argc, char* argv[])
{
	nvram_data_info_t data_info;
	char *pkg_name;
	char *cfg_name;
	char *err_str = NULL;
	int mode = 0;	//0: nvram config in ram, 1: nvram config in flash.
	void *data;

	if (argc >= 5) {
		pkg_name = argv[1];
		cfg_name = argv[2];
		data_info.data_type = atoi(argv[3]);
		switch(data_info.data_type) {
			case NVRAM_DT_STRING:
				data_info.data_len = strlen(argv[4]);
				data = (void *)argv[4];
				data_info.data = data;
				break;

			case NVRAM_DT_INT:
			case NVRAM_DT_UINT:
				data_info.data_len = 4;
				data = (void *)atoi(argv[4]);
				data_info.data = &data;
				break;

			case NVRAM_DT_FLOAT:
				//data_info.data = atof(argv[4]);
				//break;

			case NVRAM_DT_UCHAR:
			case NVRAM_DT_BIN_RAW:
			default:
				err_str = "In CLI this type not yet supported";
				break;
		}
		if ((argc == 6) && (strncmp(argv[5], "-s", 2) == 0)) {
			mode = 1;
		}
	} else {
		err_str = "Command format error";
	}

	if (err_str == NULL) {
		if (mode == 1) {
			if (snx_nvram_set_immediately(pkg_name, cfg_name, &data_info) == NVRAM_SUCCESS) {
				print_msg_queue("[Flash] set NVRAM config done!\n");
			}
		} else {
			if (snx_nvram_set(pkg_name, cfg_name, &data_info) == NVRAM_SUCCESS) {
				print_msg_queue("[Ram] set NVRAM config done!\n");
			}
		}
	} else {
		print_msg_queue("%s\n\n", err_str);

		print_msg_queue("Usage: %s [pack name] [config name] [data type] [data] [Option]\n", argv[0]);
		print_msg_queue("data type are:\n");
		print_msg_queue("0: String\n");
		print_msg_queue("1: file (Command not Support)\n");
		print_msg_queue("2: int\n");
		print_msg_queue("3: float\n");
		print_msg_queue("4: unsigned int\n");
		print_msg_queue("5: unsigned char (Hex data)\n");
		print_msg_queue("example: set nvram_test test_cfg 0 string_test\n");
		print_msg_queue("Available options are\n");
		print_msg_queue("-s This config data immediately save to flash\n");
	}
	return 0;
}
/** @} */
