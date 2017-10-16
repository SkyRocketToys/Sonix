#ifndef _CMD_RECORD_MP4_H_
#define _CMD_RECORD_MP4_H_

int cmd_record_mp4_start(int argc, char* argv[]);

int cmd_record_mp4_stop(int argc, char* argv[]);

#define CMD_TBL_RECORD_MP4		CMD_TBL_ENTRY(		\
	"record",	6,      NULL,			\
	"+record		- Record mp4 command table",	CFG_DEFAULT_CMD_LEVEL,\
	cmd_record_mp4_tbl,		cmd_main_tbl			\
),


#define CMD_TBL_RECORD_MP4_START	CMD_TBL_ENTRY(		\
	"start",		5,      cmd_record_mp4_start,			\
	"start		- Record mp4 file command start task ",	CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL			\
),

#define CMD_TBL_RECORD_MP4_STOP	CMD_TBL_ENTRY(		\
	"stop",		4,      cmd_record_mp4_stop,			\
	"stop		- Record mp4 file command stop task ",	CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL			\
),

#endif

