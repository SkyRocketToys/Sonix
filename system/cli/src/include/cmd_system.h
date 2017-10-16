#ifndef _CMD_SYSTEM_H_
#define _CMD_SYSTEM_H_

int cmd_system_date(int argc, char* argv[]);
int cmd_system_free(int argc, char* argv[]);
int cmd_system_reboot(int argc, char* argv[]);
int cmd_system_phymem_rw(int argc, char* argv[]);
int cmd_system_dbg_cfg(int argc, char* argv[]);

#define CMD_TBL_SYSTEM    CMD_TBL_ENTRY(          \
	"system",		6,	NULL,       \
	"+system		- Main the system",	CFG_DEFAULT_CMD_LEVEL,\
	cmd_system_tbl,		cmd_main_tbl			\
),

#define CMD_TBL_SYS_DATE    CMD_TBL_ENTRY(          \
	"date",		4,	cmd_system_date,       \
	"date		- Get/Set date",	CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL			\
),

#define CMD_TBL_SYS_FREE    CMD_TBL_ENTRY(          \
	"free",		4,	cmd_system_free,       \
	"free		- Show memory usage",	CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL			\
),

#define CMD_TBL_SYS_REBOOT    CMD_TBL_ENTRY(          \
	"reboot",		6,	cmd_system_reboot,       \
	"reboot		- Reboot system",	CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL			\
),

#define CMD_TBL_SYS_PHYMEM_RW    CMD_TBL_ENTRY(          \
	"phymem_rw",		9,	cmd_system_phymem_rw,       \
	"phymem_rw	- read/write register",	CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL			\
),

#define CMD_TBL_SYS_SET_DBG_PRINT    CMD_TBL_ENTRY(          \
	"dbg_cfg",		7,	cmd_system_dbg_cfg,       \
	"dbg_cfg	- set/get system debug level",	CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL			\
),

#endif
