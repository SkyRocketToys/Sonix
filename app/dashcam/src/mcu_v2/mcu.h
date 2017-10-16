/**
 * @file
 * this is mcu header file
 * @author CJ
 */

#ifndef __MCU_H__
#define __MCU_H__

#include <libmid_rec/avi_header.h>

/**
* @brief flag about 660 status transmit to mcu.
*/
enum SYSTEM_ERROR_FLAG{
	ERR_SDCARD = 0,				/**<  SD card initial fail */
	ERR_SF,						/**<  SF initial fail */
	ERR_SENSOR_ISP,				/**<  Sensor initial fail */
	ERR_WIFI,					/**<  Wifi initial fail */
	ERR_NVRAM,					/**<  Nvram initial fail */
	FIRMWARE_UPGRADE = 0x10,		/**<  660 doing firmware upgrade and do not reset 660 */
	ALL_RESET = 31,				/**<  System don't work and reset 660 immediately */
};

typedef struct gps_info_table {
	AVIGPSINFO_t *avi_gps;
	unsigned short sn;
} gps_info_t;

typedef struct g_sensor_info_table {
	AVIGSENSORINFO_t *avi_g_sensor;
	unsigned short sn;
} g_sensor_info_t;

void mcu_set_err_flag(enum SYSTEM_ERROR_FLAG flag);
void mcu_clear_err_flag(enum SYSTEM_ERROR_FLAG flag);
void mcu_reset_err_flag(void);
void mcu_init(void);
void mcu_uninit(void);
void mcu_connect_notask(void);
void app_mcu_cmds_register_empty(void);
void set_gsensor_sensitivity(int val);
void wifi_channelinfoset(int val);			//CSL
int wifi_readchannelinfo(void);				//CSL
uint8_t fbgps_getinof(uint8_t* pfbgps);		//CSL FJW
uint8_t fbnavi_getstatus(uint8_t* pfbnavi); //CSL FJW
uint8_t* app_getfbgps(void);				//CSL FJW
uint8_t* app_getfbnavista(void);			//CSL FJW
uint8_t* app_sendflightplan_tofb(uint8_t* plandata,int datalen); //CSL FJW


AVIGPSINFO_t *get_avi_gps_info(void);
unsigned short get_gps_sn(void);
gps_info_t *get_gps_info_table(void);

g_sensor_info_t *get_g_sensor_info_table(void);
AVIGSENSORINFO_t *get_avi_g_sensor_info(void);
unsigned short get_g_sensor_sn(void);
void set_flight_image_version(unsigned short);

//unsigned char Get_Receive_UART_Data(unsigned char val);		//CSL
int Get_Receive_UART_Data(int val);								//CSL
unsigned Get_Receive_UART_Size(void);

#endif
