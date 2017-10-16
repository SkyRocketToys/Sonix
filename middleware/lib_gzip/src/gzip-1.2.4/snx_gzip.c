/*
 * Sonix gzip library for RTOS
 *
 * author: Evan Chang
 * date: 2015/12/28
 * version: 1.0.0
 * description:
 *   Providing suitable compression and decompression library for RTOS, in
 *   order to speed-up image processing flow.
 * 
 * Origin library version: gzip-1.2.4
 *
 */
#include <stdint.h>
#include <nonstdlib.h>
#include "generated/snx_sdk_conf.h"
#include "tailor.h"
#include "gzip.h"
#include "lzw.h"



#ifndef CONFIG_MIDDLEWARE_GZIP_LEVEL
#define COMPRESSION_LEVEL	6
#else
#define COMPRESSION_LEVEL	CONFIG_MIDDLEWARE_GZIP_LEVEL
#endif
#define EXTHDR			16

#ifdef CONFIG_MIDDLEWARE_GZIP
uint32_t compressBound(uint32_t sourceLen);
int gzip_compress(uint8_t *data, uint32_t ndata, uint8_t *zdata, uint32_t *nzdata);
int gzip_decompress(uint8_t *zdata, uint32_t nzdata, uint8_t *data, uint32_t *ndata);
#endif

DECLARE(uch, inbuf,  INBUFSIZ +INBUF_EXTRA);
DECLARE(uch, outbuf, OUTBUFSIZ+OUTBUF_EXTRA);
DECLARE(ush, d_buf,  DIST_BUFSIZE);
DECLARE(uch, window, 2L*WSIZE);
#ifndef MAXSEG_64K
    DECLARE(ush, tab_prefix, 1L<<BITS);
#else
    DECLARE(ush, tab_prefix0, 1L<<(BITS-1));
    DECLARE(ush, tab_prefix1, 1L<<(BITS-1));
#endif

/* local variables */
#if 0
int ascii = 0;        /* convert end-of-lines to local OS conventions */
int to_stdout = 0;    /* output to stdout (-c) */
int decompress = 0;   /* decompress (-d) */
int force = 0;        /* don't ask questions, compress links (-f) */
int no_name = -1;     /* don't save or restore the original file name */
int no_time = -1;     /* don't save or restore the original file time */
int recursive = 0;    /* recurse through directories (-r) */
int list = 0;         /* list the file contents (-l) */
int verbose = 0;      /* be verbose (-v) */
int quiet = 0;        /* be very quiet (-q) */
int do_lzw = 0;       /* generate output compatible with old compress (-Z) */
int test = 0;         /* test .gz file integrity */
int foreground;       /* set if program run in foreground */
char *progname;       /* program name */
int maxbits = BITS;   /* max bits per code for LZW */
#endif
int method = DEFLATED;/* compression method */
int level = COMPRESSION_LEVEL;        /* compression level */
#if 0
int exit_code = OK;   /* program exit code */
int save_orig_name;   /* set if original name must be saved */
int last_member;      /* set for .zip and .Z files */
int part_nb;          /* number of parts in .gz file */
long time_stamp;      /* original time stamp (modification time) */
long ifile_size;      /* input file size, -1 for devices (debug only) */
char *env;            /* contents of GZIP env variable */
char **args = NULL;   /* argv pointer if GZIP env variable defined */
char z_suffix[MAX_SUFFIX+1]; /* default suffix (can be set with --suffix) */
int  z_len;           /* strlen(z_suffix) */
#endif
long bytes_in;             /* number of input bytes */
long bytes_out;            /* number of output bytes */
long total_in = 0;         /* input bytes for all files */
long total_out = 0;        /* output bytes for all files */
#if 0
char ifname[MAX_PATH_LEN]; /* input file name */
char ofname[MAX_PATH_LEN]; /* output file name */
int  remove_ofname = 0;	   /* remove output file on error */
struct stat istat;         /* status for input file */
#endif
int  ifd;                  /* input file descriptor */
int  ofd;                  /* output file descriptor */
unsigned insize;           /* valid bytes in inbuf */
unsigned inptr;            /* index of next byte to be processed in inbuf */
unsigned outcnt;           /* bytes in output buffer */
ulg crc;


