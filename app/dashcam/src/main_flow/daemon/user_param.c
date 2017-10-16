/*********************************************************************************
* use_cmd_bit.c
*
* Implementation :user sets wifi, preview, record parameter
*
* History:
*    2015/06/24 - [Allen_Chang] created file
*
* Copyright (C) 1996-2015, Sonix, Inc.
*
* All rights reserved. No Part of this file may be reproduced, stored
* in a retrieval system, or transmitted, in any form, or by any means,
* electronic, mechanical, photocopying, recording, or otherwise,
* without the prior consent of Sonix, Inc.

*********************************************************************************/



#include <nonstdlib.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <generated/snx_sdk_conf.h>
#include "user_param.h"

static wifiparam wifi_set_param = {{0}};
static previewparam preview_set_param = {{0}};
static recordparam  record_set_param = {{0}};


//wifi
void set_ssid_bit(bool value)
{
	wifi_set_param.bits.ssid = value;
	return;
}

void set_channel_bit(bool value)
{
	wifi_set_param.bits.channel = value;
	return;
}

void set_pwd_bit(bool value)
{
	wifi_set_param.bits.pwd = value;
	return;
}

void set_mode_bit(bool value)
{
	wifi_set_param.bits.mode = value;
	return;
}

int chk_ssid_bit(void)
{
	if (wifi_set_param.wifi_param & wifissidflag)
		return 1;

	return 0;
}

int chk_channel_bit(void)
{
	if (wifi_set_param.wifi_param & wifichannelflag)
		return 1;

	return 0;
}

int chk_pwd_bit(void)
{
	if (wifi_set_param.wifi_param & wifipwdflag)
		return 1;

	return 0;
}

int chk_mode_bit(void)
{
	if (wifi_set_param.wifi_param & wifimodeflag)
		return 1;

	return 0;
}


int check_wifi_param_bit(void)
{
	if (wifi_set_param.wifi_param != 0)
		return 1;
	return 0;
}

//preview
void set_preview_fps_bit(bool value)
{
	preview_set_param.bits.fps = value;
	return;
}

void set_preview_bps_bit(bool value)
{
	preview_set_param.bits.bps = value;
	return;
}

void set_preview_reso_bit(bool value)
{
	preview_set_param.bits.resolution = value;
	return;
}

int chk_preview_fps_bit(void)
{
	if (preview_set_param.preview_param & previewfpsflag)
		return 1;

	return 0;
}

int chk_preview_bps_bit(void)
{
	if (preview_set_param.preview_param & previewbpsflag)
		return 1;

	return 0;
}

int chk_preview_reso_bit(void)
{
	if (preview_set_param.preview_param & previewresolutionflag)
		return 1;

	return 0;
}

int chk_preview_param_bit(void)
{
	if (preview_set_param.preview_param != 0)
		return 1;
	return 0;
}

///////////record//////////////////

void set_record_cycle_bit(bool value)
{
	record_set_param.bits.cycle = value;
	return;
}

void set_record_audio_voice_bit(bool value)
{
#ifndef CONFIG_APP_DRONE
	record_set_param.bits.audiovoice = value;
#endif
	return;
}

int chk_record_cycle_bit(void)
{
	if (record_set_param.record_param & recordcycle)
		return 1;

	return 0;
}

int chk_record_audio_voice_bit(void)
{
#ifndef CONFIG_APP_DRONE
	if (record_set_param.record_param & recordaudiovoice)
		return 1;
#endif
	return 0;
}
