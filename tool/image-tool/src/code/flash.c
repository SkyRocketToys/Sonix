#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "crc.h"
#include "flash.h"

static int    flash_info_cnt = 0;
static struct flash_info flash[FLASH_INFO_NR];

static int flash_info_parse(char *line, struct flash_info *finfo)
{
	int i;
	char *p, *q;

	p = line;
	
	for(i = 0; i < FLASH_INFO_NR && (*p); ) {
		if(isspace(*p)) {
			p++;
			continue;
		}
		
		finfo->info[i++] = strtoll(p, &q, 0);
		p = q;
	}
	
	if(FLASH_INFO_FEILD_NR == i)
		return 0;

	return -1;
}

static void flash_info_crc(struct flash_info *finfo)
{
	finfo->crc = crcSlow((unsigned char *) (&finfo->info), sizeof(finfo->info));
}

static int flash_info_read(FILE *in_fp)
{
	char *p;
	char buf[LINE_MAX_LEN + 1];
	
	while(fgets(buf, LINE_MAX_LEN, in_fp) != NULL) {
		buf[LINE_MAX_LEN] = '\0';

		p = buf;
		while((*p) && isspace(*p))
			p++;

		if((*p) == '#' || (*p) == '\0')
			continue;

		if(strlen(buf) >= LINE_MAX_LEN) {
			fprintf(stderr, "flash info too long on the line\n");
			return -1;
		}

		if(flash_info_cnt >= FLASH_INFO_NR) {
			fprintf(stderr, "flash info count > %d\n", FLASH_INFO_NR);
			return -1;
		}

		if(flash_info_parse(buf, &flash[flash_info_cnt])) {
			fprintf(stderr, "flash info format error\n");
			return -1;
		}

		flash_info_crc(&flash[flash_info_cnt]);

		flash_info_cnt++;
	}

	return 0;
}
				
int flash_info_image(FILE *in_fp, FILE *out_fp)
{
	int i;

	if(flash_info_read(in_fp))
		return -1;
	
	for(i = 0; i < flash_info_cnt; i++) {
		fwrite(&flash[i].info, sizeof(flash[i].info), 1, out_fp);
		fwrite(&flash[i].crc,  sizeof(flash[i].crc),  1, out_fp);
	}

	return 0;
}
