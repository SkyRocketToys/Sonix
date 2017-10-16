#ifndef __SENSOR_CAPABILITY_H__
#define __SENSOR_CAPABILITY_H__

#define REC_FHD_30FPS_8_5MBPS	(0x1<<0)
#define REC_FHD_30FPS_10MBPS	(0x1<<1)
#define REC_FHD_30FPS_12MBPS  	(0x1<<2)
#define REC_HD_60FPS_6MBPS	(0x1<<3)
#define REC_HD_60FPS_8MBPS	(0x1<<4)
#define REC_HD_60FPS_10MBPS	(0x1<<5)
#define REC_HD_30FPS_3MBPS	(0x1<<6)
#define REC_HD_30FPS_4MBPS	(0x1<<7)
#define REC_HD_30FPS_5MBPS	(0x1<<8)
#define REC_HD_45FPS_4_5MBPS 	(0x1<<9)
#define REC_HD_45FPS_6MBPS 	(0x1<<10)
#define REC_HD_45FPS_7_5MBPS 	(0x1<<11)
#define REC_VGA_60FPS_2MBPS 	(0x1<<12)


#define VIEW_HD_60FPS_2MBPS	(0x1<<13)
#define VIEW_HD_45FPS_1_5MBPS	(0x1<<14)
#define VIEW_HD_45FPS_2MBPS	(0x1<<15)
#define VIEW_HD_30FPS_1MBPS	(0x1<<16)
#define VIEW_HD_30FPS_1_2MBPS	(0x1<<17)
#define VIEW_HD_30FPS_1_5MBPS	(0x1<<18)
#define VIEW_HD_20FPS_1MBPS	(0x1<<19)
#define VIEW_HD_20FPS_1_2MBPS	(0x1<<20)
#define VIEW_HD_15FPS_1MBPS	(0x1<<22)
#define VIEW_HD_12FPS_1MBPS	(0x1<<25)
#define VIEW_VGA_30FPS_700KBPS	(0x1<<28)
#define VIEW_VGA_15FPS_500KBPS	(0x1<<30)

#define REC_CAPABILITY_MASK	(VIEW_HD_60FPS_2MBPS-1)
#define VIEW_CAPABILITY_MASK	(~REC_CAPABILITY_MASK)

#define FHD_WIDTH                   1920
#define FHD_HEIGHT                  1080
#define HD_WIDTH                    1280
#define HD_HEIGHT                   720
#define VGA_WIDTH                   640
#define VGA_HEIGHT                  360
#define QVGA_WIDTH                  320
#define QVGA_HEIGHT                 180

void get_sensor_max_capability(int *key_res_width, int *key_res_height) __attribute__((deprecated));
unsigned int get_video_capability(void) __attribute__((deprecated));

/**
 * Video Resolution Mapping utility initialization
 * @param:
 *    N/A
 * @return:
 *    0, Success.
 *    < 0, Failed.
 */
int mf_video_resmap_init(void);

/**
 * Video Resolution Mapping utility un-initialized
 * @param:
 *    N/A
 * @return:
 *    0, Success.
 *    <0, Failed.
 */
int mf_video_resmap_uninit(void);

/**
 * Report sensor maximum capability
 * @param:
 *    width,
 *    height,
 * @return:
 *    N/A
 */
void mf_video_resmap_get_sensor_max_capability(int *width, int *height);

/**
 * Report record capability
 * @param:
 *    N/A
 * @return:
 *    Record capability.
 */
unsigned mf_video_resmap_get_record_capability(void);

/**
 * Report record parameters
 * @param:
 *    width, can be NULL
 *    height, can be NULL
 *    fps, can be NULL
 *    gop, can be NULL
 *    bps, can be NULL
 * @return:
 *    0, Success.
 *    <0, Failed.
 */
int mf_video_resmap_get_record_params(unsigned *w, unsigned *h, unsigned *fps,
                                      unsigned *gop, unsigned *bps);

typedef enum _updatesel {
	UPDATESEL_WIDTH		= 0x00000001,
	UPDATESEL_HEIGHT	= 0x00000002,
	UPDATESEL_FPS		= 0x00000004,
	UPDATESEL_GOP		= 0x00000008,
	UPDATESEL_BPS		= 0x00000010,
} updatesel;

#define UPDATESEL_ALL		(0x1FL)

/**
 * Update record capability
 * @param:
 *    width,
 *    height,
 *    fps,
 *    gop,
 *    bps,
 *    select, field select
 * @return:
 *    0, Success.
 *    <0, Failed.
 */
int mf_video_resmap_update_record_params(unsigned width, unsigned height, unsigned fps, unsigned gop, unsigned bps, updatesel select);

/**
 * Report preview capability
 * @param:
 *    N/A
 * @return:
 *    Preview capability.
 */
unsigned mf_video_resmap_get_preview_capability(void);

/**
 * Report preview parameters
 * @param:
 *    width, can be NULL
 *    height, can be NULL
 *    fps, can be NULL
 *    gop, can be NULL
 *    bps, can be NULL
 * @return:
 *    0, Success.
 *    <0, Failed.
 */
int mf_video_resmap_get_preview_params(unsigned *w, unsigned *h, unsigned *fps,
                                       unsigned *gop, unsigned *bps);

/**
 * Update preview capability
 * @param:
 *    width,
 *    height,
 *    fps,
 *    gop,
 *    bps,
 *    select, field select
 * @return:
 *    0, Success.
 *    <0, Failed.
 */
int mf_video_resmap_update_preview_params(unsigned width, unsigned height, unsigned fps, unsigned gop, unsigned bps, updatesel select);

#endif
