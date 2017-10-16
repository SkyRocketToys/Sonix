#ifndef __MEDIA_H__
#define __MEDIA_H__

#include <libsnx_cloud/snx_cloud.h>

#define	TRACK_TIMEOUT			(20)
#define USE_ENCRYPT 			(1)
#define USE_P2P_NAT				(1)
#define HEARTBEAT_INTERVAL		(20)
#define RELAY_TO				(8)
#define MAXIMA_USER				(4)
#define LOCAL_MIN_PORT			(6000)
#define LOCAL_MAX_PORT			(6999)
#define MAXIMA_BIND_CHECK		(20)
#define	SNX_PROFILE_NAME		"cloud"
#define	VIDEO_TRACK_NAME		"video"
#define AUDIO_TRACK_NAME		"audio"
#define TALK_TRACK_NAME 		"talk"
#define	PLAYBACK_PROFILE_NAME 	"playback"

/* Hardcode information */
#define SERVER_URL 				"http://121.199.65.105:80/v1"
#define SERVER_URL_1			"http://app.dev.sonixcloud.com/v2/c2m/v1"
#define PROXY_URL 				"http://121.199.65.105:80/v1"
#define PROXY_URL_1				"http://app.dev.sonixcloud.com/v2/c2m/v1"
#define QID 					"HPS0NW0002100000000000000000000071012000021"
#define MODEL					"C2M"
#define BRAND					"SONiX"
#define PRODUCT_KEY 			"71012000021:e9a0cddb65208c9c4196aea2089fe17e6a10be1e3f0ed4ac6bcec9aad5e343c57803200eb3d4bcd6464f0d004e533e9d70d056b14c61fc06dd860fa1c375c793:72ca80fb40a80a8ea8276a6cd10205a0c06ab38f56fa6982c281db52e63f0147b003732d814a4335e2120bb3f5ab6031b2f642369c3242984903d98f06eefcf9:010001"
#define FW_VERSION				"STIPCAM_0_1.4.3_OV9712"
#define QID						NULL

typedef struct snx_video_str
{
	int undefined;

}snx_video_str_t;

typedef struct snx_audio_str
{
	int undefined;

}snx_audio_str_t;

int snx_AVMedia_Init(struct snx_streamer *streamer, snx_video_str_t *pVdieo, snx_audio_str_t *pAudio);

int snx_startup_Test(snx_streamer_t *streamer);
#endif // __MEDIA_H__
