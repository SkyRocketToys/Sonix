#include "bootloader_client.h"

#include <alloca.h>
#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/select.h>
#include <unistd.h>
#include <stdarg.h>

#include <queue.h>

const bl_timeout_t bl_default_timeout = 100;
const uint8_t bl_prog_multi_chunksize = 192;

// @115,200:
//  16 == 130 seconds
//  32 == 126 seconds
//  64 == 124 seconds
// 128 == 110 seconds
// @625,000
//  16 == 101 seconds
//  32 == 63
//  64 == 32
// 128 == 30
// 160 == 25
// 176 == 24 seconds
// 192 == 22 seconds
// 208 == 28 seconds
// 224 == 27 seeconds
// 252 == 24 seconds

#define BOOTLOADER_CLIENT_DEBUG 0
#if BOOTLOADER_CLIENT_DEBUG
#define debug(format, args ...) bootloader_debug(format, args)
#else
#define debug(format, ...)
#endif

int bootloader_send_mavlink_reboot(const bl_timeout_t timeout)
{
    const uint8_t cmd_reboot_1[] = "\xfe\x21\x72\xff\x00\x4c\x00\x00\x80\x3f\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xf6\x00\x01\x00\x00\x48\xf0";
    const uint8_t cmd_reboot_0[] = "\xfe\x21\x45\xff\x00\x4c\x00\x00\x80\x3f\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xf6\x00\x00\x00\x00\xd7\xac";

    bootloader_write(cmd_reboot_0, sizeof(cmd_reboot_0), timeout);
    bootloader_write(cmd_reboot_0, sizeof(cmd_reboot_0), timeout);
    bootloader_write(cmd_reboot_1, sizeof(cmd_reboot_1), timeout);
    return 0;
}

/*
 * return 0 for success
 */
int bootloader_send_command(const bl_command_t command, const uint8_t *params, const uint8_t paramlen, const bl_timeout_t timeout)
{
    uint8_t x = command;
    bootloader_write(&x, 1, timeout);
    debug("command: 0x%02x\n", x);
    if (params != NULL) {
	bootloader_write(params, paramlen, timeout);
    }
    x = BL_EOC;

    bootloader_write(&x, 1, timeout);
    return 0;
}

int bootloader_send_nop()
{
    uint8_t nop = '\x00';
    bootloader_write(&nop, 1, bl_default_timeout);
    return 0;
}


/*
 * send a command, expect a sync
 */
int bootloader_cmd_simple(const bl_command_t cmd, const bl_timeout_t timeout)
{
    if (bootloader_send_command(cmd, NULL, 0, timeout) != 0) {
	return -1;
    }
    return _bootloader_recv_sync_ok(timeout);
}

int bootloader_get_sync()
{
    return bootloader_cmd_simple(BL_GET_SYNC, bl_default_timeout);
}

int bootloader_boot()
{
    return bootloader_cmd_simple(BL_BOOT, bl_default_timeout);
}

int bootloader_erase_send()
{
    if (bootloader_send_command(BL_CHIP_ERASE, NULL, 0, bl_default_timeout) != 0) {
	return -1;
    }
    return 0;
}


ssize_t _bootloader_readbyte(uint8_t *byte, const bl_timeout_t timeout)
{
    ssize_t ret = bootloader_readbyte(byte, timeout);
    debug("Got byte (0x%02x)\n", *byte);
    return ret;
}

int bootloader_recv_sync(bl_reply_t *status, const bl_timeout_t timeout)
{
    uint8_t b;
    debug("Waiting for insync\n",1);
    if (!_bootloader_readbyte(&b, timeout)) {
	return -1;
    }
    if (b != BL_INSYNC) {
	return -1;
    }

    debug("Waiting for status\n",1);
    if (!_bootloader_readbyte(&b, timeout)) {
	return -1;
    }
    *status = b;
    debug("status: 0x%02x\n", *status);
    return 0;
}

