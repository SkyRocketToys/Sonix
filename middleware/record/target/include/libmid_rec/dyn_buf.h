#ifndef __DBN_BUF__H__
#define __DBN_BUF__H__
/*#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <limits.h>*/
#include <FreeRTOS.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#define MP4_AVIO_FLAG_READ  1                                      /**< read-only */
#define MP4_AVIO_FLAG_WRITE 2                                      /**< write-only */
#define MP4_AVIO_FLAG_READ_WRITE (AVIO_FLAG_READ|AVIO_FLAG_WRITE)  /**< read-write pseudo flag */
#define MP4_AVIO_SEEKABLE_NORMAL 0x0001
#define MP4_INT_MAX 2147483647
#define MP4_FFMIN(a,b) ((a) > (b) ? (b) : (a))
#define MP4_FFMAX(a,b) ((a) > (b) ? (a) : (b))
#define MP4_FFMAX3(a,b,c) MP4_FFMAX(MP4_FFMAX(a,b),c)
typedef struct {
    unsigned char *buffer;  /**< Start of the buffer. */
    int buffer_size;        /**< Maximum buffer size */
    unsigned char *buf_ptr; /**< Current position in the buffer */
    unsigned char *buf_end; /**< End of the data, may be less than
                                 buffer+buffer_size if the read function returned
                                 less data than requested, e.g. for streams where
                                 no more data has been received yet. */
    void *opaque;           /**< A private pointer, passed to the read/write/seek/...
                                 functions. */
    int (*read_packet)(void *opaque, uint8_t *buf, int buf_size);
    int (*write_packet)(void *opaque, uint8_t *buf, int buf_size);
    int64_t (*seek)(void *opaque, int64_t offset, int whence);
    int64_t pos;
    int must_flush;
    int eof_reached;    
    int write_flag;       

    int max_packet_size;
    unsigned long checksum;
    unsigned char *checksum_ptr;
    unsigned long (*update_checksum)(unsigned long checksum, const uint8_t *buf, unsigned int size);
    int error;             
    int (*read_pause)(void *opaque, int pause); 
    int64_t (*read_seek)(void *opaque, int stream_index,
                         int64_t timestamp, int flags);
    int seekable;
} MP4AVIOContext;



MP4AVIOContext *mp4_avio_alloc_context(
                  unsigned char *buffer,
                  int buffer_size,
                  int write_flag,
                  void *opaque,
                  int (*read_packet)(void *opaque, uint8_t *buf, int buf_size),
                  int (*write_packet)(void *opaque, uint8_t *buf, int buf_size),
                  int64_t (*seek)(void *opaque, int64_t offset, int whence));

void mp4_avio_w8(MP4AVIOContext *s, int b);
void mp4_avio_write(MP4AVIOContext *s, const unsigned char *buf, int size);
void mp4_avio_wl64(MP4AVIOContext *s, uint64_t val);
void mp4_avio_wb64(MP4AVIOContext *s, uint64_t val);
void mp4_avio_wl32(MP4AVIOContext *s, unsigned int val);
void mp4_avio_wb32(MP4AVIOContext *s, unsigned int val);
void mp4_avio_wl24(MP4AVIOContext *s, unsigned int val);
void mp4_avio_wb24(MP4AVIOContext *s, unsigned int val);
void mp4_avio_wl16(MP4AVIOContext *s, unsigned int val);
void mp4_avio_wb16(MP4AVIOContext *s, unsigned int val);


int mp4_avio_put_str(MP4AVIOContext *s, const char *str);
int mp4_avio_put_str16le(MP4AVIOContext *s, const char *str);

#define MP4_AVSEEK_SIZE 0x10000
#define MP4_AVSEEK_FORCE 0x20000


int64_t mp4_avio_seek(MP4AVIOContext *s, int64_t offset, int whence);
int64_t mp4_avio_skip(MP4AVIOContext *s, int64_t offset);
int64_t mp4_avio_tell(MP4AVIOContext *s);
int64_t mp4_avio_size(MP4AVIOContext *s);

int mp4_url_feof(MP4AVIOContext *s);

void mp4_avio_flush(MP4AVIOContext *s);
int mp4_avio_read(MP4AVIOContext *s, unsigned char *buf, int size);
int          mp4_avio_r8  (MP4AVIOContext *s);
unsigned int mp4_avio_rl16(MP4AVIOContext *s);
unsigned int mp4_avio_rl24(MP4AVIOContext *s);
unsigned int mp4_avio_rl32(MP4AVIOContext *s);
uint64_t     mp4_avio_rl64(MP4AVIOContext *s);
unsigned int mp4_avio_rb16(MP4AVIOContext *s);
unsigned int mp4_avio_rb24(MP4AVIOContext *s);
unsigned int mp4_avio_rb32(MP4AVIOContext *s);
uint64_t     mp4_avio_rb64(MP4AVIOContext *s);

int mp4_avio_open_dyn_buf(MP4AVIOContext **s, int size);
int mp4_avio_close_dyn_buf(MP4AVIOContext *s, uint8_t **pbuffer);

#endif

