#ifndef _CMD_UART_H_
#define _CMD_UART_H_

int cmd_uart_status(int argc, char* argv[]);
int cmd_uart2_verify(int argc, char* argv[]);

#define CMD_TBL_UART    CMD_TBL_ENTRY(          \
	"uart",		4,	NULL,       \
	"+uart		- uart table",	CFG_DEFAULT_CMD_LEVEL,\
	cmd_uart_tbl,		cmd_main_tbl			\
),

#define CMD_TBL_UART_STATUS    CMD_TBL_ENTRY(          \
	"status",		6,	cmd_uart_status,       \
	"status		- Show uart status",	CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL			\
),

#define CMD_TBL_UART2_VERIFY    CMD_TBL_ENTRY(          \
	"uart2_verify",		12,	cmd_uart2_verify,       \
	"uart2_verify		- Show uart status",	CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL			\
),

#endif
