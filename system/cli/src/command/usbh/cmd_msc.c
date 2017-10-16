#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "queue.h"
#include <string.h>

#include "cmd_msc.h"

#include <generated/snx_sdk_conf.h>

#include "../.././driver/sn986xx/usb_host/src/USBH-ERR-HDL.h"
#include "../.././driver/sn986xx/usb_host/src/USBH-CORE.h"
#include "../.././driver/sn986xx/usb_host/src/MSC.h"

#define	SUCCESS		0	/**< success */
#define	FAIL		1	/**< fail */
#define	NYET		2	/**< not yet */
#define	TIME_OUT	3	/**< timeout */

typedef struct {
	uint32_t time;
	uint32_t format; 		// 1:continuous 2:random
	uint32_t startSectorNum;
	uint32_t endSectorNum;
	uint32_t compareType; 	// 1:R/W compare 2:R/W non-compare 3:R 4:W
	uint32_t totalSectorNums;

} MSC_TEST_STRUCTURE;

typedef enum {
	MSC_TEST_NONE,
	MSC_TEST_TIME,
	MSC_TEST_FORMAT,
	MSC_TEST_START_SEC,
	MSC_TEST_END_SEC,
	MSC_TEST_COM_TYPE,
	MSC_TEST_CLEAR,
	MSC_SHOW_DEV_INFO,
	MSC_SHOW_TEST_PARAM,
	MSC_TEST_EXE,

} MSC_TEST_ITEM_MODE;

MSC_TEST_STRUCTURE msc_test;

int msc_test_item_init(int argc, char* argv[]) {

	memset(&msc_test, 0, sizeof(MSC_TEST_STRUCTURE));

	MSC_DEVICE_STRUCT *msc_dev = NULL;

	msc_dev = (MSC_DEVICE_STRUCT*) &MSC_DEV;

	if (msc_dev->info[0].block_len != 0) {
		msc_test.totalSectorNums = msc_dev->info[0].lba + 1;
	} else {
		msc_test.totalSectorNums = 0;
	}

	msc_test.time = 100;
	msc_test.format = 1;
	msc_test.startSectorNum = 1;
	//msc_test.endSectorNum = msc_test.totalSectorNums;
	msc_test.endSectorNum = 1000;
	msc_test.compareType = 1;

	return pdPASS;
}

int msc_test_item_chk(int argc, char* argv[]) {

	MSC_TEST_ITEM_MODE mode = MSC_TEST_NONE;

	if (strcmp(argv[0], "test_time") == 0) {
		mode = MSC_TEST_TIME;
	} else if (strcmp(argv[0], "test_format") == 0) {
		mode = MSC_TEST_FORMAT;
	} else if (strcmp(argv[0], "test_start_sec") == 0) {
		mode = MSC_TEST_START_SEC;
	} else if (strcmp(argv[0], "test_end_sec") == 0) {
		mode = MSC_TEST_END_SEC;
	} else if (strcmp(argv[0], "test_com_type") == 0) {
		mode = MSC_TEST_COM_TYPE;
	} else if (strcmp(argv[0], "test_clear") == 0) {
		mode = MSC_TEST_CLEAR;

		if (argc == 1) {
			return msc_test_item_set(mode, 0);
		}

		print_msg_queue("parameter set error\n");
		print_msg_queue("only <-- %s -- > allowed", argv[0]);

		goto ERR;

	} else if ((strcmp(argv[0], "device_info")) == 0) {
		mode = MSC_SHOW_DEV_INFO;

		if (argc == 1) {
			return msc_show_dev_info();
		}

		print_msg_queue("parameter set error\n");
		print_msg_queue("only <-- %s -- > allowed", argv[0]);

		goto ERR;

	} else if ((strcmp(argv[0], "test_params")) == 0) {
		mode = MSC_SHOW_TEST_PARAM;

		if (argc == 1) {
			return msc_show_test_param();
		}

		print_msg_queue("parameter set error\n");
		print_msg_queue("only <-- %s -- > allowed", argv[0]);

		goto ERR;

	} else if ((strcmp(argv[0], "test_exe")) == 0) {
		mode = MSC_TEST_EXE;

		if (argc == 1) {
			return msc_test_exe();
		}

		print_msg_queue("parameter set error\n");
		print_msg_queue("only <-- %s -- > allowed", argv[0]);

		goto ERR;

	} else {
		print_msg_queue("Command not found\n");
		cmd_help(0, NULL);

		goto ERR;
	}

	if (argc == 2) {
		uint32_t num = 0;

		num = atoi(argv[1]);

		if (num > 0) {
			return msc_test_item_set(mode, num);
		}
	}

	print_msg_queue("parameter set error\n");
	print_msg_queue("only <-- %s (integer(greater than zero))-- > allowed",
			argv[0]);

	ERR: return pdFAIL;
}

