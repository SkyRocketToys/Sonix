#ifndef _CMD_MSC_H_
#define _CMD_MSC_H_

int msc_test_item_init(int argc, char* argv[]);
int msc_test_item_chk(int argc, char* argv[]);

#define CMD_TBL_MSC			CMD_TBL_ENTRY(		\
	"msctest",	7,		msc_test_item_init,				\
	"+msctest	- MSC Test command table",	CFG_DEFAULT_CMD_LEVEL,\
	cmd_msc_tbl,		cmd_main_tbl			\
),

#define CMD_TBL_MSC_TEST_TIME	CMD_TBL_ENTRY(		\
	"test_time",	9,      msc_test_item_chk,	\
	"test_time	- msc test time set",	CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL			\
),

#define CMD_TBL_MSC_TEST_FORMAT	CMD_TBL_ENTRY(		\
	"test_format",	11,      msc_test_item_chk,	\
	"test_format	- msc test format set(1:continuous 2:random)",	CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL			\
),

#define CMD_TBL_MSC_TEST_START_POS	CMD_TBL_ENTRY(		\
	"test_start_sec",	14,      msc_test_item_chk,	\
	"test_start_sec	- msc test start sector number",	CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL			\
),

#define CMD_TBL_MSC_TEST_END_POS	CMD_TBL_ENTRY(		\
	"test_end_sec",	12,      msc_test_item_chk,	\
	"test_end_sec	- msc test end sector number",	CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL			\
),

#define CMD_TBL_MSC_TEST_COM_TYPE	CMD_TBL_ENTRY(		\
	"test_com_type",	13,      msc_test_item_chk,	\
	"test_com_type	- msc test compare type(1:R/W compare 2:R/W non-compare 3:R 4:W)",	CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL			\
),

#define CMD_TBL_MSC_TEST_CLR	CMD_TBL_ENTRY(		\
	"test_clear",	10,      msc_test_item_chk,	\
	"test_clear	- clear setted params(back to default)",	CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL			\
),

#define CMD_TBL_MSC_DEV_INFO	CMD_TBL_ENTRY(		\
	"device_info",	11,      msc_test_item_chk,	\
	"device_info	- msc device info",	CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL			\
),

#define CMD_TBL_MSC_TEST_PARAMS	CMD_TBL_ENTRY(		\
	"test_params",	11,      msc_test_item_chk,	\
	"test_params	- msc setted params",	CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL			\
),

#define CMD_TBL_MSC_TEST_EXE	CMD_TBL_ENTRY(		\
	"test_exe",	8,      msc_test_item_chk,	\
	"test_exe	- execute test",	CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL			\
),


#endif /* SYSTEM_CLI_SRC_INCLUDE_CMD_MSC_H_ */
