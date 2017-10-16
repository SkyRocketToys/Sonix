/**
 * @file
 * this is audio driver interface file, include this file before use
 * audio_dri.h
 * @author yanjie_yang
 */
 
#ifndef _AUDIO_DRI_H_
#define _AUDIO_DRI_H_

#include <FreeRTOS.h>
#include <nonstdlib.h>
#include <semphr.h>

#define AUD_CAP_STREAM			0
#define AUD_PLY_STREAM			1

#define BIT(x)				(1UL << (x))

/**
 * \defgroup AUDIO_FORMAT The format of audio
 * @{
 */
#define AUD_FORMAT_S8			BIT(0)
#define AUD_FORMAT_U8			BIT(1)
#define AUD_FORMAT_S16_LE		BIT(2)
#define AUD_FORMAT_U16_LE		BIT(3)
#define AUD_FORMAT_HW_A_LAW		BIT(28)
#define AUD_FORMAT_HW_MS_ADPCM		BIT(29)
/** @} */

/**
* @brief interface function - return the bits per a PCM sample
* param format sample format
* @return bits per sample, a negative error code if not applicable
*/
int32_t audio_format_width(uint32_t format);

/**
 * \defgroup AUDIO_STATE The state of audio status machine
 * @{
 */
#define AUD_STATE_CLOSE			0
#define AUD_STATE_OPEN			1
#define AUD_STATE_SETUP			2
#define AUD_STATE_RUNNING		3
#define AUD_STATE_STOP			4
#define AUD_STATE_XRUN			5
#define AUD_STATE_PREPARED		6
#define AUD_STATE_DRAIN			7
/** @} */

struct audio_dri_st;
struct params_st;

/**
* @brief interface function - audio driver module init function
* @return return 0 if success
*/
int32_t audio_dri_init(void);

#ifdef UNINIT_FUNC
/**
* @brief interface function - audio driver module uninit function
*/
void audio_dri_uninit(void);
#endif

/**
* @brief interface function - open audio device
* @param audio_dri structure for audio driver
* @param type audio device type
* @return return 0 if success
*/
int32_t audio_dri_open(struct audio_dri_st **audio_dri, int32_t type);

/**
* @brief interface function - close audio device
* @param audio_dri structure for audio driver
* @return return 0 if success
*/
int32_t audio_dri_close(struct audio_dri_st *audio_dri);

/**
* @brief interface function - start audio device capture or playback
* @param audio_dri structure for audio driver
* @return return 0 if success
*/
int32_t audio_dri_start(struct audio_dri_st *audio_dri);

/**
* @brief interface function - stop audio device capture or playback
* @param audio_dri structure for audio driver
* @return return 0 if success
* @warning this function stops the audio device immediately. The pending samples on the buffer are ignored.
*/
int32_t audio_dri_stop(struct audio_dri_st *audio_dri);

/**
* @brief interface function - get the status of audio device
* @param audio_dri structure for audio driver
* @param pstatus pointer to the status of audio device
* @return return 0 if success
*/
int32_t audio_dri_status(struct audio_dri_st *audio_dri, int32_t *pstatus);

/** \struct buf_info_st
 * \brief buf_info_st
 * \n
 * \n addr: Pointer to audio device buffer start address
 * \n size: The size of audio device buffer
 * \n period_size: Each the 'period_size' bytes audio data is processed(capture/playback), audio driver call 'audio_period_callback' function once.
 */
struct buf_info_st
{
	uint8_t *addr;
	uint32_t size;
	uint32_t period_size;
};

/**
* @brief interface function - get the information of audio device buffer.
* @param audio_dri structure for audio driver
* @param buf_info pointer to the information of audio device buffer.
* @return return 0 if success
*/
int32_t audio_dri_buf_get_info(struct audio_dri_st *audio_dri, struct buf_info_st *buf_info);

/**
* @brief interface function - get the offset of hardware pointer in audio device buffer.
* @param audio_dri structure for audio driver
* @param buf_info pointer to the offset
* @return return 0 if success
*/
int32_t audio_dri_buf_get_hw_offset(struct audio_dri_st *audio_dri, uint32_t *offset);

typedef int32_t (*audio_period_callback)(int32_t type, void *param);
/**
* @brief interface function - set audio period elapsed callback function.
* @param audio_dri structure for audio driver
* @param callback period elapsed callback function
* @param param the parameter of period elapsed callback function
* @return return 0 if success
*/
int32_t audio_dri_set_period_elapsed_callback(struct audio_dri_st *audio_dri, audio_period_callback callback, void *param);

/**
* @brief interface function - allocate the memory of audio params structure
* @param params structure for audio params
* @return return 0 if success
*/
int32_t audio_dri_params_alloca(struct params_st **params);

/**
* @brief interface function - free the memory of audio params structure
* @param params structure for audio params
* @return return 0 if success
*/
int32_t audio_dri_params_free(struct params_st *params);

