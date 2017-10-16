#include <generated/snx_sdk_conf.h>
#include <stdio.h>
#include "rtoscli.h"
#include "cmd.h"
#include "schema.h"
#include "printlog.h"
#include <nonstdlib.h>
#include <string.h>

#ifdef CFG_ENABLE_LOGIN
static int login = 0;
static unsigned int login_uid = 0xffffffff;
#else
static int login = 1;
static unsigned int login_uid = 0;
#endif
static int user_flag = 0;
static int passwd_flag = 0;

int ispasswd()
{
	return passwd_flag;
}

int islogin()
{
	return login;
}

int cmd_quit(void)
{
	//printf("quit\n");

	//exit(0);
	return 0;
}

int cmd_logout(int argc, char* argv[])
{
	login = 0;
	user_flag = 0;
	passwd_flag = 0;
	login_uid = 0xffffffff;
	curr_lv = cmd_main_tbl;

	return 0;
}
int cmd_help(int argc, char* argv[])
{
	int i = 1;

	while (curr_lv[i].usage) {
		if (login_uid <= curr_lv[i].cmd_lv) {
			print_msg_queue("<0>%s\n", curr_lv[i].usage);
		}
		i++;
	}

	return 0;
}

int cmd_back(int argc, char* argv[])
{
	if (curr_lv[0].prev_lv) {
		curr_lv = curr_lv[0].prev_lv;
	}

	cmd_help(0, NULL);

	/* cmd_back return a special value */
	return 99;
}

int tok_cmd(char *argument, char *array_arg[16])
{
	int num_arg = 0;
	char *delim=" ";

	array_arg[0] = strtok(argument,delim);

	for (num_arg = 1 ; num_arg < 16 ; num_arg++) {
		if (!(array_arg[num_arg] = strtok(NULL,delim))){
			break;
		}
	}

	return num_arg;
}

int rtoscli_login(char *user_pw)
{
	int ret = 0;

	if (passwd_flag == 0) {
		if (!strncmp("admin", user_pw, 5)) {
			// Check
			user_flag = 1;
		} else {
			user_flag = 0;
		}

		passwd_flag = 1;
	} else {
		// Password check
		if (!strncmp("1234", user_pw, 4) && user_flag) {
			login = 1;
			login_uid = 0;	// Test for admin
		} else {
			ret = -1;
			login = 0;
		}
		user_flag = 0;
		passwd_flag = 0;
	}

	return ret;
}

int parse_cmd(char *cmd, int isTAB)
{
	int i = 1;
	int ret = 0;
	int found = 0;
	//int only = 0, tmp_ind = 0;
	//int x = 0;
	char *pargv[16];
	int num_argc = 0;

	if (strlen(cmd) == 0)
		return 0;

	//memset(search_tmp, 0, 128);

#ifdef CFG_ENABLE_LOGIN
	if (!islogin()) {
		rtoscli_login(cmd);

		return 0;
	}
#endif

	num_argc = tok_cmd(cmd, pargv);

	while (curr_lv[i].name != NULL) {
		if ((login_uid <= curr_lv[i].cmd_lv) && (curr_lv[i].len_name == strlen(pargv[0])))
			found = strncmp(curr_lv[i].name, pargv[0], curr_lv[i].len_name);
		else
			found = 1;	/* not found */

		if (!found) {
			if (curr_lv[i].Func != NULL) {
				//num_argc = tok_cmd(cmd, pargv);
				ret = curr_lv[i].Func(num_argc, pargv);
			}

			if (curr_lv[i].next_lv && ret != 99) {
				curr_lv = curr_lv[i].next_lv;
				cmd_help(0, NULL);
			}

			break;
		}
		i++;
	}

	if (found != 0) {
		ret = 0;
		print_msg_queue("Command not found\n");
		cmd_help(0, NULL);
	} else {
		ret = 1;
	}

	return ret;
}

void init_rtos_cli()
{
	curr_lv = cmd_main_tbl;
#ifdef CFG_ENABLE_LOGIN
	print_msg(LOGIN_PROMPT);
#else
	print_msg(PROMPT, curr_lv[0].name);
#endif
}

void show_cli_prompt()
{
	if (islogin()) {
		print_msg_queue(PROMPT, curr_lv[0].name);

		return;
	}

	if (ispasswd()){
		print_msg_queue(PASSWD_PROMPT);
	} else {
		print_msg_queue(LOGIN_PROMPT);
	}

	return;
}