/* ========================================================================
 * Check the magic number of the input file and update ofname if an
 * original name was given and to_stdout is not set.
 * Return the compression method, -1 for error, -2 for warning.
 * Set inptr to the offset of the next byte to be processed.
 * Updates time_stamp if there is one and --no-time is not used.
 * This function may be called repeatedly for an input file consisting
 * of several contiguous gzip'ed members.
 * IN assertions: there is at least one remaining compressed member.
 *   If the member is a zip file, it must be the only one.
 */
static int get_method(int in)
{
	uch flags;
	char magic[2];
	ulg stamp;

	magic[0] = (uint8_t) get_byte();
	magic[1] = (uint8_t) get_byte();

	if (memcmp(magic, GZIP_MAGIC, 2) != 0) {
#if _DEBUG_VERBOSE
		print_msg("[compr] unsupported compr header\n");
#endif
		return -1;
	}

	method = (int) get_byte();
	if (method != DEFLATED) {
#if _DEBUG_VERBOSE
		print_msg("[compr] unknown method %d\n", method);
#endif
		return -1;
	}

	flags = (uint8_t) get_byte();
	if ((flags & ENCRYPTED) != 0) {
#if _DEBUG_VERBOSE
		print_msg("[compr] data is encrypted\n");
#endif
		return -1;
	}
	if ((flags & CONTINUATION) != 0) {
#if _DEBUG_VERBOSE
		print_msg("[compr] is a multi-part compr data\n");
#endif
		return -1;
	}
	if ((flags & RESERVED) != 0) {
#if _DEBUG_VERBOSE
		print_msg("[compr] has flags 0x%08X\n", flags);
#endif
		return -1;
	}

	stamp = (uint32_t) get_byte();
	stamp |= ((uint32_t) get_byte()) << 8;
	stamp |= ((uint32_t) get_byte()) << 16;
	stamp |= ((uint32_t) get_byte()) << 24;
	if (stamp != 0) {
#if _DEBUG_VERBOSE
		print_msg("[compr] time stamp %u\n", stamp);
#endif
		// in fact, we don't care timestamp in compr state
	}

	(void) get_byte(); // ignore extra flags
	(void) get_byte(); // ignore OS type

	if ((flags & CONTINUATION) != 0) {
		uint16_t part = (uint16_t) get_byte();
		part |= ((uint16_t) get_byte()) << 8;
#if _DEBUG_VERBOSE
		print_msg("[compr] part number %u\n", part);
#endif
	}
	if ((flags & EXTRA_FIELD) != 0) {
		uint16_t len = (uint16_t) get_byte();
		len |= ((uint16_t) get_byte()) << 8;
#if _DEBUG_VERBOSE
		print_msg("[compr] extra field of %u bytes ignored\n", len);
#endif
		inptr += len;
	}
	if ((flags & ORIG_NAME) != 0) {
		char c;
		do { c = get_byte(); } while (c != 0);
#if _DEBUG_VERBOSE
		print_msg("[compr] ignore origin name\n");
#endif
	}
	if ((flags & COMMENT) != 0) {
		char c;
		do { c = get_byte(); } while (c != 0);
#if _DEBUG_VERBOSE
		print_msg("[compr] ignore comment\n");
#endif
	}

	return 0;
}

/* ===========================================================================
 * Read a new buffer from the current input file, perform end-of-line
 * translation, and update the crc and input file size.
 * IN assertion: size >= 2 (for end-of-line translation)
 */
