#ifndef _FWUPGRADE__H_
#define _FWUPGRADE__H_

#include <FreeRTOS.h>
#include <bsp.h>
#include <task.h>
#include <queue.h>
#include <semphr.h>
#include <nonstdlib.h>
#include <string.h>

#define uchar unsigned char 
#define uint unsigned int

#define BYPASS_TAG      "BYPASS_MODE"

typedef struct { 
   uchar data[64]; 
   uint datalen; 
   uint bitlen[2]; 
   uint state[4]; 
} MD5_CTX;

typedef struct _fwburning_info{
    xSemaphoreHandle FwburningMutex;
 }fwburning_info_t;
 
 enum FW_PRECHECK_ERR_CODE {
	FW_PRECHECK_OK = 0,
	FW_PRECHECK_ERR_MALLOC_FAIL,
	FW_PRECHECK_ERR_PLATFORM_DISMATCH,
	FW_PRECHECK_ERR_VERSION,
	FW_PRECHECK_ERR_MD5
 };

int fw_upgrade(void *addr, unsigned int size);
int fw_upgrade_precheck(void *addr, unsigned int size);

void md5_init(MD5_CTX *ctx);
void md5_update(MD5_CTX *ctx, uchar data[], uint len);
void md5_final(MD5_CTX *ctx, uchar hash[]);

int snx_endecrypt(char *bef_str, char *afr_str);
int reboot(void);
fwburning_info_t* get_fwburning_info(void);

int wdt_keepalive_init(unsigned int wdt_sec);
int check_platform_match(unsigned char * addr, unsigned int fszie);
int check_bypass_mode(unsigned char * addr, unsigned int fszie);
#endif
