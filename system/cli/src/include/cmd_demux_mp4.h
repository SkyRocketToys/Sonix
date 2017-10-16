#ifndef _CMD_DEMUX_MP4_H_
#define _CMD_DEMUX_MP4_H_

int cmd_demux_mp4_start(int argc, char* argv[]);



#define CMD_TBL_DEMUX_MP4		CMD_TBL_ENTRY(		\
	"demux",	5,      NULL,			\
	"+demux		- Demux mp4 command table",	CFG_DEFAULT_CMD_LEVEL,\
	cmd_demux_mp4_tbl,		cmd_main_tbl			\
),


#define CMD_TBL_DEMUX_MP4_START	CMD_TBL_ENTRY(		\
	"start",		5,      cmd_demux_mp4_start,			\
	"start		- Demux mp4 file command start task ",	CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL			\
),


#endif

