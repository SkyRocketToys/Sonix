#include <stdio.h>
#include "cmd_config.h"
#include <nonstdlib.h>

int cmd_cfg_net(int argc, char* argv[])
{
	print_msg_queue("cfg net command %d %s\n", argc, argv[1]);

	return 0;
}

int cmd_cfg_video(int argc, char* argv[])
{
	print_msg_queue("cfg video command %d %s\n", argc, argv[1]);

	return 0;
}
