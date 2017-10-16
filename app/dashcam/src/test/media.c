#include <stddef.h>

#include <FreeRTOS.h>
#include <task.h>
#include <timers.h>

#include <bsp.h>
#include <nonstdlib.h>
#ifdef CONFIG_SYSTEM_TRACE_SELECT
#include <trcUser.h>
#endif
#include "media.h"

/*   */
static int snx_h264_describe(struct snx_streamer *streamer, const struct snx_media_source *source, snx_media_info_t *info, void *cbarg);
static int snx_h264_open(struct snx_streamer *streamer, const struct snx_media_source *source, void *cbarg);
static int snx_h264_close(struct snx_streamer *streamer, const struct snx_media_source *source, void *cbarg);
static int snx_h264_start(struct snx_streamer *streamer, const struct snx_media_source *source, void *cbarg);
static int snx_h264_stop(struct snx_streamer *streamer, const struct snx_media_source *source, void *cbarg);
static int snx_h264_refresh(struct snx_streamer *streamer, const struct snx_media_source *source, void *cbarg);
//static int snx_h264_option(const struct snx_streamer *source, struct cmedia_option_list *list, void *cbarg);
//static enum snx_cfg_support_type snx_h264_cfg_support(const struct snx_media_source *source, struct cmedia_media_cfg *cfg, void *cbarg);

static int snx_audio_describe(struct snx_streamer *streamer, const struct snx_media_source *source, snx_media_info_t *info, void *cbarg);
static int snx_audio_open(struct snx_streamer *streamer, const struct snx_media_source *source, void *cbarg);
static int snx_audio_close(struct snx_streamer *streamer, const struct snx_media_source *source, void *cbarg);
static int snx_audio_start(struct snx_streamer *streamer, const struct snx_media_source *source, void *cbarg);
static int snx_audio_stop(struct snx_streamer *streamer, const struct snx_media_source *source, void *cbarg);
//static int snx_audio_option(const struct snx_streamer *source, struct cmedia_option_list *list, void *cbarg);
//static enum snx_cfg_support_type snx_audio_cfg_support(const struct snx_media_source *source, struct cmedia_media_cfg *cfg, void *cbarg);

static struct snx_media_source *pSourceH264 = NULL;
static struct snx_media_source *pSourceAudioIn = NULL;
static struct snx_media_source *pSourceAudioOut = NULL;

static int snx_h264_describe(
		struct snx_streamer *streamer,
		const struct snx_media_source *source,
		snx_media_info_t *info,
		void *cbarg)
{
	int rc = 0;
	return rc;
}

static int snx_h264_open(
		struct snx_streamer *streamer,
		const struct snx_media_source *source,
		void *cbarg)
{
	int rc = 0;
	return rc;
}

static int snx_h264_close(
		struct snx_streamer *streamer,
		const struct snx_media_source *source,
		void *cbarg)
{
	int rc = 0;
	return rc;
}

static int snx_h264_start(
		struct snx_streamer *streamer,
		const struct snx_media_source *source,
		void *cbarg)
{
	int rc = 0;
	return rc;
}

static int snx_h264_stop(
		struct snx_streamer *streamer,
		const struct snx_media_source *source,
		void *cbarg)
{
	int rc = 0;
	return rc;
}
static int snx_h264_refresh(
		struct snx_streamer *streamer,
		const struct snx_media_source *source,
		void *cbarg)
{
	int rc = 0;
	return rc;
}

/*
static int snx_h264_option(
		const struct snx_streamer *source,
		struct cmedia_option_list *list,
		void *cbarg)
{
	int rc = 0;
	return 0;
}

static enum snx_cfg_support_type snx_h264_cfg_support(
		const struct snx_media_source *source,
		struct cmedia_media_cfg *cfg, void *cbarg)
{
	return CM_CFG_SUPPORT;
}

*/
static int snx_audio_describe(struct snx_streamer *streamer,
		const struct snx_media_source *source,
		snx_media_info_t *info, void *cbarg)
{
	int rc = 0;
	return rc;
}

static int snx_audio_open(struct snx_streamer *streamer,
		const struct snx_media_source *source,
		void *cbarg)
{
	int rc = 0;
	return rc;
}

static int snx_audio_close(struct snx_streamer *streamer,
		const struct snx_media_source *source, void *cbarg)
{
	int rc = 0;
	return rc;
}

static int snx_audio_start(struct snx_streamer *streamer,
		const struct snx_media_source *source, void *cbarg)
{
	int rc = 0;
	return rc;
}

