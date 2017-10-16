#ifndef _RTOSCLI_H_
#define _RTOSCLI_H_

#define CMD_TBL_ENTRY(name, len_name, cmd, usage, uid_lv, next_lv, prev_lv)  {name, len_name, cmd, usage, uid_lv, next_lv, prev_lv}
#define CMDBUF_SIZE	128
#define PROMPT	"<0>\nSONIX (%s)> "
#define LOGIN_PROMPT	"<0>\nLogin :  "
#define PASSWD_PROMPT	"<0>Password : "

#undef CFG_ENABLE_LOGIN

#define CFG_DEFAULT_CMD_LEVEL		9999

struct cmd_table {
	char	*name;
	int		len_name;
	int 	(*Func)(int argc, char* argv[]);
	char 	*usage;
	int		cmd_lv;
	struct	cmd_table 	*next_lv;
	struct	cmd_table 	*prev_lv;
};

int cmd_quit(void);
int cmd_help(int argc, char* argv[]);
int cmd_back(int argc, char* argv[]);
int cmd_logout(int argc, char* argv[]);

/* Common command */
#define CMD_TBL_LOGOUT    CMD_TBL_ENTRY(          \
	"logout",		6,	cmd_logout,       \
	"logout		- Logout the system",	CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL			\
),

#define CMD_TBL_QUIT    CMD_TBL_ENTRY(          \
	"quit",		4,	cmd_quit,       \
	"quit		- Quit the program",	CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL			\
),

#define CMD_TBL_HELP	CMD_TBL_ENTRY(		\
	"help",		4,	cmd_help,				\
	"help		- Show usage message",	CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL			\
),

#define CMD_TBL_BACK    CMD_TBL_ENTRY(          \
		"back",		4,	cmd_back,       \
		"back            - Back to prev level",  CFG_DEFAULT_CMD_LEVEL,\
		NULL,		NULL                    \
),

#define CMD_TBL_MAIN    CMD_TBL_ENTRY(          \
	"main",		4,	NULL,       \
	"main		- Main the program",	CFG_DEFAULT_CMD_LEVEL,\
	NULL,		NULL			\
),

void init_rtos_cli(void);
int parse_cmd(char *cmd, int isTAB);
void show_cli_prompt(void);
void show_login_prompt(void);
void rtoscli_recv(char ch);
int ispasswd(void);
int islogin(void);

#endif