/**
* @brief interface function - set the audio params to audio device
* @param audio_dri structure for audio driver
* @param params structure for audio params
* @return return 0 if success
*/
int32_t audio_dri_set_params(struct audio_dri_st *audio_dri, struct params_st *params);

/**
* @brief interface function - get the audio params of audio device
* @param audio_dri structure for audio driver
* @param params structure for audio params
* @return return 0 if success
*/
int32_t audio_dri_get_params(struct audio_dri_st *audio_dri, struct params_st *params);

/**
* @brief interface function - set the sample rate of audio device
* @param audio_dri structure for audio driver
* @param params structure for audio params
* @param rate sample rate
* @return return 0 if success
*/
int32_t audio_dri_params_set_rate(struct audio_dri_st *audio_dri, struct params_st *params, uint32_t rate);

/**
* @brief interface function - set the sample format of audio device
* @param audio_dri structure for audio driver
* @param params structure for audio params
* @param format sample format
* @return return 0 if success
*/
int32_t audio_dri_params_set_format(struct audio_dri_st *audio_dri, struct params_st *params, uint32_t format);


/**
* @brief interface function - get the sample rate of audio device
* @param params structure for audio params
* @param rate sample rate
* @return return 0 if success
*/
int32_t audio_dri_params_get_rate(struct params_st *params, uint32_t *rate);

/**
* @brief interface function - get the sample format of audio device
* @param params structure for audio params
* @param format sample format
* @return return 0 if success
*/
int32_t audio_dri_params_get_format(struct params_st *params, uint32_t *format);


/** \struct ctrl_info_st
 * \brief ctrl_info_st
 * \n
 * \n min: The min value of control iterm
 * \n max: The max value of control iterm
 */
struct ctrl_info_st
{
	int32_t min;
	int32_t max;
};
#define AUD_CTRL_CMD_BIT			0
#define AUD_BUF_MGR_CMD_BIT			6
#define AUD_SUB_MOD_CMD_BIT			10
#define AUD_CTRL_CMD_MASK			(BIT(0)|BIT(1)|BIT(2)|BIT(3)|BIT(4)|BIT(5))
#define AUD_BUF_MGR_CMD_MASK			(BIT(6)|BIT(7)|BIT(8)|BIT(9))
#define AUD_SUB_MOD_CMD_MASK			(BIT(10)|BIT(11)|BIT(12)|BIT(13)|BIT(14)|BIT(15))

/**
 * \defgroup AUDIO_CMD1 The command of audio device
 * @{
 */
#define AUD_SET_CAP_VOL				(1 << AUD_CTRL_CMD_BIT)
#define AUD_GET_CAP_VOL				(2 << AUD_CTRL_CMD_BIT)
#define AUD_GET_CAP_VOL_INFO			(3 << AUD_CTRL_CMD_BIT)
#define AUD_CAP_MUTE				(4 << AUD_CTRL_CMD_BIT)

#define AUD_SET_PLY_VOL				(10 << AUD_CTRL_CMD_BIT)
#define AUD_GET_PLY_VOL				(11 << AUD_CTRL_CMD_BIT)
#define AUD_GET_PLY_VOL_INFO			(12 << AUD_CTRL_CMD_BIT)
#define AUD_PLY_MUTE				(13 << AUD_CTRL_CMD_BIT)

#define AUD_SET_SIG_VOL				(1 << AUD_SUB_MOD_CMD_BIT)
#define AUD_GET_SIG_VOL				(2 << AUD_SUB_MOD_CMD_BIT)
#define AUD_GET_SIG_VOL_INFO			(3 << AUD_SUB_MOD_CMD_BIT)
#define AUD_SIG_MUTE				(4 << AUD_SUB_MOD_CMD_BIT)

#define AUD_SET_R2R_VOL				(10 << AUD_SUB_MOD_CMD_BIT)
#define AUD_GET_R2R_VOL				(11 << AUD_SUB_MOD_CMD_BIT)
#define AUD_GET_R2R_VOL_INFO			(12 << AUD_SUB_MOD_CMD_BIT)

#define AUD_SET_BUF_SIZE			(1 << AUD_BUF_MGR_CMD_BIT)
#define AUD_GET_BUF_SIZE			(2 << AUD_BUF_MGR_CMD_BIT)
#define AUD_SET_PERIOD_SIZE			(3 << AUD_BUF_MGR_CMD_BIT)
#define AUD_GET_PERIOD_SIZE			(4 << AUD_BUF_MGR_CMD_BIT)
/** @} */

/**
* @brief interface function - set the control command for audio device
* @param audio_dri structure for audio driver
* @param cmd the control command
* @param arg the parameter of control command
* @return return 0 if success
*/
int32_t audio_dri_high_ctrl(struct audio_dri_st *audio_dri, uint32_t cmd, uint32_t arg);

#endif
