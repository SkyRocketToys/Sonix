#ifndef __FLASH_INFO_H__
#define __FLASH_INFO_H__

#include <stdio.h>

#define LINE_MAX_LEN 128

#define FLASH_INFO_NR 32
#define FLASH_INFO_FEILD_NR 7

#define FLASH_INFO_BACKUP   10

struct flash_info {
	unsigned int info[FLASH_INFO_FEILD_NR];
	unsigned int crc;
};

int flash_info_image(FILE *in_fp, FILE *out_fp);

#endif /* __FLASH_INFO_H__ */
