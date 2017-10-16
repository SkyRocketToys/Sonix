/**
 * @file
 * this is timestamp osd control header file
 * @author CJ
 */

#ifndef __OSD_CTRL_H__
#define __OSD_CTRL_H__

/**
* @brief format of osd data stamp.
*/
enum OSD_DS_FORMAT {
	DS_STRING = 0,
	DS_BMP,
};

/**
* @brief structure for osd data stamp infomation.
*/
typedef struct _OSD_DS_INFO {
	uint8_t vc_enc_num;
	uint8_t enable;
	uint8_t source_format;
	uint8_t mode;
	uint8_t weight;
	uint8_t scale;
	uint8_t transparent;
	uint32_t transparent_value;
	uint8_t bmp_rounding;
	uint32_t bmp_thres;
	uint32_t bmp_size;
	uint32_t pos_x;
	uint32_t pos_y;
	void *bmp_ptr;
	struct snx_vc *vc;
} osd_ds_info_t;

//------- for osd time stamp -------
int timestamp_osd_init(void);
int timestamp_osd_uninit(void);

//------- for osd data stamp -------
int osd_ds_init(uint8_t ch, osd_ds_info_t *ds_info, struct snx_vc *vc);
int osd_ds_set_ch(osd_ds_info_t *ds_info, uint8_t ch);
int osd_ds_set_vc_t(osd_ds_info_t *ds_info, struct snx_vc *vc);
int osd_ds_set_source_format(osd_ds_info_t *ds_info, enum OSD_DS_FORMAT source_format);
int osd_ds_set_mode(osd_ds_info_t *ds_info, uint8_t mode);
int osd_ds_set_weight(osd_ds_info_t *ds_info, uint8_t weight);
int osd_ds_set_scale(osd_ds_info_t *ds_info, uint8_t scale);
int osd_ds_set_transparent(osd_ds_info_t *ds_info, uint8_t transparent);
int osd_ds_set_bmp_rounding(osd_ds_info_t *ds_info, uint8_t bmp_rounding);
int osd_ds_set_bmp_threshold(osd_ds_info_t *ds_info, uint32_t bmp_thres);
int osd_ds_set_bmp_pos(osd_ds_info_t *ds_info, uint32_t pos_x, uint32_t pos_y);
int osd_ds_set_bmp(osd_ds_info_t *ds_info, void *bmp_ptr, uint32_t bmp_size);
int osd_ds_set_en(osd_ds_info_t *ds_info, uint8_t enable);

int chk_isdup_set_osd_gain(void);
#endif
