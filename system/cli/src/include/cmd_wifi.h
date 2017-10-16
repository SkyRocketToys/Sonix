#ifndef _CMD_WIFI_H
#define _CMD_WIFI_H

int cmd_wifi_ctrl(int argc, char * argv[]);
int cmd_wifi_ap_ctrl(int argc, char * argv[]);
int cmd_wifi_sta_set(int argc, char * argv[]);
int cmd_wifi_info(int argc, char * argv[]);


#define CMD_TBL_WIFI		CMD_TBL_ENTRY(						\
	"wifi",		4,      NULL,									\
	"+wifi		- WIFI command table",	CFG_DEFAULT_CMD_LEVEL,	\
	cmd_wifi_tbl,		cmd_main_tbl							\
),

#define CMD_TBL_WIFI_CTRL	CMD_TBL_ENTRY(						\
	"wifi_ctrl", 9,      cmd_wifi_ctrl,							\
	"wifi_ctrl	- WIFI Control",	CFG_DEFAULT_CMD_LEVEL,		\
	NULL,		NULL											\
),

#define CMD_TBL_WIFI_AP		CMD_TBL_ENTRY(						\
	"ap_ctrl", 	7,      cmd_wifi_ap_ctrl,						\
	"ap_ctrl		- AP mode config",	CFG_DEFAULT_CMD_LEVEL,		\
	NULL,		NULL											\
),

#define CMD_TBL_WIFI_STA	CMD_TBL_ENTRY(						\
	"sta_set",	7,      cmd_wifi_sta_set,						\
	"sta_set 	- sStation mode config",	CFG_DEFAULT_CMD_LEVEL,	\
	NULL,		NULL											\
),

#define CMD_TBL_WIFI_INFO	CMD_TBL_ENTRY(						\
	"wifi_info",	9,      cmd_wifi_info,						\
	"wifi_info 	- Get entry information",	CFG_DEFAULT_CMD_LEVEL,	\
	NULL,		NULL											\
),


	
#endif
