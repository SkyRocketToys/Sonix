/**
 * @file
 * this is middleware SD header file, include this file before use
 * @author Algorithm Dept Sonix. (yiling porting to RTOS)
 */
 
#ifndef __SD_LIB_H__ 
#define __SD_LIB_H__ 
#include <sd/sd_ctrl.h>

/**
 * an enum for function return status
 */
typedef enum{
	MID_SD_QUEUE_FINISH = 0,		/**< cmd has finished */
	MID_SD_QUEUE_PROCESSING,	
	MID_SD_QUEUE_ACCESS,			/**< cmd sends to queue and wait for finishing(only in block mode) */
	MID_SD_QUEUE_CARD_REMOVED,		/**< cmd executes failed because of SD card is removed */
	MID_SD_QUEUE_CARD_WRITEPROTECT,		/**< cmd executes failed because of SD card is write protected */
	MID_SD_QUEUE_FAIL			/**< cmd executes failed */
}mid_sd_queue_status_t;

/**
 * an enum for function is block or not
 */
typedef enum{
	MID_SD_NONBLOCK = 0,	/**< function nonblock */
	MID_SD_BLOCK			/**< function block until finish */
}mid_sd_block_t;

void mid_sd_init(void);
int  mid_sd_identify(uint8_t is_block, mid_sd_queue_status_t *status);                //same as mount
int  mid_sd_write(uint32_t dst_addr, void *src_addr, uint32_t size, uint8_t is_block, mid_sd_queue_status_t *status);
int  mid_sd_read(void *dst_addr, uint32_t src_addr, uint32_t size, uint8_t is_block, mid_sd_queue_status_t *status);
int mid_sd_get_capacity(uint32_t *size, uint8_t is_block, mid_sd_queue_status_t *status);
int mid_sd_sync(void);
struct sd_info* mid_sd_get_driver_info(void);
int mid_sd_card_detect(void);
#endif	//__SD_LIB_H__
