#ifndef _CMD_PWM_H
#define _CMD_PWM_H

int cmd_pwm_ctrl(int argc, char* argv[]);


#define CMD_TBL_PWM		CMD_TBL_ENTRY(		\
	"pwm",		3,      NULL,			\
	"+pwm		- PWM command table",	CFG_DEFAULT_CMD_LEVEL,\
	cmd_pwm_tbl,		cmd_main_tbl			\
),

#define CMD_TBL_PWM_CTRL	CMD_TBL_ENTRY(		\
	"pwm_ctrl",		8,      cmd_pwm_ctrl,	\
	"pwm_ctrl		- PWM Control",	CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL			\
),


#endif
