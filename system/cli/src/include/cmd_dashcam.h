#ifndef _CMD_DASHCAM_H_
#define _CMD_DASHCAM_H_



int cmd_dashcam_resolution_set(int argc, char* argv[]);
int cmd_dashcam_info_get(int argc, char* argv[]);
int cmd_dashcam_start(int argc, char* argv[]);
int cmd_dashcam_stop(int argc, char* argv[]);
int cmd_dashcam_snapshot(int argc, char* argv[]);
int cmd_dashcam_preview_audio(int argc, char* argv[]);

#define CMD_TBL_DASHCAM    CMD_TBL_ENTRY(          \
	"dashcam",		7,	NULL,       \
	"+dashcam	- Dashcam command table",	CFG_DEFAULT_CMD_LEVEL,\
	cmd_dashcam_tbl,		cmd_main_tbl			\
),

#define CMD_TBL_DASHCAM_RES    CMD_TBL_ENTRY(          \
	"res",		3,	cmd_dashcam_resolution_set,       \
	"res		- set res",	CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL			\
),

#define CMD_TBL_DASHCAM_INFO    CMD_TBL_ENTRY(          \
	"info",		4,	cmd_dashcam_info_get,       \
	"info		- get nvram info ",	CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL			\
),

#define CMD_TBL_DASHCAM_START    CMD_TBL_ENTRY(          \
	"start",		5,	cmd_dashcam_start,       \
	"start		- start dashcam ",	CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL			\
),

#define CMD_TBL_DASHCAM_STOP    CMD_TBL_ENTRY(          \
	"stop",		4,	cmd_dashcam_stop,       \
	"stop		- stop dashcam ",	CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL			\
),

#define CMD_TBL_DASHCAM_SNAPSHOT    CMD_TBL_ENTRY(          \
	"snapshot",		8,	cmd_dashcam_snapshot,       \
	"snapshot	- snapshot ",	CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL			\
),

#define CMD_TBL_DASHCAM_PREVIEW_AUDIO    CMD_TBL_ENTRY(          \
	"prevaudio",		9,	cmd_dashcam_preview_audio,       \
	"prevaudio	- preview audio ",	CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL			\
),


#endif

