#ifndef _CMD_SD_H_
#define _CMD_SD_H_


int cmd_sd_read(int argc, char* argv[]);
int cmd_sd_rwtest(int argc, char* argv[]);


#define CMD_TBL_SD    CMD_TBL_ENTRY(          \
	"sd",		2,	NULL,       \
	"+sd		- sd card command table",	CFG_DEFAULT_CMD_LEVEL,\
	cmd_sd_tbl,		cmd_main_tbl			\
),

#define CMD_TBL_SD_READ    CMD_TBL_ENTRY(          \
	"rd",		2,	cmd_sd_read,       \
	"rd		- read sd data",	CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL			\
),

#define CMD_TBL_SD_RWTEST    CMD_TBL_ENTRY(          \
	"rwtest",		6,	cmd_sd_rwtest,       \
	"rwtest		- SD read/write test",	CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL			\
),

#endif
