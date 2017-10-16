/*
  development hacks by tridge
 */
#pragma once

#include <stdarg.h>
#include <stdint.h>
#include <stdbool.h>

#include <FreeRTOS.h>
#include "semphr.h"

#define FMT_PRINTF(a,b) __attribute__((format(printf, a, b)))

#ifndef bool
typedef int bool;
#endif

#include <i2c/i2c.h>
int I2C_transfer(struct i2c_adapter *adap, struct i2c_msg *msgs, int num);

int dev_console_socket_create(void);

// mutex to only allow one firmware to be upgraded at a time:
xSemaphoreHandle any_fw_upgrade_mutex;

void console_printf(const char *, ...) FMT_PRINTF(1,2);
void console_vprintf(const char *, va_list va);
void console_write(const uint8_t *data, uint32_t len);
bool check_fw_md5(const unsigned char *fw, unsigned fwlen);
bool check_fc_fw_md5(unsigned char *fw, unsigned fwlen, uint32_t *fw_offset);
void upgrade_fc_firmware(const unsigned char *fw, unsigned fwlen, unsigned fw_offset);
void ardupilot_nvram_setup(void);
void wifi_reset(void);

void set_upload_progress(uint8_t percent);
uint8_t get_upload_progress(void);
void set_upload_message(const char *msg);
const char *get_upload_message(void);
bool file_exists(const char *filename);
void mdelay(uint32_t ms);
uint32_t get_time_boot_ms(void);
struct sock_buf;
void get_config_vars_json(struct sock_buf *sock);
const char *get_config_var(const char *varname);

int autobaud_bootloader();

#define CONSOLE_PORT 2017
#define UPLOAD_PORT 2018
#define NC_PORT 2019
#define FC_UPLOAD_PORT 2020
#define TX_UPLOAD_PORT 2021
#define UBLOX_ASSIST_PORT 2022

#define MAX_SN_FW_SIZE 1300000
#define MAX_FC_FW_SIZE 2300000
#define MAX_TX_FW_SIZE 15000

#ifndef __DECONST
#define __DECONST(type, var) ((type)(uintptr_t)(const void *)(var))
#endif

void port_read_then_call(const char *description, int port, uint32_t maxsize, void (*munch)(const uint8_t*, const uint32_t));

enum ov9732_setting {
    OV9732_NORMAL=0,
    OV9732_VIDEO_SLOW=1,
    OV9732_VIDEO_OFF=2
};

void ov9732_set_video(enum ov9732_setting setting);
int ov9732_write(uint16_t regnum, uint8_t count, uint8_t *data);

/*
  return MD5 of a blob as a talloc string
 */
char *md5_string(void *memctx, const uint8_t *data, uint32_t len);

// write a file to microSD
int dev_write_file(const char *path, const uint8_t *data, const uint32_t data_len);
    
// missing prototypes from system headers
void pic_disableInterrupt(uint8_t irq);
int snprintf(char *s, uint32_t n, const char *fmt, ...);


// missing prototype from LwIPConfig.c
int get_local_ip(char *ip, int len);
