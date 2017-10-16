#ifndef _CMD_VIDEO_H_
#define _CMD_VIDEO_H_

#include <libmid_fatfs/ff.h>
struct snx_temp {
	int total_size;
	FIL video_file;

	unsigned char *ptr;
	unsigned char *buf;

};

struct snx_app {
	char name[16];
	int dup;
	int mode;
	struct snx_m2m *m2m;
};


int cmd_video_vc_set(int argc, char* argv[]);
int cmd_video_rc_set(int argc, char* argv[]);

int cmd_video_ds_set(int argc, char* argv[]);
int cmd_video_mroi_set(int argc, char* argv[]);

#define CMD_TBL_VIDEO		CMD_TBL_ENTRY(		\
	"video",	5,      NULL,			\
	"+video		- Video command table",	CFG_DEFAULT_CMD_LEVEL,\
	cmd_video_tbl,		cmd_main_tbl			\
),


#define CMD_TBL_VIDEO_VC_SET	CMD_TBL_ENTRY(		\
	"vcset",	5,      cmd_video_vc_set,			\
	"vcset		- Video command get/set video parameter",	CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL			\
),

#define CMD_TBL_VIDEO_RC_SET	CMD_TBL_ENTRY(		\
	"rcset",	5,      cmd_video_rc_set,			\
	"rcset		- Video command get/set rate control parameter",	CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL			\
),

#define CMD_TBL_VIDEO_DS_SET	CMD_TBL_ENTRY(		\
	"dsset",		5,      cmd_video_ds_set,			\
	"dsset		- Video command set data stamp parameter", CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL			\
),

#define CMD_TBL_VIDEO_MROI_SET	CMD_TBL_ENTRY(		\
	"mroiset",		7,      cmd_video_mroi_set,			\
	"mroiset	- Video command set mroi parameter", CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL			\
),

#endif

