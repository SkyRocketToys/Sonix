/**
 * @file
 * this is Audio Gain Control header file, include this file before use
 * @author (Henry Pai porting to FreeRTOS)
 */
#include <audio/audio_dri.h>

#define AGC_GAIN_MAX				31
#define AGC_GAIN_MIN				1
#define AGC_GAIN_DEFAULT			22
#define AGC_DYN_GAIN_MAX			26
#define AGC_DYN_GAIN_MIN			18
#define AGC_DYN_TARGET_LEVEL_HIGH	3500
#define AGC_DYN_TARGET_LEVEL_LOW	2000
#define AGC_DYN_UPDATE_SPEED		4
#define AGC_PEAKAMP_THD				12000
#define AGC_PEAKCNT_THD				66
#define AGC_UPSTEP					2
#define AGC_DOWNSTEP				1

struct snx_agc_params_st {
	int gain_default;
	int gain_min;
	int gain_max;
	int dyn_gain_min;
	int dyn_gain_max;
	int upstep;
	int downstep;
	int peakamp_thres;
	int peakcnt_thres;
	int frame_bufsize;
	int samplerate;
	int dyn_targetlevel_low;
	int dyn_targetlevel_high;
	int dyn_updatespeed;
};

int SNX_AGC_Process(short *inputbuf);
void SNX_AGC_Process_init(int AGC_gain_max, int AGC_gain_min, int AGC_gain_default, int AGC_Dynamic_gain_max, int AGC_Dynamic_gain_min, int AGC_Dynamic_Target_Level_Low, int AGC_Dynamic_Target_Level_High, int AGC_Dynamic_updateSpeed, int AGC_bufsize, int sample_rate, int peakamp_thres, int peakcnt_thres, int response_up_step, int response_down_step);


