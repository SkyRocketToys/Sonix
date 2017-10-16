#ifndef __HW_SETTING_H__
#define __HW_SETTING_H__

#include <stdio.h>

#define WRITE_FEILD_NR 2
#define DELAY_FEILD_NR 1
#define BYPASS_FEILD_NR 1

#define MOD_FEILD_NR 3
#define POL_FEILD_NR 4
#define ZRO_FEILD_NR 2

struct write_struct {
	unsigned int tag;
	unsigned int data[WRITE_FEILD_NR];
};

struct delay_struct {
	unsigned int tag;
	unsigned int data[DELAY_FEILD_NR];
};

struct bypass_struct {
	unsigned int data[DELAY_FEILD_NR];
};


struct mod_struct {
	unsigned int tag;
	unsigned int data[MOD_FEILD_NR];
};

struct pol_struct {
	unsigned int tag;
	unsigned int data[POL_FEILD_NR];
};


struct zro_struct {
	unsigned int tag;
	unsigned int data[ZRO_FEILD_NR];
};


#define LINE_MAX_LEN 128

#define WRITE_TAG_CHAR 'W'
#define DELAY_TAG_CHAR 'N'
#define COMME_TAG_CHAR '#'
#define SEM_TAG_CHAR ','
#define BYPASS_TAG_CHAR 'B'

#define MOD_TAG_CHAR 'M'
#define POL_TAG_CHAR 'P'
#define ZRO_TAG_CHAR 'Z'


#define WRITE_TAG 0x00000077
#define DELAY_TAG 0x0000006E
#define BYPASS_TAG 0x00000042

#define MOD_TAG 0x0000006D
#define POL_TAG 0x00000070
#define ZRO_TAG 0x0000007A




#define PADING 0XFFFFFFFF

int hw_setting_image(FILE *in_fp, FILE * out_fp);

#endif /* __HW_SETTING_H__ */