int comType_threefour_test_exe() {

	USBH_Device_Structure *dev = NULL;
	uint32_t addr = 0;
	uint32_t i = 1;

	uint32_t j = 0;

	uint32_t size;
	size_t pattern_size;
	uint8_t *pattern_buf;
	uint8_t *compare_buf;

	dev = (USBH_Device_Structure*) msc_init(1);
	if (dev == NULL) {
		return print_err_msg("ERROR: MSC device not exist or not ready\n");
	}
	size = ((dev->BUFF.size / 2) & 0xFFFFF000);
	pattern_size = size;

	pattern_buf = (uint8_t*) (dev->BUFF.ptr);
	pattern_buf = (uint8_t*) (((uint32_t) pattern_buf & 0xFFFFFC00) + 0x400); //do 1k alignment
	compare_buf = (uint8_t*) (((uint32_t) (pattern_buf)) + size);

	memset(compare_buf, 0, pattern_size);
	for (j = 0; j < pattern_size; j++) {
		pattern_buf[j] = rand() % 256;
	}

	if (msc_test.format == 1) {
		while (i <= msc_test.time) {

			addr = msc_test.startSectorNum - 1;

			while ((addr + 1) <= msc_test.endSectorNum) {

				if (msc_test.compareType == 4) {
					if (msc_write(dev, addr, (uint8_t*) pattern_buf,
							(pattern_size / 512)) == FAIL) {
						MSC_DBG("MSC Write 10 FAIL");
						goto ERR;
					}
				}
				if (msc_test.compareType == 3) {
					if (msc_read(dev, (uint8_t*) compare_buf, addr,
							(pattern_size / 512)) == FAIL) {
						MSC_DBG("MSC Read 10 FAIL");
						goto ERR;
					}
				}
				MSC_INFO(".");
				addr += (pattern_size / 512);
			}
			MSC_INFO("\n");
			i++;
		}
	} else if (msc_test.format == 2) {

		int temp = msc_test.time
				* (msc_test.endSectorNum - msc_test.startSectorNum + 1);

		if (temp >= (pattern_size / 512)) {
			temp = temp / (pattern_size / 512);
		}

		while (i <= temp) {

			addr = (rand()
					% (msc_test.endSectorNum - msc_test.startSectorNum + 1))
					+ msc_test.startSectorNum;

			if (msc_test.compareType == 4) {
				if (msc_write(dev, addr, (uint8_t*) pattern_buf,
						(pattern_size / 512)) == FAIL) {
					MSC_DBG("MSC Write 10 FAIL");
					goto ERR;
				}
			}
			if (msc_test.compareType == 3) {
				if (msc_read(dev, (uint8_t*) compare_buf, addr,
						(pattern_size / 512)) == FAIL) {
					MSC_DBG("MSC Read 10 FAIL");
					goto ERR;
				}
			}

			MSC_INFO(".");
			i++;
		}
		MSC_INFO("\n");

	} else {
		goto ERR;
	}

	return SUCCESS;

	ERR: return FAIL;
}

