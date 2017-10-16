#pragma once

#define JSON_DUMP_TO_FILE 1
#define JSON_DUMP_FILEPATH "/JSONDUMP.TXT"

#include <stdint.h>

void json_dump_init();
void json_dump_uninit();
void json_dump_received_data(const uint8_t *data, uint32_t datalen);

void cmd_json_sync(uint8_t argc, const char *argv[]);
void cmd_json_status(uint8_t argc, const char *argv[]);
void cmd_json_help(uint8_t argc, const char *argv[]);
void cmd_json_dump(uint8_t argc, const char *argv[]);
