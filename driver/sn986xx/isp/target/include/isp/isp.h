/** \file isp.h
 * Functions in this file are show :
 * \n 1.define isp capture interface
 * \n 
 * \author Qingbin Li
 * \date   2015-8-31
 */

#ifndef __DRIVER_ISP_H__
#define __DRIVER_ISP_H__

#ifdef __cplusplus
extern "C"{
#endif

#include <sys_clock.h>
#include <time.h>
#include <sys/time.h>

/*  Four-character-code (FOURCC) */
#define __fourcc(a, b, c, d) 	((unsigned int)(a) | ((unsigned int)(b) << 8) | ((unsigned int)(c) << 16) | ((unsigned int)(d) << 24))

#define VIDEO_PIX_FMT_SNX420   	__fourcc('S', '4', '2', '0') /* SONIX SN98600/98610 capture for both H.264 and JPEG */
#define VIDEO_PIX_FMT_SBGGR10 	__fourcc('B', 'G', '1', '0') /* 10  BGBG.. GRGR.. */
#define VIDEO_PIX_FMT_SBGGR8  	__fourcc('B', 'A', '8', '1') /*  8  BGBG.. GRGR.. */


struct snx_frame_ctx{
	int index;
	void *userptr;
	int length;
	int size;
	struct timeval	timestamp;
	int reserved;
};

struct snx_iq_entry{
	unsigned char name[8];
	unsigned long start;
	unsigned long end;
};

extern int snx_isp_open(int ch, int width, int height, int rate, unsigned int fmt);
extern int snx_isp_close(int ch);

extern int snx_isp_reqbufs(int ch, int *c);
extern int snx_isp_querybuf(int ch, struct snx_frame_ctx *ctx);

extern int snx_isp_qbuf(int ch, struct snx_frame_ctx *ctx);
extern int snx_isp_dqbuf(int ch, struct snx_frame_ctx *ctx);

extern int snx_isp_streamon(int ch);
extern int snx_isp_streamoff(int ch);

extern void snx_isp_print_drop_frame(int on_off);

extern int snx_isp_get_rate(int ch, int *rate);
extern int snx_isp_set_rate(int ch, int rate);
extern int snx_isp_cap_xy(int ch, int x, int y, int w, int h);

extern int snx_isp_ioctl(int cmd, const char *path, char *buffer, int *sz);

extern int snx_isp_init(void);
extern int snx_isp_exit(void);

#ifdef __cplusplus
}
#endif

#endif