int comType_onetwo_test_exe() {

	USBH_Device_Structure *dev = NULL;
	uint32_t addr = 0;
	uint32_t i = 1;

	uint32_t j = 0;

	uint32_t size;
	size_t pattern_size;
	uint8_t *pattern_buf;
	uint8_t *compare_buf;

	dev = (USBH_Device_Structure*) msc_init(1);
	if (dev == NULL) {
		return print_err_msg("ERROR: MSC device not exist or not ready\n");
	}
	size = ((dev->BUFF.size / 2) & 0xFFFFF000);
	pattern_size = size;

	pattern_buf = (uint8_t*) (dev->BUFF.ptr);
	pattern_buf = (uint8_t*) (((uint32_t) pattern_buf & 0xFFFFFC00) + 0x400); //do 1k alignment
	compare_buf = (uint8_t*) (((uint32_t) (pattern_buf)) + size);

	memset(compare_buf, 0, pattern_size);
	for (j = 0; j < pattern_size; j++) {
		pattern_buf[j] = rand() % 256;
	}

	if (msc_test.format == 1) {
		while (i <= msc_test.time) {

			addr = msc_test.startSectorNum - 1;

			while ((addr + 1) <= msc_test.endSectorNum) {
				if (msc_test.compareType == 1) {

					for (j = 0; j < pattern_size; j++) {
						pattern_buf[j] = rand() % 256;
					}
				}
				if (msc_write(dev, addr, (uint8_t*) pattern_buf,
						(pattern_size / 512)) == FAIL) {
					MSC_DBG("MSC Write 10 FAIL");
					goto ERR;
				}
				if (msc_read(dev, (uint8_t*) compare_buf, addr,
						(pattern_size / 512)) == FAIL) {
					MSC_DBG("MSC Read 10 FAIL");
					goto ERR;
				}

				if (msc_test.compareType == 1) {
					if (memcmp(pattern_buf, compare_buf, pattern_size) != 0) {
						MSC_DBG("MSC Data Compare FAIL");
						goto ERR;
					}
				}
				MSC_INFO(".");
				addr += (pattern_size / 512);
			}
			MSC_INFO("\n");
			i++;
		}
	} else if (msc_test.format == 2) {

		int temp = msc_test.time
				* (msc_test.endSectorNum - msc_test.startSectorNum + 1);

		if (temp >= (pattern_size / 512)) {
			temp = temp / (pattern_size / 512);
		}

		while (i <= temp) {

			addr = (rand()
					% (msc_test.endSectorNum - msc_test.startSectorNum + 1))
					+ msc_test.startSectorNum;

			if (msc_test.compareType == 1) {

				for (j = 0; j < pattern_size; j++) {
					pattern_buf[j] = rand() % 256;
				}
			}
			if (msc_write(dev, addr, (uint8_t*) pattern_buf,
					(pattern_size / 512)) == FAIL) {
				MSC_DBG("MSC Write 10 FAIL");
				goto ERR;
			}
			if (msc_read(dev, (uint8_t*) compare_buf, addr,
					(pattern_size / 512)) == FAIL) {
				MSC_DBG("MSC Read 10 FAIL");
				goto ERR;
			}

			if (msc_test.compareType == 1) {
				if (memcmp(pattern_buf, compare_buf, pattern_size) != 0) {
					MSC_DBG("MSC Data Compare FAIL");
					goto ERR;
				}
			}
			MSC_INFO(".");
			i++;
		}
		MSC_INFO("\n");

	} else {
		goto ERR;
	}

	return SUCCESS;

	ERR: return FAIL;

}

int print_err_msg(char *str) {

	print_msg_queue("%s", str);

	return FAIL;
}

