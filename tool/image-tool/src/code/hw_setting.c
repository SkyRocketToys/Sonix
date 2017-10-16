#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
 
#include "hw_setting.h"

#include "crc.h"
#include "generated/snx_sdk_conf.h"


static int setting_zero(char *p, unsigned char **pp)
{
	int i;
	char *q;
	struct zro_struct *ws;

	ws = (struct zro_struct *)*pp;
	
	ws->tag = ZRO_TAG;

	for(i = 0; i < ZRO_FEILD_NR && (*p); ) {
		if(isspace(*p)) {
			p++;
			continue;
		}

		if (toupper(*p) ==  SEM_TAG_CHAR) {
			p++;
			continue;
		}

		//printf ("(%x)\n",strtoll(p, &q, 0));
		
		ws->data[i++] = strtoll(p, &q, 0);
		p = q;
	}
	
	if(ZRO_FEILD_NR == i) {
		*pp += sizeof(struct zro_struct);
		
		return 0;
	}

	return -1;
}


static int setting_polling(char *p, unsigned char **pp)
{
	int i;
	char *q;
	struct pol_struct *ws;

	ws = (struct pol_struct *)*pp;
	
	ws->tag = POL_TAG;

	for(i = 0; i < POL_FEILD_NR && (*p); ) {
		if(isspace(*p)) {
			p++;
			continue;
		}

		if (toupper(*p) ==  SEM_TAG_CHAR) {
			p++;
			continue;
		}

		//printf ("(%x)\n",strtoll(p, &q, 0));
		
		ws->data[i++] = strtoll(p, &q, 0);
		p = q;
	}
	
	if(POL_FEILD_NR == i) {
		*pp += sizeof(struct pol_struct);
		
		return 0;
	}

	return -1;
}


static int setting_modify(char *p, unsigned char **pp)
{
	int i;
	char *q;
	struct mod_struct *ws;

	ws = (struct mod_struct *)*pp;
	
	ws->tag = MOD_TAG;

	for(i = 0; i < MOD_FEILD_NR && (*p); ) {
		if(isspace(*p)) {
			p++;
			continue;
		}

		if (toupper(*p) ==  SEM_TAG_CHAR) {
			p++;
			continue;
		}

		//printf ("(%x)\n",strtoll(p, &q, 0));
		
		ws->data[i++] = strtoll(p, &q, 0);
		p = q;
	}
	
	if(MOD_FEILD_NR == i) {
		*pp += sizeof(struct mod_struct);
		
		return 0;
	}

	return -1;
}

static int setting_write(char *p, unsigned char **pp)
{
	int i;
	char *q;
	struct write_struct *ws;

	ws = (struct write_struct *)*pp;
	
	ws->tag = WRITE_TAG;

	for(i = 0; i < WRITE_FEILD_NR && (*p); ) {
		if(isspace(*p)) {
			p++;
			continue;
		}

		if (toupper(*p) ==  SEM_TAG_CHAR) {
			p++;
			continue;
		}

		//printf ("(%x)\n",strtoll(p, &q, 0));
		
		ws->data[i++] = strtoll(p, &q, 0);
		p = q;
	}
	
	if(WRITE_FEILD_NR == i) {
		*pp += sizeof(struct write_struct);
		
		return 0;
	}

	return -1;
}
static int setting_bypass(char *p, unsigned char **pp)
{
	int i;
	char *q;
	struct bypass_struct *ds;

	ds = (struct bypass_struct *)*pp;

	for(i = 0; i < BYPASS_FEILD_NR && (*p); ) {
		if(isspace(*p)) {
			p++;
			continue;
		}
		
		ds->data[i++] = strtoll(p, &q, 0);
		p = q;
	}
	
	if(BYPASS_FEILD_NR == i) {
		*pp += sizeof(struct bypass_struct);
		
		return 0;
	}

	return -1;
}

static int setting_delay(char *p, unsigned char **pp)
{
	int i;
	char *q;
	struct delay_struct *ds;

	ds = (struct delay_struct *)*pp;
	
	ds->tag = DELAY_TAG;

	for(i = 0; i < DELAY_FEILD_NR && (*p); ) {
		if(isspace(*p)) {
			p++;
			continue;
		}
		
		ds->data[i++] = strtoll(p, &q, 0);
		p = q;
	}
	
	if(DELAY_FEILD_NR == i) {
		*pp += sizeof(struct delay_struct);
		
		return 0;
	}

	return -1;
}
	