static int snx_audio_stop(struct snx_streamer *streamer,
		const struct snx_media_source *source, void *cbarg)
{
	int rc = 0;
	return rc;
}

/*
static int snx_audio_option(const struct snx_streamer *source,
		struct cmedia_option_list *list, void *cbarg)
{
	int rc = 0;
	return rc;
}

static enum snx_cfg_support_type snx_audio_cfg_support(
		const struct snx_streamer *source,
		struct cmedia_media_cfg *cfg, void *cbarg)
{
	return CM_CFG_SUPPORT;
}
*/

/*
if ((rc = snx_add_profile(streamer, H264_PROFILE_NAME,
		CM_EDIT_YES))) {
	fprintf(stderr, "failed to add profile: %s\n", strerror(rc));
	goto finally;
}

fprintf(stdout, "adding video track\n");

if ((rc = cmedia_streamer_add_track(streamer, H264_PROFILE_NAME,
		VIDEO_TRACK_NAME, TRACK_TIMEOUT, media_h264))) {
	fprintf(stderr, "failed to add video track: %s\n",
			strerror(rc));
	goto finally;
}
*/

int snx_audio_output_data(void *data, int len, void *cbarg)
{
	return 0;
}

static int procs_apps_requests(const char *name, struct json_object *in, struct json_object *out, void *cbarg)
{

    return 0;
}
int snx_AVMedia_Init(snx_streamer_t *streamer, snx_video_str_t *pVdieo, snx_audio_str_t *pAudio)
{
	int rc = 0;

	if(!(pSourceH264 = snx_add_media(
			streamer,
			SNX_H264,
			snx_h264_describe,
			snx_h264_open,
			snx_h264_close,
			snx_h264_start,
			snx_h264_stop,
			snx_h264_refresh,
			NULL,
			NULL,
			0,
			NULL)))
	{
		print_msg("Prepare SONiX Cloud source - H264 failed!! \n");
	}
	//print_msg("Create pSourceH264 = %p\n", pSourceH264);


	if ((rc = snx_add_profile(streamer, SNX_PROFILE_NAME, CM_EDIT_YES)))
	{
		print_msg("Failed to add profile: %s\n", SNX_PROFILE_NAME);
	}

	if ((rc = snx_add_track(streamer, SNX_PROFILE_NAME, VIDEO_TRACK_NAME, TRACK_TIMEOUT, pSourceH264)))
	{
		print_msg("Failed to add track: %s in profile %s\n", VIDEO_TRACK_NAME, SNX_PROFILE_NAME);
	}

	if(!(pSourceAudioIn = snx_add_media(
			streamer,
			SNX_PCMU,
			snx_audio_describe,
			snx_audio_open,
			snx_audio_close,
			snx_audio_start,
			snx_audio_stop,
			NULL,
			NULL,
			NULL,
			0,
			NULL)))
	{
		print_msg("Prepare SONiX Cloud source - Audio In failed!! \n");
	}

	//print_msg("Create pSourceAudioIn = %p\n", pSourceAudioIn);

	if ((rc = snx_add_track(streamer, SNX_PROFILE_NAME, AUDIO_TRACK_NAME, TRACK_TIMEOUT, pSourceAudioIn)))
	{
		print_msg("Failed to add track: %s in profile %s\n", AUDIO_TRACK_NAME, SNX_PROFILE_NAME);
	}

	if(!(pSourceAudioOut = snx_add_media(
			streamer,
			SNX_PCMU,
			snx_audio_describe,
			snx_audio_open,
			snx_audio_close,
			snx_audio_start,
			snx_audio_stop,
			NULL,
			NULL,
			NULL,
			0,
			NULL)))
	{
		print_msg("Prepare SONiX Cloud source - Audio Output failed!! \n");
	}

	//print_msg("Create pSourceAudioOut = %p\n", pSourceAudioOut);

	if ((rc = snx_add_talk_track(streamer, SNX_PROFILE_NAME, pSourceAudioOut, snx_audio_output_data, pSourceAudioOut)))
	{
		print_msg("failed to add audio talk track\n");
	}

	if (snx_cmd_add(streamer, "getMemoryUsage", procs_apps_requests, NULL))
	{
		print_msg("add command fail\n");
	}

	return 0;
}


int snx_startup_Test(snx_streamer_t *streamer)
{
	print_msg("Wakeup SONiX streamer\n");
	if (snx_streamer_start(streamer))
	{
		print_msg("startup SONiX streamer fail\n");
	}

	return 0;
}
