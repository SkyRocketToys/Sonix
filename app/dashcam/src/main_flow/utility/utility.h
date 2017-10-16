#ifndef __APP_UTILITY_H__ 
#define __APP_UTILITY_H__ 
#include <sys_clock.h>

#define WIFI_PWD_LENTH 			8    	/**< length of wifi password*/
#define WIFI_SSID_MAX_LENTH 	32   	/**< max length of wifi ssid*/
#define MAX_RECORD_LEN_SIG  	(9999)
#define HDR_LEN 				14
#define FILE_DIR_LEN 			62 		/**< limitation directory length for Carcam*/  

#define PREVIEW_AUDIO_ON	1
#define PREVIEW_AUDIO_OFF	0

#ifdef CONFIG_APP_DRONE
#define ENGINNER_MODE   		0
#endif
/**
* @brief structure for connected infomation
*/
typedef struct _ConnItem{
	uint32_t ip_addr;		/**< ip address for connection*/
	char mac[20];
	struct _ConnItem *next;	/**< point to next connected item*/
}ConnItem_t;


typedef struct _time_zone_tbl{
	char time_zone[10];
	uint8_t num;
}time_zone_tbl_t;


enum SD_DIR_INFO{
	U_SD_ROOT = 1,
	U_SD_RECORD,
	U_SD_PROTECT,
	U_SD_PICTURE,
	U_SD_TIMELAPSE,
	U_SD_THUMBNAIL,
};

//Wifi API
int app_uti_set_wifi_channel(int channel);
int app_uti_set_wifi_pwd(char* pwd);
int app_uti_set_wifi_ssid(char* ssid);
#ifdef ENGINNER_MODE
int app_uti_set_wifi_param(char *ssid, char *pwd, int channel, int retry_count);
#else
int app_uti_set_wifi_param(char *ssid, char *pwd, int channel);
#endif

//Video API
int app_uti_get_capability(void);
void app_uti_get_preview_params(unsigned int *width, unsigned int *height, unsigned int *ucfps, unsigned int *ucgop, unsigned int *uibps);
int app_uti_wid_hight_to_resolution(int width, int height);
int app_uti_check_all_task_is_running(void);
int app_uti_check_resolution_change(int resolution, int *width, int *height);

int app_uti_set_video_wdr(int wdr_en);
int app_uti_set_video_mirror(int mirror);
int app_uti_set_video_flip(int flip);
int app_uti_get_video_status(int *wdr_status, int *mirror_status, int *flip_status, int *usbdclassmode, int *capability, unsigned int *width, unsigned int *height, unsigned int *ucfps, unsigned int *ucgop, unsigned int *uibps, int *resolution);
int app_uti_set_video_ext_qp(int pre_ext_pframe_num, int pre_ext_qp_range, int pre_ext_qp_max, int pre_ext_upper_pframe, int pre_ext_upper_pframe_dup1, int pre_qp_max, int pre_qp_min);
int app_uti_set_video_preview_time(int resume, int suspend);
int app_uti_set_preview_bitrate(unsigned int bitrate);
int app_uti_set_preview_fps(int fps);
int app_uti_set_preview_resolution(int resolution);
int app_uti_set_power_frequency(int power_frequency);
int app_uti_get_video_ext_qp(int *pre_ext_pframe_num, int *pre_ext_qp_range, int *pre_ext_qp_max, int *pre_ext_upper_pframe,int *pre_ext_upper_pframe_dup1, int *pre_qp_max, int *pre_qp_min);
int app_uti_get_video_preview_time(int *resume, int *suspend);
int app_uti_take_picture(int pic_num);
int app_uti_get_preview_audio_mode(void);
int app_uti_set_preview_audio_mode(int flag);