int bootloader_read_blword(uint32_t *ret, const bl_timeout_t timeout)
{
    *ret = 0;
    uint8_t i;
    for (i=0; i<4; i++) {
	uint8_t c;
	if (!_bootloader_readbyte(&c, timeout)) {
	    return -1;
	}
	*ret |= c << 8*i;
    }
    return 0;
}

int bootloader_get_device_info_bytes(const bl_info_req_t what, uint8_t *ret, const uint8_t count, const bl_timeout_t timeout)
{
    uint8_t param[1];
    param[0] = what;
    if (bootloader_send_command(BL_GET_DEVICE, param, 1, timeout) != 0) {
        return -1;
    }

    uint8_t i;
    for (i=0; i<count; i++) {
        if (_bootloader_readbyte(&ret[i], timeout) == -1) {
            return -1;
        }
    }

    return _bootloader_recv_sync_ok(timeout);
}

int bootloader_get_device_info_blword(const bl_info_req_t what, uint32_t *ret, const uint32_t timeout)
{
    uint8_t param[1];
    param[0] = what;
    if (bootloader_send_command(BL_GET_DEVICE, param, 1, timeout) != 0) {
	return -1;
    }

    if (bootloader_read_blword(ret, timeout) == -1) {
	return -1;
    }

    return _bootloader_recv_sync_ok(timeout);
}

int bootloader_get_bl_rev(uint32_t *ret)
{
    return bootloader_get_device_info_blword(BL_INFO_REV, ret, bl_default_timeout);
}

int bootloader_get_board_id(uint32_t *ret)
{
    return bootloader_get_device_info_blword(BL_INFO_BOARD_ID, ret, bl_default_timeout);
}

int bootloader_get_board_rev(uint32_t *ret)
{
    return bootloader_get_device_info_blword(BL_INFO_BOARD_REV, ret, bl_default_timeout);
}

int bootloader_get_fw_size(uint32_t *ret)
{
    return bootloader_get_device_info_blword(BL_INFO_FW_SIZE, ret, bl_default_timeout);
}

int bootloader_get_vec_area(uint32_t *ret)
{
    uint8_t vecdata[16];
    if (bootloader_get_device_info_bytes(BL_INFO_VEC_AREA, vecdata, 16, bl_default_timeout) == -1) {
	return -1;
    }
    uint8_t vecdata_off = 0;
    uint8_t i;
    for (i=0; i<4; i++) {
	ret[i] = 0;
	ret[i] |= vecdata[vecdata_off++] << 24;
	ret[i] |= vecdata[vecdata_off++] << 16;
	ret[i] |= vecdata[vecdata_off++] << 8;
	ret[i] |= vecdata[vecdata_off++];
    }

    return 0;
}

/*
 * execute a command, return a blword
 */
int bootloader_cmd_blword(bl_command_t cmd, const uint8_t *params, const uint8_t paramlen, uint32_t *ret, const bl_timeout_t timeout)
{
    if (bootloader_send_command(cmd, params, paramlen, timeout) != 0) {
	return -1;
    }

    if (bootloader_read_blword(ret, timeout) == -1) {
	return -1;
    }

    return _bootloader_recv_sync_ok(timeout);
}

int bootloader_get_chip_id(uint32_t *ret)
{
    return bootloader_cmd_blword(BL_GET_CHIP, NULL, 0, ret, bl_default_timeout);
}

int bootloader_get_crc(uint32_t *ret)
{
    return bootloader_cmd_blword(BL_GET_CRC, NULL, 0, ret, bl_default_timeout*100);
}

int _bootloader_get_sn_chunk(uint8_t chunk, uint32_t *ret, const bl_timeout_t timeout)
{
    uint8_t param[4];
    param[0] = chunk;
    param[1] = 0;
    param[2] = 0;
    param[3] = 0;
    return bootloader_cmd_blword(BL_GET_SN, param, sizeof(param), ret, timeout);
}

int bootloader_get_sn(uint32_t *ret)
{
    uint8_t i = 0;
    for(i=0; i<3; i++) {
	if (_bootloader_get_sn_chunk(i*4, &ret[i], bl_default_timeout) == -1) {
	    return -1;
	}
    }
    return 0;
}

