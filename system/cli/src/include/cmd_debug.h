#ifndef _CMD_DEBUG_H_
#define _CMD_DEBUG_H_

int cmd_dbg_memory(int argc, char* argv[]);
int cmd_dbg_cmd01(int argc, char* argv[]);
int cmd_dbg_cmd02(int argc, char* argv[]);
int cmd_dbg_trace(int argc, char* argv[]);
int cmd_dbg_audio(int argc, char* argv[]);

#define CMD_TBL_DBG_MEMORY    CMD_TBL_ENTRY(          \
	"memtest",		7,	cmd_dbg_memory,       \
	"memtest		- Memory test",	CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL			\
),

#define CMD_TBL_DEBUG    CMD_TBL_ENTRY(          \
	"debug",		5,	NULL,       \
	"+debug		- Main the debug",	CFG_DEFAULT_CMD_LEVEL,\
	cmd_debug_tbl,		cmd_main_tbl			\
),

#define CMD_TBL_DBG_CMD01    CMD_TBL_ENTRY(          \
	"cmd01",		5,	cmd_dbg_cmd01,       \
	"cmd01		- Debug command",	CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL			\
),

#define CMD_TBL_DBG_CMD02    CMD_TBL_ENTRY(          \
	"cmd02",		5,	cmd_dbg_cmd02,       \
	"cmd02		- Debug command",	CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL			\
),

#define CMD_TBL_DBG_TRACE	CMD_TBL_ENTRY(		\
	"trace",		5,      cmd_dbg_trace,			\
	"trace		- trace command table, 0:stop 1:start",	CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL			\
),

#define CMD_TBL_DBG_AUDIO CMD_TBL_ENTRY(		\
	"audio",	5,      cmd_dbg_audio,			\
	"audio		- cap/ply, format, rate, buffer size, buffer threshold, block",	CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL			\
),

#endif