int file_read(buf, size)
    char *buf;
    unsigned size;
{
    unsigned len;

    Assert(insize == 0, "inbuf not empty");

    len = read(ifd, buf, size);
#if _DEBUG_VERBOSE
    print_msg("%s: buf %p size %lu len %lu\n", __func__, buf, size, len);
#endif
    if (len == (unsigned)(-1) || len == 0) return (int)len;

    crc = updcrc((uch*)buf, len);
    isize += (ulg)len;
    return (int)len;
}

/* ===========================================================================
     If the default memLevel or windowBits for deflateInit() is changed, then
   this function needs to be updated.
 */
#define GZIP_HEADERSIZE		18
uint32_t compressBound(uint32_t sourceLen)
{
#if 1
	return sourceLen + (sourceLen >> 12) + (sourceLen >> 14) +
		(sourceLen >> 25) + 13 + GZIP_HEADERSIZE;
#else
	return sourceLen + ((sourceLen + 7) >> 3) + ((sourceLen + 63) >> 6) + 5
		+ GZIP_HEADERSIZE;
#endif
}

int gzip_compress(uint8_t *data, uint32_t ndata, uint8_t *zdata, uint32_t *nzdata)
{
	uch flags = 0;
	ush attr = 0;
	ush deflate_flags = 0;

#if _DEBUG_VERBOSE
	print_msg("%s in %p (%u bytes) out %p\n", __func__,
			data, ndata, zdata);
#endif
	clear_bufs();
#if _DEBUG_VERBOSE
	print_msg("[compr] total_in %u bytes_in %u "
			"insize %u inptr %u\n",
			total_in, bytes_in, insize, inptr);
#endif
	// initialize variables, hack
	ALLOC(uch, inbuf,  INBUFSIZ +INBUF_EXTRA);
	ALLOC(uch, outbuf, OUTBUFSIZ+OUTBUF_EXTRA);
	ALLOC(ush, d_buf, DIST_BUFSIZE);
	ALLOC(uch, window, 2L*WSIZE);
#ifndef MAXSEG_64K
	ALLOC(ush, tab_prefix, 1L<<BITS);
#else
	ALLOC(ush, tab_prefix0, 1L<<(BITS-1));
	ALLOC(ush, tab_prefix1, 1L<<(BITS-1));
#endif
	ifd = (uint32_t) data;
	ofd = (uint32_t) zdata;
	total_in = ndata;

	// Write the header to the output buffer
	method = DEFLATED;
	put_byte(GZIP_MAGIC[0]);
	put_byte(GZIP_MAGIC[1]);
	put_byte(DEFLATED);

	put_byte(flags);
	put_long(0L);

#if _DEBUG_VERBOSE
	print_msg("[compr] output header 0x%02x, 0x%02x, 0x%02x, 0x%02x\n",
			outbuf[0], outbuf[1], outbuf[2], outbuf[3]);
#endif

	// Write deflated data
	crc = updcrc(0, 0);

	// Varify compression level
	if ((level < 1) || (level > 9)) {
		print_msg_queue("%s: invalid level %d [1-9]\n", level);
		level = 2;
	}

	bi_init(ofd);
	ct_init(&attr, &method);
	lm_init(level, &deflate_flags);

	put_byte((uch)deflate_flags);
	put_byte(OS_CODE);

#if _DEBUG_VERBOSE
	print_msg("%s:%d\n", __func__, __LINE__);
#endif
	(void)deflate();

#if _DEBUG_VERBOSE
	print_msg("%s:%d\n", __func__, __LINE__);
#endif
	// Write the crc and uncompressed size
	put_long(crc);
	put_long(isize);
	flush_outbuf();

	// report output length
	*nzdata = bytes_out;

	// de-initialize globe variables
	FREE(inbuf);
	FREE(outbuf);
	FREE(d_buf);
	FREE(window);
#ifndef MAXSEG_64K
	FREE(tab_prefix);
#else
	FREE(tab_prefix0);
	FREE(tab_prefix1);
#endif
	total_in = total_out = 0;

	return 0;
}

