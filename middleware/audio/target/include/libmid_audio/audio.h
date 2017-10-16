/**
 * @file
 * this is audio middleware interface file, include this file before use
 * audio.h
 * @author yanjie_yang
 */
#ifndef _AUDIO_H_
#define _AUDIO_H_

#include <audio/audio_dri.h>
#include <audio/audio_error.h>

/** \defgroup mid_audio Audio middleware modules
 * \n
 * @{
 */
/** @} */

#define BIT(x)				(1UL << (x))

/** \defgroup func_audio Audio function
 *  \ingroup mid_audio
 * @{
 */

/**
 * \defgroup AUDIO_FORMAT The format of audio
 * @{
 */
#define AUD_FORMAT_A_LAW                BIT(16)
#define AUD_FORMAT_MU_LAW               BIT(17)
#define AUD_FORMAT_G726                 BIT(18)
#define AUD_FORMAT_AUD32                BIT(19)
#define AUD_FORMAT_AAC			BIT(20)
#define AUD_FORMAT_AMRNB                BIT(21)
#define AUD_FORMAT_AMRWB                BIT(22)
/** @} */


struct audio_stream_st;
struct params_st;

/**
* @brief interface function - audio middleware module init function
* @return return 0 if success
*/
int32_t audio_init(void);

/**
* @brief interface function - open audio stream
* @param stream structure for audio stream
* @param type the type of audio stream
* @return return 0 if success
*/
int32_t audio_open(struct audio_stream_st **stream, int32_t type);

/**
* @brief interface function - close audio stream
* @param stream structure for audio stream
* @return return 0 if success
*/
int32_t audio_close(struct audio_stream_st *stream);

/**
* @brief interface function - stop capture/playback audio stream
* @param stream structure for audio stream
* @return return 0 if success
* @warning for playback wait for all pending frames to be played and then stop the audio device. For capture stop the audio device immediately.
*/
int32_t audio_drain(struct audio_stream_st *stream);

/**
* @brief interface function - stop capture/playback audio stream
* @param stream structure for audio stream
* @return return 0 if success
* @warning this function stops the audio device immediately. The pending samples on the buffer are ignored.
*/
int32_t audio_drop(struct audio_stream_st *stream);

/**
* @brief interface function - resume capture/playback audio stream frome stop/overflow/underrun status.
* @param stream structure for audio stream
* @return return 0 if success
*/
int32_t audio_resume(struct audio_stream_st *stream);

/**
* @brief interface function - get the status of audio stream
* @param stream structure for audio stream
* @param status pointer to the status of audio stream
* @return return 0 if success
*/
int32_t audio_status(struct audio_stream_st *stream, int32_t *status);

/**
* @brief interface function - read audio data from audio device (capture)
* @param stream structure for audio stream
* @param buf pointer to the buffer for storing audio data
* @param size the size of the buffer
* @param is_block indicates whether this function is blocked at a call
* @return return 0 if success
*/
int32_t audio_read(struct audio_stream_st *stream, void *buf, uint32_t size, int32_t is_block);

/**
* @brief interface function - write audio data to audio device (playback)
* @param stream structure for audio stream
* @param buf pointer to the buffer for storing audio data
* @param size the size of the buffer
* @param is_block indicates whether this function is blocked at a call
* @return return 0 if success
*/
int32_t audio_write(struct audio_stream_st *stream, const void *buf, uint32_t size, int32_t is_block);

/**
* @brief interface function - allocate the memory of audio params structure
* @param params structure for audio params
* @return return 0 if success
*/
int32_t audio_params_alloca(struct params_st **params);

/**
* @brief interface function - free the memory of audio params structure
* @param params structure for audio params
* @return return 0 if success
*/
int32_t audio_params_free(struct params_st *params);

/**
* @brief interface function - set the audio params to audio device
* @param stream structure for audio stream
* @param params structure for audio params
* @return return 0 if success
*/
int32_t audio_set_params(struct audio_stream_st *stream, struct params_st *params);

/**
* @brief interface function - get the audio params of audio device
* @param stream structure for audio stream
* @param params structure for audio params
* @return return 0 if success
*/
int32_t audio_get_params(struct audio_stream_st *stream, struct params_st *params);

/**
* @brief interface function - set the sample rate of audio device
* @param stream structure for audio stream
* @param params structure for audio params
* @param rate sample rate
* @return return 0 if success
*/
int32_t audio_params_set_rate(struct audio_stream_st *stream, struct params_st *params, uint32_t rate);

/**
* @brief interface function - set the sample format of audio device
* @param stream structure for audio stream
* @param params structure for audio params
* @param format sample format
* @return return 0 if success
*/
int32_t audio_params_set_format(struct audio_stream_st *stream, struct params_st *params, uint32_t format);

/**
* @brief interface function - set the min avail size for audio_read/audio_write.
* @param stream structure for audio stream
* @param params structure for audio params
* @param size the min avail size
* @return return 0 if success
*/
int32_t audio_params_set_min_avail(struct audio_stream_st *stream, struct params_st *params, uint32_t size);

/**
* @brief interface function - set the align size for audio_read/audio write.
* @param stream structure for audio stream
* @param params structure for audio params
* @param size the align size
* @return return 0 if success
*/
int32_t audio_params_set_align_size(struct audio_stream_st *stream, struct params_st *params, uint32_t size);

/**
* @brief interface function - get the sample rate of audio device
* @param params structure for audio params
* @param rate sample rate
* @return return 0 if success
*/
int32_t audio_params_get_rate(struct params_st *params, uint32_t *rate);

/**
* @brief interface function - get the sample format of audio device
* @param params structure for audio params
* @param format sample format
* @return return 0 if success
*/
int32_t audio_params_get_format(struct params_st *params, uint32_t *format);

/**
* @brief interface function - get the min avail size for audio_read/audio_write.
* @param params structure for audio params
* @param size pointer to the min avail size
* @return return 0 if success
*/
int32_t audio_params_get_min_avail(struct params_st *params, uint32_t *size);

/**
* @brief interface function - get the align size for audio_read/audio_write.
* @param params structure for audio params
* @param size pointer to the align size
* @return return 0 if success
*/
int32_t audio_params_get_align_size(struct params_st *params, uint32_t *size);


/**
* @brief interface function - set the control command for audio device
* @param stream structure for audio stream
* @param cmd the control command
* @param arg the parameter of control command
* @return return 0 if success
*/
int32_t audio_high_ctrl(struct audio_stream_st *stream, uint32_t cmd, uint32_t arg);

#endif
/** @} */

