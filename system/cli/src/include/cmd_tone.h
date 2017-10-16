#ifndef _CMD_TONE_H
#define _CMD_TONE_H

int cmd_tone_detection(int argc, char* argv[]);
int cmd_tone_raw_data(int argc, char* argv[]);

#define CMD_TBL_TONE		CMD_TBL_ENTRY(		\
	"tone",		4,      NULL,			\
	"+tone		- Audio Tone Detection command table",	CFG_DEFAULT_CMD_LEVEL,\
	cmd_tone_tbl,		cmd_main_tbl			\
),

#define CMD_TBL_TONE_DETECT	CMD_TBL_ENTRY(		\
	"detect",		6,      cmd_tone_detection,	\
	"detect		- tone detect",	CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL			\
),

#define CMD_TBL_TONE_RAW_DATA	CMD_TBL_ENTRY(		\
	"rawdata",		7,      cmd_tone_raw_data,	\
	"rawdata		- read raw data from file",	CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL			\
),

#endif
