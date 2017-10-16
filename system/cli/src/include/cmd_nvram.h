#ifndef _CMD_NVRAM_H_
#define _CMD_NVRAM_H_

int cmd_nvram_list_all_name(int argc, char* argv[]);		/**< List all of the NVRAM pack/config name */
int cmd_nvram_set(int argc, char* argv[]);					/**< Set NVRAM Config data */


#define CMD_TBL_NVRAM		CMD_TBL_ENTRY(		\
	"nvram",		5,      NULL,			\
	"+nvram		- NVRAM command table",	CFG_DEFAULT_CMD_LEVEL,\
	cmd_nvram_tbl,		cmd_main_tbl			\
),

#define CMD_TBL_NVRAM_LIST	CMD_TBL_ENTRY(		\
	"list",			4,      cmd_nvram_list_all_name,	\
	"list		- List NVRAM information",	CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL			\
),

#define CMD_TBL_NVRAM_SET	CMD_TBL_ENTRY(		\
	"set",			3,      cmd_nvram_set,	\
	"set		- Set NVRAM data",	CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL			\
),


#endif /* _CMD_NVRAM_H_ */
