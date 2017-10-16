#ifndef _CMD_I2C_H
#define _CMD_I2C_H

int cmd_i2c_ctrl(int argc, char* argv[]);


#define CMD_TBL_I2C		CMD_TBL_ENTRY(		\
	"i2c",		3,      NULL,			\
	"+i2c		- I2C command table",	CFG_DEFAULT_CMD_LEVEL,\
	cmd_i2c_tbl,		cmd_main_tbl			\
),

#define CMD_TBL_I2C_CTRL	CMD_TBL_ENTRY(		\
	"i2c_ctrl",		8,      cmd_i2c_ctrl,	\
	"i2c_ctrl		- I2C Control",	CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL			\
),


#endif
