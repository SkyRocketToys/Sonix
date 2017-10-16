#ifndef __RTSP_SERVER__H__
#define __RTSP_SERVER__H__

#include <sys/time.h>
#include <stdbool.h>
#define PLBACK_STREAM   0
#define PREVIEW_STREAM  1

extern int get_media_id_in_content(char *name);

/** \defgroup mid_rtsp rtsp middleware modules
 * \n
 * @{
 */
/** @} */

/** \defgroup func_rtsp rtsp function
 *  \ingroup mid_rtsp
 * @{
 */

/**
* @brief structure for audio attribute
*/
typedef struct audio_attr {
	char audio_codec[32];	/**<  Audio codec string: mulaw alaw aac */
	int samplerate;			/**<  Audio samplerate */
	int bitrate;				/**<  Audio bitrate */
	int channels;				/**<  Audio channels */
} audio_attr;
/**
* @brief structure for video attribute
*/
typedef struct video_attr {
	char video_codec[32];	/**<  Video codec string: h264 mjpeg */
	int width;				/**<  Video pic width */
	int height;				/**<  Video pic height */
	int fps;					/**<  Video frame rate */
} video_attr;

/**
 * The callback function start to play the record file.
 * @param url: Record file name.  ex: "2020_10_05_14_13_59_2.avi".
 * @param time: Record file start time.
 */
typedef void (*play_back_start_cb)(const char *url, int time);


/**
 * The callback function start to force iframe .
 */
typedef void (*rtsp_force_iframe_cb)(void);
//typedef void (*stream_start_cb_t)(int live_view, char *filename, int file_type, char *mac, uint32_t clientip);
typedef int (*stream_start_cb_t)(int live_view, char *filename, int file_type, char *mac, unsigned long clientip);
typedef void (*stream_finish_cb_t)(int live_view, char *mac);

/**
 *Register a callback function to start playing record file.
 *@param cb: Pointer to the function to start palying record file.
 */
void rtsp_reg_playback_start(play_back_start_cb cb);

/**
 * Register a callback function to force iframe.
 */
void rtsp_reg_force_iframe(rtsp_force_iframe_cb cb);
void rtsp_reg_stream_start(stream_start_cb_t cb);
void rtsp_reg_stream_finish(stream_finish_cb_t cb);
/**
 * Provide the functions to Create, Take, Give, Destroy Semaphore
   for waiting playback_init_task finish.
 *@param filerename: Record file name. ex: "2020_10_05_14_13_59_2.avi".
 */
int rtsp_play_init(char *filerename);
void wait_rtsp_stream_create(char *filerename);
void rtsp_stream_create_finish(char *filerename);
void rtsp_play_destroy(const char *filerename);

/**
* @brief interface function - init rtsp server.
* @return return 0 if success.
*/
int init_rtsp_server(bool audio_enable);
/**
* @brief interface function - destroy rtsp server.
* @return return 0 if success.
*/
int destroy_rtsp_server();
/**
* @brief interface function - send data to rtsp server.
* @param id data stream id.
* @param data data  pointer.
* @param len data  length.
* @param pb_tv the fake time for AV sync in playback mode.
* @param live_view RTP_PREVIEW_MODE or RTP_PLBACK_MODE.
* @return return 0 if success.
*/
int send_rtp_data(int id, char *data, int len, struct timeval *pb_tv, int live_view);
/**
* @brief interface function - create rtsp streaming.
* @param filename rtsp stream name.
* @param video structure  pointer of video attribute.
* @param audio structure  pointer of audio attribute.
* @return return video and audio data id (video_id | audio_id << 16).
*/
int add_rtsp_stream(char *filename, int file_len, video_attr *video, audio_attr *audio);
/**
* @brief interface function - destroy rtsp streaming from server.
* @param filename rtsp stream name.
*/
void del_rtsp_stream(char *filename);
/**
* @brief interface function - set max connection number of rtsp server.
* @param conn_num max connection number.
*/
void set_max_connections(unsigned int conn_num);
/**
* @brief interface function - stop all of the rtsp connection.
*/
void stop_all_rtsp_stream();
/**
* @brief interface function - stop  the rtsp connection by ip address.
* @param ipaddr: ip address.
*/
void close_rtsp_stream_by_ip(char *ipaddr);
/**
* @brief interface function - stop  the rtsp connection by ip address and port.
* @param ipaddr: ip address.
* @param port: ip port.
*/
void close_rtsp_stream_by_ip_and_port(char *ipaddr, int port);
void close_rtsp_stream_by_mac(char *mac);


/** @} */
#endif

