
#ifndef _CMD_ISP_H_
#define _CMD_ISP_H_


int cmd_isp_capture(int argc, char* argv[]);
int cmd_isp_echo(int argc, char* argv[]);
int cmd_isp_cat(int argc, char* argv[]);



#define CMD_TBL_ISP		CMD_TBL_ENTRY(		\
	"isp",	3,      NULL,			\
	"isp		- isp command table",	CFG_DEFAULT_CMD_LEVEL,\
	cmd_isp_tbl,		cmd_main_tbl			\
),

#define CMD_TBL_ISP_CAPTURE	CMD_TBL_ENTRY(		\
	"capture",		7,      cmd_isp_capture,			\
	"capture		- isp command capture\n \
			example: capture 1280 720 30", CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL			\
),


#define CMD_TBL_ISP_ECHO	CMD_TBL_ENTRY(		\
	"echo",		4,      cmd_isp_echo,			\
	"echo		- isp command write value into proc interface\n \
			example: echo 0 /proc/isp/ae/enable", CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL			\
),

#define CMD_TBL_ISP_CAT	CMD_TBL_ENTRY(		\
	"cat",		3,      cmd_isp_cat,			\
	"cat		- isp command read value from proc interface\n \
			example: cat /proc/isp/ae/enable", CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL			\
),

#endif
