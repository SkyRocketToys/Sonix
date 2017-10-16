#include "dev_console_json.h"

#include <stdlib.h>
#include <libmid_fatfs/ff.h>
#include "dev_console.h"

#include <string.h>
#define streq(a,b) (strcmp(a, b) == 0)

#if JSON_DUMP_TO_FILE
FIL json_fptr;
uint8_t dump_to_console;
uint8_t dump_to_file;

void json_dump_init()
{
    dump_to_console = 0;
    dump_to_file = 0;
}
void json_dump_uninit()
{
    f_close(&json_fptr);
}

static const int dump_fh_is_open()
{
    return (json_fptr.fs != 0);
}

static void dump_fh_close()
{
    f_close(&json_fptr);
    json_fptr.fs = 0; // OS doesn't do this if it fails to close...
}

static void json_dump_data_file(const uint8_t *data, uint32_t datalen)
{
    if (!dump_to_file) {
        if (dump_fh_is_open(json_fptr)) {
            console_printf("[dcj] closed: %s\n", JSON_DUMP_FILEPATH);
            dump_fh_close();
        }
        return;
    }
    if (!dump_fh_is_open(json_fptr)) {
        if (f_open(&json_fptr, JSON_DUMP_FILEPATH, FA_CREATE_ALWAYS|FA_WRITE) != FR_OK) {
            console_printf("[dcj] failed to open %s\n", JSON_DUMP_FILEPATH);
            return;
        }
        console_printf("[dcj] opened: %s\n", JSON_DUMP_FILEPATH);
    }

    uint32_t bytes_written;
    if (f_write(&json_fptr, data, datalen, &bytes_written) == FR_OK) {
        if (bytes_written != datalen) {
            // whinge about short write
            console_printf("[dcj] short write\n");
        }
    } else {
        // whinge about failed write
        console_printf("[dcj] failed write\n");
    }
}

static void json_dump_data(const uint8_t *data, uint32_t datalen)
{
    json_dump_data_file(data, datalen);
    if (dump_to_console) {
        console_write(data, datalen);
    }
}

void json_dump_received_data(const uint8_t *data, uint32_t datalen)
{
    const char *header = "\n\nFrom App:\n";
    json_dump_data((const uint8_t *)header, strlen(header));
    json_dump_data(data, datalen);
}

void json_dump_sent_data(const uint8_t *data, uint32_t datalen)
{
    const char *header = "\nTo App:\n";
    json_dump_data((const uint8_t *)header, strlen(header));
    json_dump_data(data, datalen);
}




void cmd_json_help(uint8_t argc, const char *argv[])
{
    console_printf("json sync|help|status|dump\n");
}
void cmd_json_status(uint8_t argc, const char *argv[])
{
    console_printf("dump-to-console: %u\n", dump_to_console);
    console_printf("dump-to-file: %u\n", dump_to_file);
    console_printf("dump-filepath: %s\n", JSON_DUMP_FILEPATH);
}
void cmd_json_sync(uint8_t argc, const char *argv[])
{
    f_sync(&json_fptr);
}
void cmd_json_dump(uint8_t argc, const char *argv[])
{
    const char *usage = "json dump console|file 0|1\n";
    if (argc < 2) {
        console_printf("[dcj] insufficient arguments\n");
        console_printf(usage);
        return;
    }
    const uint8_t val = atoi(argv[1]);
    if (streq(argv[0], "file")) {
        dump_to_file = val;
        if (!val) {
            if (dump_fh_is_open(json_fptr)) {
                console_printf("[dcj] closed: %s\n", JSON_DUMP_FILEPATH);
                dump_fh_close(json_fptr);
            }
        }
    } else if (streq(argv[0], "console")) {
        dump_to_console = val;
    } else {
        console_printf("[dcj] unknown command (%s)\n", argv[0]);
        console_printf(usage);
    }
}


#endif