int chk_test_params() {

	USBH_Device_Structure *dev = NULL;
	MSC_DEVICE_STRUCT *msc_dev = NULL;

	dev = (USBH_Device_Structure*) msc_init(1);
	if (dev == NULL) {
		return print_err_msg("ERROR: MSC device not exist or not ready\n");
	}

	msc_dev = (MSC_DEVICE_STRUCT*) &MSC_DEV;

	msc_test.totalSectorNums = msc_dev->info[0].lba + 1;

	if (msc_test.totalSectorNums == 1) {
		return print_err_msg("ERROR: MSC device not exist or not ready\n");
	}

	uint32_t size = 0;
	size_t pattern_size = 0;
	size = ((dev->BUFF.size / 2) & 0xFFFFF000);
	pattern_size = size;
	if (msc_test.endSectorNum <= (msc_test.totalSectorNums - (pattern_size/512) + 1)) {
	} else {
		return print_err_msg("ERROR: end sector number exceeds boundary\n");
	}

	return SUCCESS;
}

int msc_test_exe() {

	if (chk_test_params() == FAIL) {
		return FAIL;
	}

	if ((msc_test.compareType == 1) || (msc_test.compareType == 2)) {
		return comType_onetwo_test_exe();
	} else if ((msc_test.compareType == 3) || (msc_test.compareType == 4)) {
		return comType_threefour_test_exe();
	}

	return FAIL;
}

int msc_show_test_param() {

	print_msg_queue("test_time:      %d\n", msc_test.time);
	print_msg_queue("test_format:    %d\n", msc_test.format);
	print_msg_queue("test_start_sec: %d\n", msc_test.startSectorNum);
	print_msg_queue("test_end_sec:   %d\n", msc_test.endSectorNum);
	print_msg_queue("test_com_type:  %d\n", msc_test.compareType);

	return pdPASS;
}

int msc_show_dev_info() {

	MSC_DEVICE_STRUCT *msc_dev = NULL;

	msc_dev = (MSC_DEVICE_STRUCT*) &MSC_DEV;

	if (msc_dev->info[0].block_len != 0) {
		print_msg_queue("DevInfo: Each  block length : %d\n",
				msc_dev->info[0].block_len);
		print_msg_queue("DevInfo: Total block numbers: %d\n",
				msc_dev->info[0].lba + 1);

		return pdPASS;
	}

	print_msg_queue("ERROR: MSC device not exist or not ready\n");

	return pdFAIL;
}

int msc_test_item_set(int mode, uint32_t num) {

	switch (mode) {
	case MSC_TEST_NONE:
		break;
	case MSC_TEST_TIME:
		msc_test.time = num;
		break;
	case MSC_TEST_FORMAT:
		if ((num != 1) && (num != 2)) {
			print_msg_queue("ERROR: only 1:continuous 2: random allowed\n");
			break;
		}
		msc_test.format = num;
		break;
	case MSC_TEST_START_SEC:
		if (num <= msc_test.endSectorNum) {
			msc_test.startSectorNum = num;
		} else {
			print_msg_queue(
					"ERROR: start sector number exceeds end sector number\n");
		}
		break;
	case MSC_TEST_END_SEC:
		if (msc_test.totalSectorNums == 0) {
			MSC_DEVICE_STRUCT *msc_dev = NULL;

			msc_dev = (MSC_DEVICE_STRUCT*) &MSC_DEV;

			if (msc_dev->info[0].block_len != 0) {
				msc_test.totalSectorNums = msc_dev->info[0].lba + 1;
			}
		}
		if ((msc_test.totalSectorNums == 0)
				|| (num <= (msc_test.totalSectorNums - 8 + 1))) {
			msc_test.endSectorNum = num;
		} else {
			print_msg_queue("ERROR: end sector number exceeds boundary\n");
		}
		break;
	case MSC_TEST_COM_TYPE:
		if ((num != 1) && (num != 2) && (num != 3) && (num != 4)) {
			print_msg_queue(
					"ERROR: only 1:R/W compare 2:R/W non-compare 3:R 4:W allowed\n");
			break;
		}
		msc_test.compareType = num;
		break;
	case MSC_TEST_CLEAR:
		msc_test_item_init(0, NULL);
		break;
	default:
		break;
	}

	return pdPASS;
}