int _bootloader_get_otp_chunk(uint8_t chunk, uint32_t *ret, const bl_timeout_t timeout)
{
    uint8_t param[4];
    param[0] = chunk;
    param[1] = 0;
    param[2] = 0;
    param[3] = 0;
    return bootloader_cmd_blword(BL_GET_OTP, param, sizeof(param), ret, timeout);
}

int bootloader_get_otp(uint8_t *ret)
{
    uint16_t i;
    for(i=0; i<512; i+=4) {
	uint32_t chunk;
	if (_bootloader_get_otp_chunk(i, &chunk, bl_default_timeout) == -1) {
	    return -1;
	}
        uint8_t j;
	for (j=0; j<4; j++) {
	    ret[i+j] = chunk & 0xff;
	    chunk >>= 8;
	}
    }
    return 0;
}

int bootloader_set_baudrate(uint32_t rate)
{
    uint8_t param[4];
    uint8_t i;
    for (i=0; i<4; i++) {
        param[i] = rate & 0xff;
        rate >>= 8;
    }
    if (bootloader_send_command(BL_SET_BAUD, param, sizeof(param), bl_default_timeout) != 0) {
        return -1;
    }

    return _bootloader_recv_sync_ok(bl_default_timeout);
}

int _bootloader_recv_sync_ok(const bl_timeout_t timeout)
{
    bl_reply_t status;
    if (bootloader_recv_sync(&status, timeout) == -1) {
        bootloader_debug("Failed to recv sync\n");
	return -1;
    }

    switch(status) {
    case BL_REPLY_INVALID:
        bootloader_debug("Reply: Invalid\n");
	return -1;
    case BL_REPLY_FAILED:
        bootloader_debug("Reply: Failed\n");
	return -1;
    case BL_REPLY_OK:
	return 0;
    }
    bootloader_debug("Reply: Weird\n");
    return -1;
}

int bootloader_get_chip_des(uint8_t *ret, const uint32_t retlen)
{
    if (bootloader_send_command(BL_GET_CHIP_DES, NULL, 0, bl_default_timeout) != 0) {
	return -1;
    }

    /* read length in, but look for an error code in first two bytes */
    uint8_t l1;
    if (!_bootloader_readbyte(&l1, bl_default_timeout)) {
	return -1;
    }

    uint8_t l2;
    if (!_bootloader_readbyte(&l2, bl_default_timeout)) {
	return -1;
    }

    if (l1 == BL_INSYNC && (l2 == BL_REPLY_FAILED || l2 == BL_REPLY_INVALID)) {
	debug("Possible bad command response when reading length\n", 1);
	return -1;
    }

    uint8_t l3;
    if (!_bootloader_readbyte(&l3, bl_default_timeout)) {
	return -1;
    }

    uint8_t l4;
    if (!_bootloader_readbyte(&l4, bl_default_timeout)) {
	return -1;
    }


    const uint32_t target_len = l4 <<24 | l3 << 16 | l2 << 8 | l1;

    uint32_t i;
    for (i=0; i<target_len; i++) {
	uint8_t c;
	if (!_bootloader_readbyte(&c, bl_default_timeout)) {
	    return -1;
	}
	if (i<retlen) {
	    ret[i] = c;
	}
    }

    bl_reply_t status;
    if (bootloader_recv_sync(&status, bl_default_timeout) == -1) {
	return -1;
    }
    if (status != BL_REPLY_OK) {
	return -1;
    }

    return 0;
}


