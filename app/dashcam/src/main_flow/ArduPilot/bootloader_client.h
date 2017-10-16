/*
 * basic library for uploading firmware via a serial port to an STM32
 * running Bootloader software
 * (https://github.com/arudpilot/Bootloader)
 */

#ifndef _BOOTLOADER_CLIENT_H
#define _BOOTLOADER_CLIENT_H

#include <stdint.h>
#include <libmid_fatfs/ff.h>

#include <queue.h>

#include <unistd.h>

#include <stdarg.h>
#include <stddef.h>

typedef uint32_t bl_timeout_t;

/*
 * higher-level functions
 */


/*
 * attempt to reboot the STM32 by sending a mavlink reboot
 */
int bootloader_send_mavlink_reboot();

/*
 * nops are ignored by the bootloader
 */
int bootloader_send_nop();

/*
 * attempt to synchronise with bootloader
 */
int bootloader_get_sync();

/*
 * exit bootloader and attempt to run app
 */
int bootloader_boot();

/*
 * return bootloader revision
 */
int bootloader_get_bl_rev(uint32_t *ret);

/*
 * return board id
 */
int bootloader_get_board_id(uint32_t *ret);

/*
 * return board revision
 */
int bootloader_get_board_rev(uint32_t *ret);

/*
 * return size of flash firm area
 */
int bootloader_get_fw_size(uint32_t *ret);

/*
 * return boot vectors
 * ret must be 4*uint32 long
 *
 */
int bootloader_get_vec_area(uint32_t *ret);

/*
 * return STM Chip ID
 */
int bootloader_get_chip_id(uint32_t *ret);

/*
 * retrieve serial number from processor
 * ret must be d*uint32_t bytes in size
 */
int bootloader_get_sn(uint32_t *ret);

/*
 * retrieve One-Time-Programmable bytes from processor;
 * ret must be 512 bytes in size
 */
int bootloader_get_otp(uint8_t *ret);

/*
 * get textual description of chip
 */
int bootloader_get_chip_des(uint8_t *ret, const uint32_t retlen);

/*
 * retrieve crc of programmed flash area
 */
int bootloader_get_crc(uint32_t *ret);

/*
 * send erase command to bootloader
 *
 * Caller must wait for sync using bootloader_recv_sync(); it may take
 * 10+ seconds to complete an erase
 */
int bootloader_erase_send();

/*
 * bootloader waits d seconds before launching app
 */
int bootloader_set_boot_delay(const uint8_t d);

/*
 * write fw of length fwlen to flash storage
 *
 * f is called periodically, its argument being percentage-complete
 * chip must be erased before calling this funtion
 */
int bootloader_program(uint8_t *fw, const uint32_t fwlen, void (*f)(uint8_t));

/*
 * calculate a CRC for the supplied firmware
 * this should match what the PixHawk returns after flashing is complete
 */
uint32_t bootloader_crc32_firmware(const uint8_t *fw, uint32_t fwlen, const uint32_t padlen);


/*
 * set a baudrate on the bootloader side of the serial connection
 */
int bootloader_set_baudrate(uint32_t baud);


/*
 * lower-level functions
 */


/*
 * these two functions must be implemented by the user of this library:
 */
ssize_t bootloader_write(const uint8_t *bytes, const size_t bytecount, const bl_timeout_t timeout);
ssize_t bootloader_readbyte(uint8_t *byte, const bl_timeout_t timeout);
#ifdef __STDC__
void bootloader_debug(const char  *format, ...);
#else
void bootloader_debug(const char  *format, arg...);
#endif

enum bl_reply {
    BL_REPLY_OK           = '\x10',
    BL_REPLY_FAILED       = '\x11',
    BL_REPLY_INVALID      = '\x13',
};
typedef uint8_t bl_reply_t;


enum {
    BL_INSYNC             = '\x12',
    BL_EOC                = '\x20',
    BL_GET_SYNC           = '\x21',

    BL_GET_DEVICE         = '\x22',
    BL_CHIP_ERASE         = '\x23',
    BL_PROG_MULTI         = '\x27',
    BL_GET_CRC            = '\x29',
    BL_GET_OTP            = '\x2a',
    BL_GET_SN             = '\x2b',
    BL_GET_CHIP           = '\x2c',
    BL_SET_DELAY          = '\x2d',
    BL_GET_CHIP_DES       = '\x2e',
    BL_BOOT               = '\x30',
    BL_SET_BAUD           = '\x33',
} bl_command;
typedef uint8_t bl_command_t;

typedef enum bl_info_req {
    BL_INFO_REV           = '\x01',
    BL_INFO_BOARD_ID      = '\x02',
    BL_INFO_BOARD_REV     = '\x03',
    BL_INFO_FW_SIZE       = '\x04',
    BL_INFO_VEC_AREA      = '\x05',
}  bl_info_req_t;

int bootloader_recv_sync(bl_reply_t *status, const bl_timeout_t timeout);
int bootloader_cmd_blword(bl_command_t cmd, const uint8_t *params, const uint8_t paramlen, uint32_t *ret, const bl_timeout_t timeout);
int bootloader_read_blword(uint32_t *ret, const bl_timeout_t timeout);
int bootloader_get_device_info_bytes(const bl_info_req_t what, uint8_t *ret, const uint8_t count, const bl_timeout_t timeout);
int bootloader_get_device_info_blword(const bl_info_req_t what, uint32_t *ret, const uint32_t timeout);
int _bootloader_get_sn_chunk(uint8_t chunk, uint32_t *ret, const bl_timeout_t timeout);
int _bootloader_get_otp_chunk(uint8_t chunk, uint32_t *ret, const bl_timeout_t timeout);

int bootloader_cmd_simple(const bl_command_t cmd, const bl_timeout_t timeout);
int bootloader_send_command(const bl_command_t command, const uint8_t *params, const uint8_t paramlen, const bl_timeout_t timeout);;

int _bootloader_recv_sync_ok(const bl_timeout_t timeout);

#endif
