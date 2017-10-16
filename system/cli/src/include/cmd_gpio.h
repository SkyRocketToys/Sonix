#ifndef _CMD_GPIO_H
#define _CMD_GPIO_H

int cmd_gpio_ctrl(int argc, char* argv[]);
int cmd_pwm_gpio_ctrl(int argc, char* argv[]);
int cmd_spi_gpio_ctrl(int argc, char* argv[]);
int cmd_ms1_gpio_ctrl(int argc, char* argv[]);
int cmd_aud_gpio_ctrl(int argc, char* argv[]);
int cmd_i2c_gpio_ctrl(int argc, char* argv[]);
int cmd_gpio_ctrl_interrupt(int argc, char* argv[]);
int cmd_gpio_ctrl_click(int argc, char* argv[]);
int cmd_uart2_gpio_ctrl(int argc, char* argv[]);
int cmd_jtag_gpio_ctrl(int argc, char* argv[]);
int cmd_totaldev_gpio_ctrl(int argc, char* argv[]);
int cmd_ms2_gpio_ctrl(int argc, char* argv[]);

#define CMD_TBL_GPIO		CMD_TBL_ENTRY(		\
	"gpio",		4,      NULL,			\
	"+gpio		- GPIO command table",	CFG_DEFAULT_CMD_LEVEL,\
	cmd_gpio_tbl,		cmd_main_tbl			\
),

#define CMD_TBL_GPIO_CTRL	CMD_TBL_ENTRY(		\
	"gpio_ctrl",		9,      cmd_gpio_ctrl,	\
	"gpio_ctrl		- GPIO Control",	CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL			\
),

#define CMD_TBL_PWM_GPIO_CTRL	CMD_TBL_ENTRY(		\
	"pwm_gpio_ctrl",		13,      cmd_pwm_gpio_ctrl,	\
	"pwm_gpio_ctrl		- PWM GPIO Control",	CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL			\
),

#define CMD_TBL_SPI_GPIO_CTRL	CMD_TBL_ENTRY(		\
	"spi_gpio_ctrl",		13,      cmd_spi_gpio_ctrl,	\
	"spi_gpio_ctrl		- SPI GPIO Control",	CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL			\
),

#define CMD_TBL_MS1_GPIO_CTRL	CMD_TBL_ENTRY(		\
	"ms1_gpio_ctrl",		13,      cmd_ms1_gpio_ctrl,	\
	"ms1_gpio_ctrl		- MS1 GPIO Control",	CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL			\
),


#define CMD_TBL_AUD_GPIO_CTRL	CMD_TBL_ENTRY(		\
	"aud_gpio_ctrl",		13,      cmd_aud_gpio_ctrl,	\
	"aud_gpio_ctrl		- Audio GPIO Control",	CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL			\
),

#define CMD_TBL_I2C_GPIO_CTRL	CMD_TBL_ENTRY(		\
	"i2c_gpio_ctrl",		13,      cmd_i2c_gpio_ctrl,	\
	"i2c_gpio_ctrl		- I2C GPIO Control",	CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL			\
),

#define CMD_TBL_GPIO_CTRL_INTERRUPT	CMD_TBL_ENTRY(		\
	"gpio_ctrl_interrupt",		19,      cmd_gpio_ctrl_interrupt,	\
	"gpio_ctrl_interrupt		- GPIO Control Interrupt",	CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL			\
),

#define CMD_TBL_GPIO_CTRL_CLICK	CMD_TBL_ENTRY(		\
	"gpio_ctrl_click",		15,      cmd_gpio_ctrl_click,	\
	"gpio_ctrl_click		- GPIO Control Click",	CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL			\
),

#define CMD_TBL_UART2_GPIO_CTRL	CMD_TBL_ENTRY(		\
	"uart2_gpio_ctrl",		15,      cmd_uart2_gpio_ctrl,	\
	"uart2_gpio_ctrl		- UART2 GPIO Control",	CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL			\
),

#define CMD_TBL_JTAG_GPIO_CTRL	CMD_TBL_ENTRY(		\
	"jtag_gpio_ctrl",		14,      cmd_jtag_gpio_ctrl,	\
	"jtag_gpio_ctrl		- JTAG GPIO Control",	CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL			\
),

#define CMD_TBL_TOTALDEV_GPIO_CTRL	CMD_TBL_ENTRY(		\
	"totaldev_gpio_ctrl",		18,      cmd_totaldev_gpio_ctrl,	\
	"totaldev_gpio_ctrl	- TOTALDEV GPIO Control",	CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL			\
),

#define CMD_TBL_MS2_GPIO_CTRL	CMD_TBL_ENTRY(		\
	"ms2_gpio_ctrl",		13,      cmd_ms2_gpio_ctrl,	\
	"ms2_gpio_ctrl		- MS2 GPIO Control",	CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL			\
),

#endif
