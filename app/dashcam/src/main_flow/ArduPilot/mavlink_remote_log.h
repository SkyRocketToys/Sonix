#pragma once

#include "web_server/mavlink_core.h"

#define REMOTE_LOG_BASEDIR "/DATAFLASH"

void mavlink_handle_remote_log_data_block(mavlink_remote_log_data_block_t *msg);
void mavlink_remote_log_periodic();
void mavlink_remote_log_sync(bool force);
void cmd_rl_status(unsigned argc, const char *argv[]);
void cmd_rl_disable(unsigned argc, const char *argv[]);
void cmd_rl_enable(unsigned argc, const char *argv[]);
void cmd_rl_help(unsigned argc, const char *argv[]);
