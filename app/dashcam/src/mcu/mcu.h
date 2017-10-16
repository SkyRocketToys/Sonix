/**
 * @file
 * this is mcu header file
 * @author CJ
 */

#ifndef __MCU_H__
#define __MCU_H__

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

void mcu_set_err_flag(enum SYSTEM_ERROR_FLAG flag);
void mcu_clear_err_flag(enum SYSTEM_ERROR_FLAG flag);
void mcu_reset_err_flag(void);
void mcu_init(void);
void mcu_uninit(void);
void mcu_connect_notask(void);
#endif