int gzip_decompress(uint8_t *zdata, uint32_t nzdata, uint8_t *data, uint32_t *ndata)
{
	ulg orig_crc = 0;
	ulg orig_len = 0;
	int n, ret;
	uch buf[EXTHDR];

#if _DEBUG_VERBOSE
	print_msg("%s in %p (%u bytes) out %p\n", __func__,
			zdata, nzdata, data);
#endif
	clear_bufs();
#if _DEBUG_VERBOSE
	print_msg("[compr] total_in %u bytes_in %u "
			"insize %u inptr %u\n",
			total_in, bytes_in, insize, inptr);
#endif

	// initialize variables, hack
	ALLOC(uch, inbuf,  INBUFSIZ +INBUF_EXTRA);
	ALLOC(uch, outbuf, OUTBUFSIZ+OUTBUF_EXTRA);
	ALLOC(ush, d_buf, DIST_BUFSIZE);
	ALLOC(uch, window, 2L*WSIZE);
#ifndef MAXSEG_64K
	ALLOC(ush, tab_prefix, 1L<<BITS);
#else
	ALLOC(ush, tab_prefix0, 1L<<(BITS-1));
	ALLOC(ush, tab_prefix1, 1L<<(BITS-1));
#endif
	ifd = (uint32_t) zdata;
	ofd = (uint32_t) data;
	total_in = nzdata;

	method = get_method(ifd);
	if (method < 0) {
#if _DEBUG_VERBOSE
		print_msg("[compr] parsing compressed header failed\n");
#endif
		return -1;
	}

	// initialize crc
	updcrc(NULL, 0);

	// Actually do the compression/decompression. Loop over zipped members.
	for (;;) {
		ret = inflate();
		if (ret == 3) {
#if _DEBUG_VERBOSE
			print_msg("[compr] out of memory\n");
#endif
		} else if (ret != 0) {
#if _DEBUG_VERBOSE
			print_msg("[compr] invalid compressed data\n");
#endif
		}

		// Get the crc and original length
		for (n = 0; n < 8; ++n) {
			buf[n] = (uch) get_byte(); // may cause an error if EOF
		}
		orig_crc = LG(buf);
		orig_len = LG(buf+4);

		ulg act_crc = updcrc(outbuf, 0);
#if _DEBUG_VERBOSE
		print_msg("[compr] origin crc 0x%08x, length %u "
			"actual crc 0x%08x\n", orig_crc, orig_len, act_crc);
#endif

		// validate decompression
		if (orig_crc != updcrc(outbuf, 0)) {
#if _DEBUG_VERBOSE
			print_msg("[compr] orig_crc 0x%08x, actual crc 0x%08x\n",
					orig_crc, updcrc(outbuf, 0));
#endif
		}
		if (orig_len != (ulg) bytes_out) {
#if _DEBUG_VERBOSE
			print_msg("[compr] orig_len %u bytes_out %u\n",
					orig_len, bytes_out);
#endif
		}

#if _DEBUG_VERBOSE
		print_msg("[compr] total_in %u bytes_in %u "
				"insize %u inptr %u\n",
				total_in, bytes_in, insize, inptr, bytes_out);
		print_msg("[compr] total_out %u bytes_out %u\n",
				total_out, bytes_out);
#endif
		if (inptr == insize) break;
	}

#if _DEBUG_VERBOSE
	print_msg("[compr] output length %u\n", bytes_out);
#endif
	// report output length
	*ndata = bytes_out;

	// de-initialize globe variables
	FREE(inbuf);
	FREE(outbuf);
	FREE(d_buf);
	FREE(window);
#ifndef MAXSEG_64K
	FREE(tab_prefix);
#else
	FREE(tab_prefix0);
	FREE(tab_prefix1);
#endif
	total_in = total_out = 0;

	return 0;
}

