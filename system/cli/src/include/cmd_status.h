#ifndef _CMD_STATUS_H_
#define _CMD_STATUS_H_

int cmd_stat_sysinfo(int argc, char* argv[]);
int cmd_stat_cpu(int argc, char* argv[]);
int cmd_stat_intr(int argc, char* argv[]);

#define CMD_TBL_STATUS    CMD_TBL_ENTRY(          \
	"status",		6,	NULL,       \
	"+status		- show status",	CFG_DEFAULT_CMD_LEVEL,\
	cmd_status_tbl,		cmd_main_tbl			\
),

#define CMD_TBL_STAT_SYSINFO    CMD_TBL_ENTRY(          \
	"sysinfo",		7,	cmd_stat_sysinfo,       \
	"sysinfo		- show the basic system information",	CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL			\
),

#define CMD_TBL_STAT_CPU    CMD_TBL_ENTRY(          \
	"cpu",		3,	cmd_stat_cpu,       \
	"cpu		- show cpu status",	CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL			\
),

#if (defined(CONFIG_APP_DASHCAM) && CONFIG_APP_DASHCAM) || \
	(defined(CONFIG_APP_IPCAM) && CONFIG_APP_IPCAM)|| \
	(defined(CONFIG_APP_INTEGRATION) && CONFIG_APP_INTEGRATION)
#define CMD_TBL_STAT_INTR    CMD_TBL_ENTRY(          \
	"intr",		4,	cmd_stat_intr,       \
	"intr		- show interrupt counter status",	CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL			\
),
#else
#define CMD_TBL_STAT_INTR
#endif

#ifdef CONFIG_CLI_CMD_STATUS_MCU
int cmd_stat_mcu(int argc, char* argv[]);
#define CMD_TBL_STAT_MCU    CMD_TBL_ENTRY(          \
	"mcu",		3,	cmd_stat_mcu,       \
	"mcu		- show mcu counter status",	CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL			\
),
#else
#define CMD_TBL_STAT_MCU
#endif

#endif
