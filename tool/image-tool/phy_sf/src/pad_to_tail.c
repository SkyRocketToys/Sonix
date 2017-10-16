/*
* function:  pad file to 16m
*
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include<ctype.h>
#include "generated/snx_sdk_conf.h"

int pad_to_tail(int out_fd, char *pad_type2_value)
{
	unsigned int size_out;
	unsigned char *p, *q;
	unsigned int pad = 0;
	unsigned char padvalue_type2 = 0;
	unsigned int size_16m_ui = 0;
	char *size_flash = NULL;

#ifdef CONFIG_SYSTEM_SERIAL_FLASH_16M
	size_flash = "0x1000000";
#endif
#ifdef CONFIG_SYSTEM_SERIAL_FLASH_8M
	size_flash = "0x800000";
#endif	
#ifdef CONFIG_SYSTEM_SERIAL_FLASH_4M
	size_flash = "0x400000";
#endif
#ifdef CONFIG_SYSTEM_SERIAL_FLASH_2M
	size_flash = "0x200000";
#endif
	struct stat stats_out;
	int check = 0;
	
	fstat(out_fd, &stats_out);
	size_out = stats_out.st_size;
	
	size_16m_ui =  strtoll(size_flash, NULL, 0);
	padvalue_type2 = (unsigned char) strtoll(pad_type2_value, NULL, 0);
	
	/**
   	 * make serial image to 16m
   	 */
	/*pad in front*/
	check = size_16m_ui - size_out;
	if(check < 0){
		fprintf(stderr, "flash layout error\n");
		return -1;
	}
	pad = size_16m_ui - size_out;
	if(pad > 0){
		q = p = (unsigned char *)malloc(pad);
		if(!q) {
			fprintf(stderr, "malloc failed\n");
			return -1;
		}

		memset(q,padvalue_type2,pad);
		
		write(out_fd, p, pad);
	
		free(p);
	}


	return 0;
}