//SD Manager API
int app_uti_chk_sd_state(void);
int app_uti_get_format_status(void);
int app_uti_is_format_running(void);
int app_uti_get_sd_status(void);
int app_uti_get_sd_space(int *fre_percent, int *tot_GByte);
int app_uti_get_sd_format_status(void);
void app_uti_set_sd_format(void *args);
int app_uti_create_sd_format_task(void *args);
int app_uti_set_sd_dir_info(int type, char *name);
int app_uti_get_root_dir_name(char *name);
int app_uti_get_record_dir_name(char *name);
int app_uti_get_protect_dir_name(char *name);
int app_uti_get_picture_dir_name(char *name);
int app_uti_get_timelapse_dir_name(char *name);
int app_uti_get_thumbnail_dir_name(char *name);
int app_uti_is_test_running(void);
int app_uti_get_sd_test_status(void);
void app_uti_get_test_report(char *avg_write, char *avg_read);
int app_uti_create_sd_test_task(void *args);
//Device Manager API
int app_uti_sync_time(system_date_t *time);
int app_uti_get_timezone_num(char *time_zone);
int app_uti_get_iq_version(int *version);
int app_uti_get_device_engmode_params(int *tx_retry, int *udp_type);
int app_uti_get_device_params(int *version, int *gsensor_sensitivity, unsigned char *channel, unsigned char *ssid, unsigned char *pwd, int *powerfrequency, char *dev_ver, int *wifi_mode);
int app_uti_get_osd_status(int *osd_enable, unsigned char **osd_str, int *nvramnodata, unsigned int *len);
int app_uti_set_nvram_to_default(void);
int app_uti_set_class_mode(int usbdclassmode);
int app_uti_set_osd_onoff(int osd_enable);
int app_uti_set_gsensor_sensitivity(int sensitivity);

//Record API
int app_uti_enable_record(void);
int app_uti_disable_record(void);
int app_uti_get_record_status(int *rec_status, int *rec_running, unsigned int *recordfps, unsigned int *recordgop, unsigned int *recordbps, int *recordresolution, int *capability, int *level, int *recordlength, int *cycle, int *protectlength);
int app_uti_get_record_ext_qp(int *rec_ext_pframe_num, int *rec_ext_qp_range, int *rec_ext_qp_max, int *rec_ext_upper_pframe, int *rec_qp_max, int *rec_qp_min);
int app_uti_set_record_status(int record_status);
int app_uti_set_record_length(int minutes);
int app_uti_set_protect_length(int second);
int app_uti_set_record_loop(int status);
int app_uti_set_record_audio_volume(int vol_level);
int app_uti_set_record_ext_qp(int rec_ext_pframe_num, int rec_ext_qp_range, int rec_ext_qp_max, int rec_ext_upper_pframe, int rec_qp_max, int rec_qp_min);

//Transmit API
//int app_uti_get_indexfile(int list_src, int *port, int *get_seed, int *is_full, SocketItem_t *psocket);
//int app_uti_chk_and_add_connection(int filetype, SocketItem_t *psocket, int conn_state);
//int app_uti_chk_and_delete_connection(int filetype, SocketItem_t *psocket);
//int app_uti_add_file_upload(char *fname, int number, int pos, int filetype, int *port, int *file_size, SocketItem_t *psocket);
//int app_uti_stop_file_upload(int filetype, char *fname, SocketItem_t *psocket);
int app_uti_delete_file(int filetype, char *fname);
int app_uti_del_protect_file(char *abs_path, char *fname);
int app_uti_del_picture_file(char *abs_path, char *fname);
int app_uti_del_record_file(char *abs_path, char *fname);
int app_uti_set_stream_uvc(int view_enable);
void app_uti_set_stream_preview(int enable);
void app_uti_restart_playback(int force_record);
//int app_uti_set_stream_playback(char *fname, char *frename, int filetype, SocketItem_t *psocket);
//int app_uti_del_stream_preview(SocketItem_t *psocket);
//int app_uti_del_stream_playback(SocketItem_t *psocket);
int modify_fw_upgrade_status(int val);
//int app_uti_dwnlod_fw_create_socket(uint16_t *port, int filesize, SocketItem_t *psocket);
//int app_uti_get_fw_dwnlod_stat(SocketItem_t *psocket);


int app_util_init(void);
int app_util_uninit(void);


#endif