static int setting_parse(char *p, unsigned char **pp)
{	
	while(*p && isspace(*p))
			p++;

	if(*p == '\0')
		return 0;
		
	switch(toupper(*p++)) {
	case COMME_TAG_CHAR:
			return 0;

	case WRITE_TAG_CHAR:
			return setting_write(p, pp);

	case DELAY_TAG_CHAR:
			return setting_delay(p, pp);
	case BYPASS_TAG_CHAR:
			return setting_bypass(p, pp);

	case MOD_TAG_CHAR:
			return setting_modify(p, pp);
	case POL_TAG_CHAR:
			return setting_polling(p, pp);
	case ZRO_TAG_CHAR:
			return setting_zero(p, pp);						

				

			
				
	default:
		fprintf(stderr, "HW_setting format error\n");

		return -1;
	}
}

static int setting_read(FILE *in_fp, unsigned char **pp)
{
	char buf[LINE_MAX_LEN + 1];
	
	while(fgets(buf, LINE_MAX_LEN, in_fp) != NULL) {
		buf[LINE_MAX_LEN] = '\0';

		if(strlen(buf) == 0) {
				printf("-----       ----\n");
				continue;
		}
		
		if(strlen(buf) >= LINE_MAX_LEN) {
			fprintf(stderr, "HW_setting info too long on the line\n");
			return -1;
		}
		
		if(setting_parse(buf, pp)) {
			fprintf(stderr, "HW_setting info format error\n");
			return -1;
		}
	}

	return 0;
}

//#ifndef CONFIG_SYSTEM_PLATFORM_ST58660FPGA
#if defined(CONFIG_SYSTEM_PLATFORM_ST58660FPGA) || defined(CONFIG_SYSTEM_PLATFORM_SN98660) || defined(CONFIG_SYSTEM_PLATFORM_SN98661) || defined (CONFIG_SYSTEM_PLATFORM_SN98670) || defined (CONFIG_SYSTEM_PLATFORM_SN98671) || defined (CONFIG_SYSTEM_PLATFORM_SN98672) || defined (CONFIG_SYSTEM_PLATFORM_SN98293)
int hw_setting_image(FILE *in_fp, FILE * out_fp)
{
	unsigned int size;
	unsigned char *p, *q;
	unsigned int hw_crc;
	
	struct stat stats;

	fstat(fileno(in_fp), &stats);


//	size = stats.st_size + sizeof(PADING) + sizeof(hw_crc);
	size = stats.st_size + sizeof(hw_crc);

	
	p = q = (unsigned char *)malloc(size);
	
	if(!p) {
		fprintf(stderr, "hw_setting: malloc failed\n");
		return -1;
	}

	if(setting_read(in_fp, &q))
		goto out;

//	*((unsigned int *)q) = PADING;
//	q += sizeof(PADING);

	hw_crc = crcSlow(p, (unsigned int)(q - p));

	// *((unsigned short *)q) = hw_crc;
	// q += sizeof(hw_crc);

	fwrite(p, sizeof(char), (unsigned int)(q - p), out_fp);

	free(p);

	return 0;

out:
	free(p);
	
	return -1;
}
#else
int hw_setting_image(FILE *in_fp, FILE * out_fp)
{
	unsigned int size;
	unsigned char *p, *q;
	unsigned int hw_crc;
	
	struct stat stats;

	fstat(fileno(in_fp), &stats);


	size = stats.st_size + sizeof(PADING) + sizeof(hw_crc);

	
	p = q = (unsigned char *)malloc(size);
	
	if(!p) {
		fprintf(stderr, "hw_setting: malloc failed\n");
		return -1;
	}

	if(setting_read(in_fp, &q))
		goto out;

	*((unsigned int *)q) = PADING;
	q += sizeof(PADING);

	hw_crc = crcSlow(p, (unsigned int)(q - p));

	*((unsigned short *)q) = hw_crc;
	q += sizeof(hw_crc);

	fwrite(p, sizeof(char), (unsigned int)(q - p), out_fp);

	free(p);

	return 0;

out:
	free(p);
	
	return -1;
}

#endif
