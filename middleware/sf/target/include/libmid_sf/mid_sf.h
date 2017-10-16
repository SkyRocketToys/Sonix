/**
 * @file
 * this is middleware serial flash header file, include this file before use
 * @author Algorithm Dept Sonix. (yiling porting to RTOS)
 */


#ifndef __SF_LIB_H__ 
#define __SF_LIB_H__ 

/**
 * an enum for function return status
 */
typedef enum{
	MID_SF_QUEUE_PROCESSING = 0,	
	MID_SF_QUEUE_ACCESS,
	MID_SF_QUEUE_FINISH,			/**< cmd has finished */
	MID_SF_QUEUE_FAIL				/**< cmd executed fail */
}mid_sf_queue_status_t;

/*
typedef enum{
        MID_SF_NONBLOCK = 0,
        MID_SF_BLOCK
}mid_sf_block_t;
uint8_t sf_mid_block(sf_block_t enable);
*/

typedef struct mid_sf_capacity{
	char *name;
	uint32_t page_size;    /* unit: bytes, set to 0x0 if not support page program */
	uint32_t sector_size;  /* unit: bytes, set to 0x0 if not support sector erase */
	uint32_t block_size;   /* unit: bytes, set to 0x0 if not support block erase  */
	uint32_t chip_size;    /* unit: bytes */

}mid_sf_capacity_t;

void mid_sf_init(void);
int mid_sf_capacity(mid_sf_capacity_t *sf_cap);
int mid_sf_chip_erase(mid_sf_queue_status_t *status);
int mid_sf_sector_erase(uint32_t addr, size_t size, mid_sf_queue_status_t *status);
int mid_sf_write(uint32_t dst_addr, void *src_addr, size_t size, mid_sf_queue_status_t *status);
int mid_sf_read(void *dst_addr, uint32_t src_addr, size_t size, mid_sf_queue_status_t *status);

//only support for nvram before scheduler start
int mid_sf_chip_erase_direct(void);
int mid_sf_sector_erase_direct(uint32_t addr, size_t size);
int mid_sf_write_direct(uint32_t dst_addr, void *src_addr, size_t size);
int mid_sf_read_direct(void *dst_addr, uint32_t src_addr, size_t size);


#endif	//__SF_LIB_H__ 
