#ifndef _CMD_CONFIG_H_
#define _CMD_CONFIG_H_

int cmd_cfg_net(int argc, char* argv[]);
int cmd_cfg_video(int argc, char* argv[]);

#define CMD_TBL_CONFIG    CMD_TBL_ENTRY(          \
	"config",		6,	NULL,       \
	"config		- Main the config",	CFG_DEFAULT_CMD_LEVEL,\
	cmd_config_tbl,		cmd_main_tbl			\
),

#define CMD_TBL_CFG_NET    CMD_TBL_ENTRY(          \
	"net",		3,	cmd_cfg_net,       \
	"net		- net the program",	CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL			\
),

#define CMD_TBL_CFG_VIDEO    CMD_TBL_ENTRY(          \
	"video",		5,	cmd_cfg_video,       \
	"video		- Video the program",	CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL			\
),

#endif