const uint32_t bootloader_crctab[] = {
    0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f, 0xe963a535, 0x9e6495a3,
    0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988, 0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91,
    0x1db71064, 0x6ab020f2, 0xf3b97148, 0x84be41de, 0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
    0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9, 0xfa0f3d63, 0x8d080df5,
    0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172, 0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b,
    0x35b5a8fa, 0x42b2986c, 0xdbbbc9d6, 0xacbcf940, 0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
    0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423, 0xcfba9599, 0xb8bda50f,
    0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924, 0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d,
    0x76dc4190, 0x01db7106, 0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
    0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d, 0x91646c97, 0xe6635c01,
    0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e, 0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457,
    0x65b0d9c6, 0x12b7e950, 0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
    0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7, 0xa4d1c46d, 0xd3d6f4fb,
    0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0, 0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9,
    0x5005713c, 0x270241aa, 0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
    0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81, 0xb7bd5c3b, 0xc0ba6cad,
    0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a, 0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683,
    0xe3630b12, 0x94643b84, 0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
    0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb, 0x196c3671, 0x6e6b06e7,
    0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc, 0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5,
    0xd6d6a3e8, 0xa1d1937e, 0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
    0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55, 0x316e8eef, 0x4669be79,
    0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236, 0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f,
    0xc5ba3bbe, 0xb2bd0b28, 0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
    0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f, 0x72076785, 0x05005713,
    0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38, 0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21,
    0x86d3d2d4, 0xf1d4e242, 0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
    0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69, 0x616bffd3, 0x166ccf45,
    0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2, 0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db,
    0xaed16a4a, 0xd9d65adc, 0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
    0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693, 0x54de5729, 0x23d967bf,
    0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94, 0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d
};


const uint8_t bootloader_crcpad[] =  { '\xff', '\xff', '\xff', '\xff' };

uint32_t bootloader_crc32_chunk(const uint8_t *chunk, const uint32_t chunklen, uint32_t state)
{
    uint32_t i;
    for (i=0; i<chunklen; i++) {
        uint32_t index = (state ^ chunk[i]) & 0xff;
        state = bootloader_crctab[index] ^ (state >> 8);
    }
    return state;}


uint32_t bootloader_crc32_firmware(const uint8_t *fw, uint32_t fwlen, const uint32_t padlen)
{
    if (fwlen % 4) {
        debug("not a multiple of 4 - bad!\n", 1);
    }
    uint32_t state;
    state = bootloader_crc32_chunk(fw, fwlen, 0);
    uint32_t i;
    for (i=fwlen; i<padlen; i+=4) {
        state = bootloader_crc32_chunk(bootloader_crcpad, 4, state);
    }
    return state;
}

int bootloader_program_chunk(const uint8_t *chunk, const uint8_t chunksize, const uint8_t writesize)
{
    const uint8_t params_size = 1 + writesize;
    uint8_t *params = alloca(params_size);
    if (params == NULL) {
        return -1;
    }
    memset(params, '\xff', params_size);

    /* debug("chunksize=%u\n", chunksize); */
    params[0] = chunksize;
    memcpy(&params[1], chunk, chunksize);
    if (bootloader_send_command(BL_PROG_MULTI, params, params_size, bl_default_timeout) != 0) {
        return -1;
    }
    return _bootloader_recv_sync_ok(bl_default_timeout);
}

/*
 * write fw
 */
int bootloader_program(uint8_t *fw, const uint32_t fwlen, void (*f)(uint8_t))
{
    uint32_t total_fw_bytes_written = 0;
    while (total_fw_bytes_written < fwlen) {
        uint32_t chunksize = fwlen - total_fw_bytes_written;
        if (chunksize > bl_prog_multi_chunksize) {
            chunksize = bl_prog_multi_chunksize;
        }
        const uint8_t writesize = chunksize + (chunksize%4);
        if (bootloader_program_chunk(&fw[total_fw_bytes_written], chunksize, writesize) == -1) {
            return -1;
        }
        if (f != NULL) {
            f(total_fw_bytes_written*100/fwlen);
        }
        total_fw_bytes_written += chunksize;
    }
    return 0;
}

int bootloader_set_boot_delay(const uint8_t d)
{
    if (bootloader_send_command(BL_SET_DELAY, &d, 1, bl_default_timeout) == -1) {
        return -1;
    }

    return _bootloader_recv_sync_ok(bl_default_timeout);
}
