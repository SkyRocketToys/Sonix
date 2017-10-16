#ifndef _CMD_RTSP_H_
#define _CMD_RTSP_H_

int cmd_rtsp_start(int argc, char* argv[]);
int cmd_rtsp_stop(int argc, char* argv[]);


#define CMD_TBL_RTSP		CMD_TBL_ENTRY(		\
	"rtsp",	4,      NULL,			\
	"+rtsp		- Rtsp command table",	CFG_DEFAULT_CMD_LEVEL,\
	cmd_rtsp_tbl,		cmd_main_tbl			\
),


#define CMD_TBL_RTSP_START	CMD_TBL_ENTRY(		\
	"start",		5,      cmd_rtsp_start,			\
	"start		- Rtsp command start task ",	CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL			\
),

#define CMD_TBL_RTSP_STOP	CMD_TBL_ENTRY(		\
	"stop",		4,      cmd_rtsp_stop,			\
	"stop		- Rtsp command stop task ",	CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL			\
),



#endif

