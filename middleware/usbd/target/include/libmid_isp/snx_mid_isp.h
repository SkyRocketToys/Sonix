
/** \file isp_lib_api.h
 * Functions in this file as below :
 * \n  SONiX ISP middleware header file, which include IQ, motion detection,
 * \n  OSD, sensor setting,  DRC,  private mask, ae & awb, image filters functions.
 * \n
 * \author Qingbin Li
 * \date   2014-08-13 update
 */


#ifndef __ISP_LIB_API_H__
#define __ISP_LIB_API_H__

#ifdef __cplusplus
extern "C"{
#endif

#include "generated/snx_sdk_conf.h"

/**
 * \defgroup isp_mid ISP middleware modules
 * \n This is the middleware ISP application interface
 * \n
 * @{
 */

/**
 * \defgroup A1 Constant Macro Define
 * \n
 * @{
 */


enum{
	ISP_CH_0 = 0x0,
	ISP_CH_1 = 0x1,
	ISP_CH_2 = 0x2,
};

enum{
	ISP_OSD_LINE_1 = 0x1,
	ISP_OSD_LINE_2 = 0x2,
};


enum{
	ISP_DISABLE = 0,
	ISP_ENABLE = 1,
};

enum{
	ISP_MIRROR_FLIP_OFF = 0x0,
	ISP_FLIP_ON,
	ISP_MIRROR_ON,
	ISP_MIRROR_FLIP_ON,
};

#ifndef TIMESTAMP_TEMP
#define TIMESTAMP_TEMP "0123456789:/."      /*!< data string for timestamp */
#define SPACE_TEMP " "                      /*!< space for timestamp */
#endif

/** @} */


/**
 * \defgroup A2 IQ Tunning Functions
 * \n
 * @{
 */

/** \fn int snx_isp_iq_write(void *b, int s)
 * \brief write iq command
 * \n
 * \param b :command string
 * \param s :sizeof command string
 * \n
 * \return 0:sucess, -1:fail
 */
int snx_isp_iq_write(void *b, int s);

/** \fn int snx_isp_iq_read(void *b, int s)
 * \brief read iq command execuation result
 * \n
 * \param b :command string
 * \param s :sizeof command string
 * \n
 * \return 0:sucess, -1:fail
 */
int snx_isp_iq_read(void *b, int s);

/** \fn int snx_isp_iq_enable_get(int *enable)
 * \brief get iq function enable/disable status
 * \n
 * \param enable :1:enable 0:disable
 * \n
 * \return 0:sucess, -1:fail
 */
int snx_isp_iq_enable_get(int *enable);

/** \fn int snx_isp_iq_enable_set(int enable)
 * \brief set iq function enable/disable
 * \n
 * \param enable :1:enable 0:disable
 * \n
 * \return 0:sucess, -1:fail
 */
int snx_isp_iq_enable_set(int enable);

/** \fn int snx_isp_iq_firmware_reload(void)
 * \brief reload iq firmware into driver from filesystem /etc/firmware named IQ.bin
 * \n
 * \n
 * \return 0:sucess, -1:fail
 */
int snx_isp_iq_firmware_reload(void);

int snx_isp_iq_nra_get(int *val);
int snx_isp_iq_nra_set(int val);

#if defined(CONFIG_SYSTEM_PLATFORM_ST58660FPGA) || defined(CONFIG_SYSTEM_PLATFORM_SN98660) || defined(CONFIG_SYSTEM_PLATFORM_SN98661) || defined(CONFIG_SYSTEM_PLATFORM_SN98670) || defined(CONFIG_SYSTEM_PLATFORM_SN98671) || defined(CONFIG_SYSTEM_PLATFORM_SN98672) || defined (CONFIG_SYSTEM_PLATFORM_SN98293)

int snx_isp_iq_nrn_get(int *val);
int snx_isp_iq_nrn_set(int val);

typedef struct {
	struct {
		unsigned int d0:4;
		unsigned int d1:4;
		unsigned int m0:4;
		unsigned int m1:4;
		unsigned int y0:4;
		unsigned int y1:4;
		unsigned int y2:4;
		unsigned int y3:4;
	}date;
	struct {
		unsigned int unused:8;
		unsigned int s0:4;
		unsigned int s1:4;
		unsigned int i0:4;
		unsigned int i1:4;
		unsigned int h0:4;
		unsigned int h1:4;
	}time;
}isp_iq_ver_t;

int snx_isp_iq_version_get(isp_iq_ver_t *ver);

#endif


/** @} */


/**
 * \defgroup A3 Motion Detection Functions
 * \n
 * @{
 */

/** \fn int snx_isp_md_enable_get(int *enable)
 * \brief get motion detection enable/diable status
 * \n
 * \param enable :1:enable, 0:disable
 * \n
 * \return 0:sucess, -1:fail
 */
int snx_isp_md_enable_get(int *enable);

/** \fn int snx_isp_md_enable_set(int enable)
 * \brief set motion detection enable/disable
 * \n
 * \param enable :1:enable, 0:disable
 * \
 * \n
 * \return 0:sucess, -1:fail
 */
int snx_isp_md_enable_set(int enable);

/** \fn int snx_isp_md_threshold_get(int *threshold)
 * \brief get Y threshold
 * \n
 * \param threshold :0~65535
 * \n
 * \return 0:sucess, -1:fail
 */
int snx_isp_md_threshold_get(int *threshold);

/** \fn int snx_isp_md_threshold_set(int threshold)
 * \brief set Y threshold into driver,
 * \n      when the Y subtraction of previous & current block
 * \n      greater than threshold, motion interruption will be tirggered.
 * \n
 * \param threshold :0~65535
 * \n
 * \return 0:sucess, -1:fail
 */
int snx_isp_md_threshold_set(int threshold);

/** \fn int snx_isp_md_int_get(int *status)
 * \brief  wait for motion interruption occur
 * \n
 * \param status :1:detected motion, 0:timeout
 * \n
 * \return 0:sucess, -1:fail
 */
int snx_isp_md_int_get(int *status);

/** \fn int snx_isp_md_int_timeout_set(int ms)
 * \brief set interruption waiting timeout value
 * \n
 * \param ms :unit millisecond
 * \n
 * \return 0:sucess, -1:fail
 */
int snx_isp_md_int_timeout_set(int ms);

/** \fn int snx_isp_md_int_timeout_get(int *ms)
 * \brief get wait interruption timeout value
 * \n
 * \param ms :unit millisecond
 * \n
 * \return 0:sucess, -1:fail
 */
int snx_isp_md_int_timeout_get(int *ms);

/** \fn int snx_isp_md_int_threshold_get(int *threshold)
 * \brief  get motion interruption block count threshold
 * \n
 * \param threshold :1~192
 * \n
 * \return 0:sucess, -1:fail
 */
int snx_isp_md_int_threshold_get(int *threshold);

/** \fn int snx_isp_md_int_threshold_set(int threshold)
 * \brief set motion interruption block count threshold
 * \n
 * \param threshold :1~192
 * \n
 * \return 0:sucess, -1:fail
 */
int snx_isp_md_int_threshold_set(int threshold);

/** \fn int snx_isp_md_block_mask_get(unsigned int mask[])
 * \brief get motion block mask
 * \n
 * \param mask :192bits, 6*32bits
 * \n
 * \return 0:sucess, -1:fail
 */
int snx_isp_md_block_mask_get(unsigned int mask[]);

/** \fn int snx_isp_md_block_mask_set(unsigned int mask[])
 * \brief set motion block mask
 * \n
 * \param mask :192bits, 6*32bits
 * \n
 * \return 0:sucess, -1:fail
 */
int snx_isp_md_block_mask_set(unsigned int mask[]);

/** \fn int snx_isp_md_block_report_get(unsigned int report[])
 * \brief get motion block report
 * \n
 * \param report :192bits, 6*32bits
 * \n
 * \return 0:sucess, -1:fail
 */
int snx_isp_md_block_report_get(unsigned int report[]);

/** \fn int snx_isp_md_moving_counts_get(unsigned int *counts)
 * \brief get moving blocks count
 * \n
 * \param report :0~192
 * \n
 * \return 0:sucess, -1:fail
 */
int snx_isp_md_moving_counts_get(unsigned int *counts);

/** \fn int snx_isp_md_csum_set(int enable)
 * \brief enable or disable block check sum
 * \n
 * \param enable :0, 1
 * \n
 * \return 0:sucess, -1:fail
 */
int snx_isp_md_csum_set(int enable);

/** \fn int snx_isp_md_csum_get(unsigned int report[12][16])
 * \brief get blocks check sum
 * \n
 * \param report :192bits, 6*32bits
 * \n
 * \return 0:sucess, -1:fail
 */
int snx_isp_md_csum_get(unsigned int report[12][16]);


#if defined(CONFIG_SYSTEM_PLATFORM_ST58660FPGA) || defined(CONFIG_SYSTEM_PLATFORM_SN98660) || defined(CONFIG_SYSTEM_PLATFORM_SN98661) || defined(CONFIG_SYSTEM_PLATFORM_SN98670) || defined(CONFIG_SYSTEM_PLATFORM_SN98671) || defined(CONFIG_SYSTEM_PLATFORM_SN98672) || defined (CONFIG_SYSTEM_PLATFORM_SN98293)

int snx_isp_md_ch_threshold_get(int ch, int *threshold);
int snx_isp_md_ch_threshold_set(int ch, int threshold);
int snx_isp_md_ch_int_get(int ch, int *status);
int snx_isp_md_ch_int_timeout_set(int ch, int ms);
int snx_isp_md_ch_int_timeout_get(int ch, int *ms);
int snx_isp_md_ch_int_threshold_get(int ch, int *threshold);
int snx_isp_md_ch_int_threshold_set(int ch, int threshold);
int snx_isp_md_ch_block_mask_get(int ch, unsigned int mask[]);
int snx_isp_md_ch_block_mask_set(int ch, unsigned int mask[]);
int snx_isp_md_ch_block_report_get(int ch, unsigned int report[]);
int snx_isp_md_ch_moving_counts_get(int ch, unsigned int *counts);


#endif

/** @} */


/**
 * \defgroup A4 ISP OSD Functions
 * \n
 * @{
 */

/** \fn int snx_isp_osd_enable_set(int ch, int enable)
 * \brief enable/disable video channel osd
 * \n
 * \param ch :0~1
 * \param enable :1 enable, 0 disable
 * \n
 * \return 0:sucess, -1:fail
 */
int snx_isp_osd_enable_set(int ch, int enable);

/** \fn int snx_isp_osd_enable_get(int ch, int *enable)
 * \brief get video channel osd enable/disable status
 * \n
 * \param ch :0~1
 * \param enable :1 enable, 0 disable
 * \n
 * \return 0:sucess, -1:fail
 */
int snx_isp_osd_enable_get(int ch, int *enable);

/** \fn int snx_isp_osd_data_str_set(int ch, const char *str)
 * \brief set osd data string tow display
 * \n
 * \param ch :0~1
 * \param str :string to display
 * \n
 * \return 0:sucess, -1:fail
 */
int snx_isp_osd_data_str_set(int ch, const char *str);

/** \fn int snx_isp_osd_data_str_get(int ch, char *str)
 * \brief get osd data string tow display
 * \n
 * \param ch :0~1
 * \param str :string to display
 * \n
 * \return 0:sucess, -1:fail
 */
int snx_isp_osd_data_str_get(int ch, char *str);

/** \fn int snx_isp_osd_timestamp_set(int ch, int enable)
 * \brief enable/disable osd timestamp
 * \n
 * \param ch :0~1
 * \param enable : 1 enable, 0 disable
 * \n
 * \return 0:sucess, -1:fail
 */
int snx_isp_osd_timestamp_set(int ch, int line);

/** \fn int snx_isp_osd_timestamp_get(int ch, int *enable)
 * \brief get osd timestamp enable/disable status
 * \n
 * \param ch :0~1
 * \param enable :1 enable, 0 disable
 * \n
 * \return 0:sucess, -1:fail
 */
int snx_isp_osd_timestamp_get(int ch, int *line);

/** \fn int snx_isp_osd_gain_set(int ch, int gain)
 * \brief set osd gain value
 * \n
 * \param ch :0~1
 * \param gain :0~7
 * \n
 * \return 0:sucess, -1:fail
 */
int snx_isp_osd_gain_set(int ch, int gain);

/** \fn int snx_isp_osd_gain_get(int ch, int *gain)
 * \brief get osd gain value
 * \n
 * \param ch :0~1
 * \param gain :0~7
 * \n
 * \return 0:sucess, -1:fail
 */
int snx_isp_osd_gain_get(int ch, int *gain);

/** \fn int snx_isp_osd_txt_color_set(int ch, int color)
 * \brief set txt font color
 * \n
 * \param ch :0~1
 * \param color :0x000000~0xffffff
 * \n
 * \return 0:sucess, -1:fail
 */
int snx_isp_osd_txt_color_set(int ch, int color);

/** \fn int snx_isp_osd_txt_color_get(int ch, int *color)
 * \brief get txt font color
 * \n
 * \param ch :0~1
 * \param color :0x000000~0xffffff
 * \n
 * \return 0:sucess, -1:fail
 */
int snx_isp_osd_txt_color_get(int ch, int *color);

/** \fn int snx_isp_osd_edg_color_set(int ch, int color)
 * \brief set edg font color
 * \n
 * \param ch :0~1
 * \param color :0x000000~0xffffff
 * \n
 * \return 0:sucess, -1:fail
 */
int snx_isp_osd_edg_color_set(int ch, int color);

/** \fn int snx_isp_osd_edg_color_get(int ch, int *color)
 * \brief get edg font color
 * \n
 * \param ch :0~1
 * \param color :0x000000~0xffffff
 * \n
 * \return 0:sucess, -1:fail
 */
int snx_isp_osd_edg_color_get(int ch, int *color);

/** \fn int snx_isp_osd_bg_color_set(int ch, int color)
 * \brief set background color
 * \n
 * \param ch :0~1
 * \param color :0x000000~0xffffff
 * \n
 * \return 0:sucess, -1:fail
 */
int snx_isp_osd_bg_color_set(int ch, int color);

/** \fn int snx_isp_osd_bg_color_get(int ch, int *color)
 * \brief get background color
 * \n
 * \param ch :0~1
 * \param  color :0x000000~0xffffff
 * \n
 * \return 0:sucess, -1:fail
 */
int snx_isp_osd_bg_color_get(int ch, int *color);

/** \fn int snx_isp_osd_txt_transp_set(int ch, int transp)
 * \brief set txt transpency
 * \n
 * \param ch :0~1
 * \param  transp :1 enable, 0 disable
 * \n
 * \return 0:sucess, -1:fail
 */
int snx_isp_osd_txt_transp_set(int ch, int transp);

/** \fn int snx_isp_osd_txt_transp_get(int ch, int *transp)
 * \brief get txt transprency status
 * \n
 * \param ch :0~1
 * \param  transp :1 enable, 0 disable
 * \n
 * \return 0:sucess, -1:fail
 */
int snx_isp_osd_txt_transp_get(int ch, int *transp);

/** \fn int snx_isp_osd_edg_transp_set(int ch, int transp)
 * \brief set edg transpency
 * \n
 * \param ch :0~1
 * \param  transp :1 enable, 0 disable
 * \n
 * \return 0:sucess, -1:fail
 */
int snx_isp_osd_edg_transp_set(int ch, int transp);

/** \fn int snx_isp_osd_edg_transp_get(int ch, int *transp)
 * \brief get edg transprency status
 * \n
 * \param ch :0~1
 * \param  transp :1 enable, 0 disable
 * \n
 * \return 0:sucess, -1:fail
 */
int snx_isp_osd_edg_transp_get(int ch, int *transp);


/** \fn int snx_isp_osd_bg_transp_set(int ch, int transp)
 * \brief set background transprency status
 * \n
 * \param ch :0~1
 * \param  transp :1 enable, 0 disable
 * \n
 * \return 0:sucess, -1:fail
 */
int snx_isp_osd_bg_transp_set(int ch, int transp);

/** \fn int snx_isp_osd_bg_transp_get(int ch, int *transp)
 * \brief get background transprency status
 * \n
 * \param ch :0~1
 * \param transp :1 enable, 0 disable
 * \n
 * \return 0:sucess, -1:fail
 */
int snx_isp_osd_bg_transp_get(int ch, int *transp);

/** \fn int snx_isp_osd_position_set(int ch, int x, int y)
 * \brief set font display position
 * \n
 * \param ch :0~1
 * \param x :horital postion to display
 * \param y :vertical postion to display
 * \n
 * \return 0:sucess, -1:fail
 */
int snx_isp_osd_position_set(int ch, int x, int y);

/** \fn int snx_isp_osd_position_get(int ch, int *x, int *y)
 * \brief get font display position
 * \n
 * \param ch :0~1
 * \param x :horizonal postion to display
 * \param y :vertical postion to display
 * \n
 * \return 0:sucess, -1:fail
 */
int snx_isp_osd_position_get(int ch, int *x, int *y);

/** \fn int snx_isp_osd_template_set(int ch, const char *str)
 * \brief set font template
 * \n
 * \param ch :0~1
 * \param str :template for font display
 * \n
 * \return 0:sucess, -1:fail
 */
int snx_isp_osd_template_set(int ch, const char *str);

/** \fn int snx_isp_osd_template_get(int ch, char *str)
 * \brief get font template
 * \n
 * \param ch :0~1
 * \param str :template for font display
 * \n
 * \return 0:sucess, -1:fail
 */
int snx_isp_osd_template_get(int ch, char *str);

/** \fn int snx_isp_osd_font_set(int ch, char *font)
 * \brief set binary font data
 * \n
 * \param ch :0~1
 * \param font :binary font data
 * \n
 * \return 0:sucess, -1:fail
 */
int snx_isp_osd_font_set(int ch, char *font);

/** \fn int snx_isp_osd_width_get(int ch, int *width)
 * \brief get image height
 * \n
 * \param ch :0~1
 * \param width :image width
 * \n
 * \return 0:sucess, -1:fail
 */
int snx_isp_osd_width_get(int ch, int *width);

/** \fn int snx_isp_osd_height_get(int ch, int *height)
 * \brief get image height
 * \n
 * \param ch :0~1
 * \param height :image height
 * \n
 * \return 0:sucess, -1:fail
 */
int snx_isp_osd_height_get(int ch, int *height);


#if defined(CONFIG_SYSTEM_PLATFORM_ST58660FPGA) || defined(CONFIG_SYSTEM_PLATFORM_SN98660) || defined(CONFIG_SYSTEM_PLATFORM_SN98661) || defined(CONFIG_SYSTEM_PLATFORM_SN98670) || defined(CONFIG_SYSTEM_PLATFORM_SN98671) || defined(CONFIG_SYSTEM_PLATFORM_SN98672) || defined (CONFIG_SYSTEM_PLATFORM_SN98293)

int snx_isp_osd_bgd_color_set(int ch, int color);
int snx_isp_osd_bgd_color_get(int ch, int *color);

int snx_isp_osd_line_x_get(int ch, int line, int *x);
int snx_isp_osd_line_x_set(int ch, int line, int x);

int snx_isp_osd_line_y_get(int ch, int line, int *y);
int snx_isp_osd_line_y_set(int ch, int line, int y);

int snx_isp_osd_line_gain_get(int ch, int line, int *gain);
int snx_isp_osd_line_gain_set(int ch, int line, int gain);

int snx_isp_osd_line_txt_get(int ch, int line, char *txt);
int snx_isp_osd_line_txt_set(int ch, int line, const char *txt);

int snx_isp_osd_templatew_get(int ch, unsigned short *txt);
int snx_isp_osd_templatew_set(int ch, const unsigned short *txt);

int snx_isp_osd_line_txtw_get(int ch, int line, unsigned short *txt);
int snx_isp_osd_line_txtw_set(int ch, int line, const unsigned short *txt);

int snx_isp_osd_size_get(int ch, int *size);
int snx_isp_osd_size_set(int ch, int size);

#endif


/** @} */


/**
 * \defgroup A5 Sensor Common Functions
 * \n
 * @{
 */

/** \fn int snx_isp_sensor_mirror_set(int enable)
 * \brief enable/disable sensor mirror
 * \n
 * \param enable :1 enable, 0 disable
 * \n
 * \return 0:sucess, -1:fail
 */
int snx_isp_sensor_mirror_set(int enable);

/** \fn int snx_isp_sensor_mirror_get(int *enable)
 * \brief get sensor mirror enable/disable status
 * \n
 * \param enable :1 enable, 0 disable
 * \n
 * \return 0:sucess, -1:fail
 */
int snx_isp_sensor_mirror_get(int *enable);

/** \fn int snx_isp_sensor_flip_set(int enable)
 * \brief enable/disable sensor flip 
 * \n
 * \param enable :1 enable, 0 disable
 * \n
 * \return 0:sucess, -1:fail
 */
int snx_isp_sensor_flip_set(int enable);

/** \fn int snx_isp_sensor_flip_get(int *enable)
 * \brief get sensor flip enable/disable status
 * \n
 * \param enable :1 enable, 0 disable
 * \n
 * \return 0:sucess, -1:fail
 */
int snx_isp_sensor_flip_get(int *enable);

/** \fn int snx_isp_sensor_aec_set(int enable)
 * \brief enable/disable sensor auto exposure
 * \n
 * \param enable :1 enable, 0 disable
 * \n
 * \return 0:sucess, -1:fail
 */
int snx_isp_sensor_aec_set(int enable);

/** \fn int snx_isp_sensor_aec_get(int *enable)
 * \brief get sensor auto exposure enable/disable status
 * \n
 * \param enable :1 enable, 0 disable
 * \n
 * \return 0:sucess, -1:fail
 */
int snx_isp_sensor_aec_get(int *enable);

/** \fn int snx_isp_sensor_exposure_set(int val)
 * \brief set sensor manual exposure value
 * \n
 * \param val :0x1~0xffff
 * \n
 * \return 0:sucess, -1:fail
 */
int snx_isp_sensor_exposure_set(int val);

/** \fn int snx_isp_sensor_exposure_get(int *val)
 * \brief get sensor manual exposure value
 * \n
 * \param val :0x1~0xffff
 * \n
 * \return 0:sucess, -1:fail
 */
int snx_isp_sensor_exposure_get(int *val);

/** \fn int snx_isp_sensor_awb_set(int enable)
 * \brief enable/disable sensor auto white balance
 * \n
 * \param enable :1 enable, 0 disable
 * \n
 * \return 0:sucess, -1:fail
 */
int snx_isp_sensor_awb_set(int enable);

/** \fn int snx_isp_sensor_awb_get(int *enable)
 * \brief get sensor auto white balance enable/disable status
 * \n
 * \param enable :1 enable, 0 disable
 * \n
 * \return 0:sucess, -1:fail
 */
int snx_isp_sensor_awb_get(int *enable);

/** \fn int snx_isp_sensor_gain_set(int val)
 * \brief set sensor exposure gain
 * \n
 * \param val :0x40~0x1900
 * \n
 * \return 0:sucess, -1:fail
 */
int snx_isp_sensor_gain_set(int val);

/** \fn int snx_isp_sensor_gain_get(int *val)
 * \brief get sensor exposure gain
 * \n
 * \param val :0x40~0x1900
 * \n
 * \return 0:sucess, -1:fail
 */
int snx_isp_sensor_gain_get(int *val);

/** \fn int snx_isp_sensor_redGain_set(int val)
 * \brief set red gain
 * \n
 * \param val :0x0000~0xffff
 * \n
 * \return 0:sucess, -1:fail
 */
int snx_isp_sensor_redGain_set(int val);

/** \fn int snx_isp_sensor_redGain_get(int *val)
 * \brief get red gain
 * \n
  * \param val :0x0000~0xffff
 * \n
 * \return 0:sucess, -1:fail
 */
int snx_isp_sensor_redGain_get(int *val);

/** \fn int snx_isp_sensor_greenGain_set(int val)
 * \brief set green gain
 * \n
 * \param val :0x0000~0xffff
 * \n
 * \return 0:sucess, -1:fail
 */
int snx_isp_sensor_greenGain_set(int val);

/** \fn int snx_isp_sensor_greenGain_get(int *val)
 * \brief get green gain
 * \n
 * \param val :0x0000~0xffff
 * \n
 * \return 0:sucess, -1:fail
 */
int snx_isp_sensor_greenGain_get(int *val);

/** \fn int snx_isp_sensor_blueGain_set(int val)
 * \brief set blue gain
 * \n
 * \param val :0x0000~0xffff
 * \n
 * \return 0:sucess, -1:fail
 */
int snx_isp_sensor_blueGain_set(int val);

/** \fn int snx_isp_sensor_blueGain_get(int *val)
 * \brief get blue gain
 * \n
 * \param val :0x0000~0xffff
 * \n
 * \return 0:sucess, -1:fail
 */
int snx_isp_sensor_blueGain_get(int *val);

/** \fn int snx_isp_sensor_name_get(char *name)
 * \brief get current sensor name
 * \n
 * \param name :sensor name
 * \n
 * \return 0:sucess, -1:fail
 */
int snx_isp_sensor_name_get(char *name);

#if defined(CONFIG_SYSTEM_PLATFORM_ST58660FPGA) || defined(CONFIG_SYSTEM_PLATFORM_SN98660) || defined(CONFIG_SYSTEM_PLATFORM_SN98661) || defined(CONFIG_SYSTEM_PLATFORM_SN98670) || defined(CONFIG_SYSTEM_PLATFORM_SN98671) || defined(CONFIG_SYSTEM_PLATFORM_SN98672) || defined (CONFIG_SYSTEM_PLATFORM_SN98293)

int snx_isp_sensor_mode_enum(int index, int *width, int *height, int *rate, int *x, int *y, int *cw, int *ch);

int snx_isp_sensor_mode_get(int *width, int *height, int *rate);

int snx_isp_sensor_mode_set(int width, int height, int rate);

int snx_isp_sensor_max_resolution_get(int *width, int *height);

int snx_isp_sensor_lense_set(int lense);

int snx_isp_sensor_lense_get(int *lense);

/** \fn int snx_isp_sensor_suspend_set(int status)
 * \brief tune sensor suspend mode
 * \n
 * \param status :1 sensor power on, 0 sensor power off
 * \n
 * \return 0:sucess, -1:fail
 */
int snx_isp_sensor_suspend_set(int status);

/** \fn int snx_isp_sensor_suspend_get(int *status)
 * \brief get sensor suspend mode status
 * \n
 * \param status :1 sensor power on, 0 sensor power off
 * \n
 * \return 0:sucess, -1:fail
 */
int snx_isp_sensor_suspend_get(int *status);

#endif


/** @} */


/**
 * \defgroup A6 Mirror & Flip Functions
 * \n
 * @{
 */

/** \fn int snx_isp_mirror_flip_mode_set(int mode)
 * \brief set isp mirror/flip
 * \n
 * \param mode :0 normal, 1 flip, 2 mirror, 3 mirror&flip
 * \n
 * \return 0:sucess, -1:fail
 */
int snx_isp_mirror_flip_mode_set(int mode);

/** \fn int snx_isp_mirror_flip_mode_get(int *mode)
 * \brief get isp mirror/flip status
 * \n
 * \param mode :0 normal, 1 flip, 2 mirror, 3 mirror&flip
 * \n
 * \return 0:sucess, -1:fail
 */
int snx_isp_mirror_flip_mode_get(int *mode);

/** @} */


/**
 * \defgroup A7 AE & AWB Functions
 * \n
 * @{
 */

/** \fn int snx_isp_aec_enable_set(int enable)
 * \brief enable/disable isp auto exposure
 * \n
 * \param enable :1 enable, 0 disable
 * \n
 * \return 0:sucess, -1:fail
 */
int snx_isp_aec_enable_set(int enable);

/** \fn int snx_isp_aec_enable_get(int *enable)
 * \brief get isp auto exposure enable/disable status
 * \n
 * \param enable :1 enable, 0 disable
 * \n
 * \return 0:sucess, -1:fail
 */
int snx_isp_aec_enable_get(int *enable);

/** \fn int snx_isp_light_frequency_set(int val)
 * \brief set light_frequency value
 * \n
 * \param val :50 or 60 (Hz)
 * \n
 * \return 0:sucess, -1:fail
 */
int snx_isp_light_frequency_set(int val);

/** \fn int snx_isp_light_frequency_get(int *val)
 * \brief get light_frequency value
 * \n
 * \param val :50 or 60 (Hz)
 * \n
 * \return 0:sucess, -1:fail
 */
int snx_isp_light_frequency_get(int *val);

/** \fn int snx_isp_light_exposure_time_set(int val)
 * \brief set light_exposure value
 * \n
 * \param val :0x0~0xffff
 * \n
 * \return 0:sucess, -1:fail
 */
int snx_isp_light_exposure_time_set(int val);

/** \fn int snx_isp_light_exposure_time_get(int *val)
 * \brief get light_exposure value
 * \n
 * \param val :0~0xffff
 * \n
 * \return 0:sucess, -1:fail
 */
int snx_isp_light_exposure_time_get(int *val);

// Modify By Raymond
//int snx_isp_fps_get(int *val);
//int snx_isp_fps_set(int val);
int snx_isp_fps_get(int ch, int *val);
int snx_isp_fps_set(int ch, int val);
int snx_isp_absy_get(int *val);
int snx_isp_offset_get(int *val);
int snx_isp_offset_set(int *val);


/** \fn int snx_isp_awb_enable_set(int enable)
 * \brief enable/disable isp auto white balance
 * \n
 * \param enable :1 enable, 0 disable
 * \n
 * \return 0:sucess, -1:fail
 */
int snx_isp_awb_enable_set(int enable);

/** \fn int snx_isp_awb_enable_get(int *enable)
 * \brief get isp auto white balance enable/disable status
 * \n
 * \param enable :1 enable, 0 disable
 * \n
 * \return 0:sucess, -1:fail
 */
int snx_isp_awb_enable_get(int *enable);

/** @} */

/**
 * \defgroup A8 ISP DRC Functions
 * \n
 * @{
 */

/** \fn int snx_isp_drc_status_get(int *status)
 * \brief get drc enable/disable status
 * \n
 * \param status :1 enable, 0 disable
 * \n
 * \return 0:sucess, -1:fail
 */
int snx_isp_drc_status_get(int *status);

/** \fn int snx_isp_drc_status_set(int status)
 * \brief enable/disable drc
 * \n
 * \param status :1 enable, 0 disable
 * \n
 * \return 0:sucess, -1:fail
 */
int snx_isp_drc_status_set(int status);

/** \fn int snx_isp_drc_value_set(int val)
 * \brief set drc intensity value
 * \n
 * \param val :1~16
 * \n
 * \return 0:sucess, -1:fail
 */
int snx_isp_drc_value_set(int val);

/** \fn int snx_isp_drc_value_get(int *val)
 * \brief get drc intensity value
 * \n
 * \param val :1~16
 * \n
 * \return 0:sucess, -1:fail
 */
int snx_isp_drc_value_get(int *val);

/** @} */

/**
 * \defgroup A9 ISP Private Mask Functions
 * \n
 * @{
 */

/** \fn int snx_isp_pm_enable_set(int enable)
 * \brief enable/disable private mask
 * \n
 * \param enable :1 enable, 0 disable
 * \n
 * \return 0:sucess, -1:fail
 */
int snx_isp_pm_enable_set(int enable);

/** \fn int snx_isp_pm_enable_get(int *enable)
 * \brief get private mask enable/disable status
 * \n
 * \param enable :1 enable, 0 disable
 * \n
 * \return 0:sucess, -1:fail
 */
int snx_isp_pm_enable_get(int *enable);

/** \fn int snx_isp_pm_color_set(int color)
 * \brief set private mask color, rgb888 format
 * \n
 * \param color :0x000000~0xffffff
 * \n
 * \return 0:sucess, -1:fail
 */
int snx_isp_pm_color_set(int color);

/** \fn int snx_isp_pm_color_get(int *color)
 * \brief get private mask color, rgb888 format
 * \n
 * \param color :0x000000~0xffffff
 * \n
 * \return 0:sucess, -1:fail
 */
int snx_isp_pm_color_get(int *color);

/** \fn int snx_isp_pm_area_set(unsigned int area[])
 * \brief set private mask area
 * \n
 * \param area :192bits, 6*32bits
 * \n
 * \return 0:sucess, -1:fail
 */
int snx_isp_pm_area_set(unsigned int area[]);

/** \fn int snx_isp_pm_area_get(unsigned int area[])
 * \brief get private mask area
 * \n
 * \param area :192bits, 6*32bits
 * \n
 * \return 0:sucess, -1:fail
 */
int snx_isp_pm_area_get(unsigned int area[]);

/** @} */


/**
 * \defgroup A10 ISP Filter Functions
 * \n
 * @{
 */

/** \fn int snx_isp_filter_contrast_set(int val)
 * \brief set contrast value
 * \n
 * \param val :0~63
 * \n
 * \return 0:sucess, -1:fail
 */
int snx_isp_filter_contrast_set(int val);

/** \fn int snx_isp_filter_contrast_get(int *val)
 * \brief get contrast value
 * \n
 * \param val :0~63
 * \n
 * \return 0:sucess, -1:fail
 */
int snx_isp_filter_contrast_get(int *val);

/** \fn int snx_isp_filter_sharpness_set(int val)
 * \brief set sharpness value
 * \n
 * \param val :0~6
 * \n
 * \return 0:sucess, -1:fail
 */
int snx_isp_filter_sharpness_set(int val);

/** \fn int snx_isp_filter_sharpness_get(int *val)
 * \brief get sharpness value
 * \n
 * \param val :0~6
 * \n
 * \return 0:sucess, -1:fail
 */
int snx_isp_filter_sharpness_get(int *val);

/** \fn int snx_isp_filter_saturation_set(int val)
 * \brief set saturation value
 * \n
 * \param val :0~127
 * \n
 * \return 0:sucess, -1:fail
 */
int snx_isp_filter_saturation_set(int val);

/** \fn int snx_isp_filter_saturation_get(int *val)
 * \brief get saturation value
 * \n
 * \param val :0~127
 * \n
 * \return 0:sucess, -1:fail
 */
int snx_isp_filter_saturation_get(int *val);

/** \fn int snx_isp_filter_hue_set(int val)
 * \brief set hue value
 * \n
 * \param val :0~359
 * \n
 * \return 0:sucess, -1:fail
 */
int snx_isp_filter_hue_set(int val);

/** \fn int snx_isp_filter_hue_get(int *val)
 * \brief get hue value
 * \n
 * \param val :0~359
 * \n
 * \return 0:sucess, -1:fail
 */
int snx_isp_filter_hue_get(int *val);

/** \fn int snx_isp_filter_brightness_set(int val)
 * \brief set brightness value
 * \n
 * \param val :0~127
 * \n
 * \return 0:sucess, -1:fail
 */
int snx_isp_filter_brightness_set(int val);

/** \fn int snx_isp_filter_brightness_get(int *val)
 * \brief get brightness value
 * \n
 * \param val :0~127
 * \n
 * \return 0:sucess, -1:fail
 */
int snx_isp_filter_brightness_get(int *val);

/** \fn int snx_isp_af_w0_sum_get(int *val)
 * \brief get AF w0 sum value
 * \n
 * \param val :32bits
 * \n
 * \return 0:sucess, -1:fail
 */
int snx_isp_af_w0_sum_get(int *val);

/** \fn int snx_isp_af_w1_sum_get(int *val)
 * \brief get AF w1 sum value
 * \n
 * \param val :32bits
 * \n
 * \return 0:sucess, -1:fail
 */
int snx_isp_af_w1_sum_get(int *val);

/** \fn int snx_isp_af_w0_thd_set(int val)
 * \brief set AF w0 threshold value
 * \n
 * \param val :8bits
 * \n
 * \return 0:sucess, -1:fail
 */
int snx_isp_af_w0_thd_set(int val);

/** \fn int snx_isp_af_w0_thd_get(int *val)
 * \brief get AF w0 threshold value
 * \n
 * \param val :8bits
 * \n
 * \return 0:sucess, -1:fail
 */
int snx_isp_af_w0_thd_get(int *val);

/** \fn int snx_isp_af_w1_thd_set(int val)
 * \brief set AF w1 threshold value
 * \n
 * \param val :8bits
 * \n
 * \return 0:sucess, -1:fail
 */
int snx_isp_af_w1_thd_set(int val);

/** \fn int snx_isp_af_w1_thd_get(int *val)
 * \brief get AF w1 threshold value
 * \n
 * \param val :8bits
 * \n
 * \return 0:sucess, -1:fail
 */
int snx_isp_af_w1_thd_get(int *val);

/** \fn int snx_isp_af_w0_en_set(int val)
 * \brief set AF w0 enable value
 * \n
 * \param val :1 enable, 0 disable
 * \n
 * \return 0:sucess, -1:fail
 */
int snx_isp_af_w0_en_set(int val);

/** \fn int snx_isp_af_w0_en_get(int *val)
 * \brief get AF w0 enable value
 * \n
 * \param val :1 enable, 0 disable
 * \n
 * \return 0:sucess, -1:fail
 */
int snx_isp_af_w0_en_get(int *val);

/** \fn int snx_isp_af_w1_en_set(int val)
 * \brief set AF w1 enable value
 * \n
 * \param val :1 enable, 0 disable
 * \n
 * \return 0:sucess, -1:fail
 */
int snx_isp_af_w1_en_set(int val);

/** \fn int snx_isp_af_w1_en_get(int *val)
 * \brief get AF w1 enable value
 * \n
 * \param val :1 enable, 0 disable
 * \n
 * \return 0:sucess, -1:fail
 */
int snx_isp_af_w1_en_get(int *val);


/** @} */

/** @} */ //defgroup isp_mid ISP middleware modules

#ifdef __cplusplus
}
#endif



#endif /*__ISP_LIB_API_H__*/
