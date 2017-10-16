/**
 * @file
 * this is application header file for audio push talk, include this file before use
 * @author Algorithm Dept Sonix.
 */

#ifndef __PT_AUDIO_H__
#define __PT_AUDIO_H__


#include <libmid_audio/audio.h>
#include <libmid_audio/audio_codec.h>

#define AAC_ENABLE 1

#if AAC_ENABLE
#include <libmid_audio/snx_aac.h>
#else
#include <libmid_audio/snx_alaw.h>
#endif

#define PT_PORT				8828
#define PT_MAX_CONNECT_NUM	1
#define PT_RCV_BUF_SIZE		1024
#define PT_CLIENT_TIMEOUT	10000

#define PT_AAC_SAMPLE_BYTE			2
#define PT_AAC_BLOCK				0
#define PT_AAC_SMAPLE_RATE 			11025
#define PT_AAC_BITRATE				15999
#define PT_AAC_BUF_SIZE				8192
#define PT_AAC_BUF_THR_WAIT			0
#define PT_AAC_BUF_THR_PLAY			0
#define PT_AAC_BUF_THR_FULL			8000
#define PT_AAC_BUF_THR_FULL_WAIT	6000

enum PT_AAC_BUF_THR_STAT {
	PT_AAC_BUF_STAT_WAIT = 0,
	PT_AAC_BUF_STAT_PLAY,
	PT_AAC_BUF_STAT_FULL
};

enum PT_PLAY_MODE {
	PT_PLAY_MODE_NORMAL = 0,
	PT_PLAY_MODE_PLAYALL
};

int pt_audio_init(int port);
void pt_audio_uninit(void);

#endif
