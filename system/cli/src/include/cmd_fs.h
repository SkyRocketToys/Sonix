#ifndef _CMD_FS_H_
#define _CMD_FS_H_


int cmd_fs_mount(int argc, char* argv[]);
int cmd_fs_umount(int argc, char* argv[]);
int cmd_fs_ls(int argc, char* argv[]);
int cmd_fs_pwd(int argc, char* argv[]);
int cmd_fs_cd(int argc, char* argv[]);
int cmd_fs_mkdir(int argc, char* argv[]);
int cmd_fs_rm(int argc, char* argv[]);
int cmd_fs_du(int argc, char* argv[]);
int cmd_fs_write_file(int argc, char* argv[]);
int cmd_fs_format(int argc, char* argv[]);



#define CMD_TBL_FS    CMD_TBL_ENTRY(          \
	"fs",		2,	NULL,       \
	"+fs		- file system command table",	CFG_DEFAULT_CMD_LEVEL,\
	cmd_fs_tbl,		cmd_main_tbl			\
),

#define CMD_TBL_FS_MOUNT    CMD_TBL_ENTRY(          \
	"mount",		5,	cmd_fs_mount,       \
	"mount		- mount a filesystem",	CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL			\
),

#define CMD_TBL_FS_UMOUNT    CMD_TBL_ENTRY(          \
	"umount",		6,	cmd_fs_umount,       \
	"umount		- unmount file systems",	CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL			\
),


#define CMD_TBL_FS_LS    CMD_TBL_ENTRY(          \
	"ls",		2,	cmd_fs_ls,       \
	"ls		- list directory contents",	CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL			\
),

#define CMD_TBL_FS_PWD    CMD_TBL_ENTRY(          \
	"pwd",		3,	cmd_fs_pwd,       \
	"pwd		- print name of current working directory",	CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL			\
),

#define CMD_TBL_FS_CD    CMD_TBL_ENTRY(          \
	"cd",		2,	cmd_fs_cd,       \
	"cd		- change directory",	CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL			\
),

#define CMD_TBL_FS_MKDIR    CMD_TBL_ENTRY(          \
	"mkdir",		5,	cmd_fs_mkdir,       \
	"mkdir		- make directory",	CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL			\
),

#define CMD_TBL_FS_RM    CMD_TBL_ENTRY(          \
	"rm",		2,	cmd_fs_rm,       \
	"rm		- remove file or directory",	CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL			\
),
	
#define CMD_TBL_FS_DU    CMD_TBL_ENTRY(          \
		"du",		2,	cmd_fs_du,		 \
		"du		- estimate file space usage", CFG_DEFAULT_CMD_LEVEL,\
		NULL,		NULL			\
	),

#define CMD_TBL_FS_WF    CMD_TBL_ENTRY(          \
		"wf",		2,	cmd_fs_write_file,		 \
		"wf		- write memory to file", CFG_DEFAULT_CMD_LEVEL,\
		NULL,		NULL			\
	),

#define CMD_TBL_FS_FMT    CMD_TBL_ENTRY(          \
	"format",		6,	cmd_fs_format,       \
	"format		- format sd",	CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL			\
),


#define CMD_TBL_FS_SDRD    CMD_TBL_ENTRY(          \
	"sdrd",		4,	cmd_sd_read,       \
	"sdrd		- read sd data",	CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL			\
),


#endif
