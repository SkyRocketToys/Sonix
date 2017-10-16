#ifndef _CMD_AUDIO_H
#define _CMD_AUDIO_H

int cmd_audio_record(int argc, char* argv[]);

int cmd_audio_play(int argc, char* argv[]);

int cmd_audio_twoway(int argc, char* argv[]);

int cmd_audio_codec(int argc, char* argv[]);

#define CMD_TBL_AUDIO		CMD_TBL_ENTRY(		\
	"audio",		5,      NULL,			\
	"+audio		- AUDIO command table",	CFG_DEFAULT_CMD_LEVEL,\
	cmd_audio_tbl,		cmd_main_tbl			\
),

#define CMD_TBL_AUDIO_TWOWAY	CMD_TBL_ENTRY(		\
	"twoway",		6,      cmd_audio_twoway,	\
	"twoway		- audio twoway",	CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL			\
),

#define CMD_TBL_AUDIO_REC	CMD_TBL_ENTRY(		\
	"record",		6,      cmd_audio_record,	\
	"record		- audio record",	CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL			\
),
#define CMD_TBL_AUDIO_PLAY	CMD_TBL_ENTRY(		\
	"play",		4,      cmd_audio_play,	\
	"play		- audio play",	CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL			\
),
                        
#define CMD_TBL_AUDIO_CODEC    CMD_TBL_ENTRY(          \
	"acodec",		6,	cmd_audio_codec,       \
	"acodec		- audio codec enc/dec test",	CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL			\
),

#endif
