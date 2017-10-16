/*********************************************************************************
* use_cmd_bit.h
*
* Header file of  use_cmd_bit
*
* History:
*    2016/04/21 - [Allen_Chang] created file
*
* Copyright (C) 1996-2015, Sonix, Inc.
*
* All rights reserved. No Part of this file may be reproduced, stored
* in a retrieval system, or transmitted, in any form, or by any means,
* electronic, mechanical, photocopying, recording, or otherwise,
* without the prior consent of Sonix, Inc.

*********************************************************************************/

#ifndef __USER_CMD_BIT_H__
#define __USER_CMD_BIT_H__

#include <stdbool.h>


#define wifissidflag                (0x1<<0)
#define wifipwdflag                 (0x1<<1)
#define wifichannelflag             (0x1<<2)
#if 1
#define wifimodeflag            	(0x1<<3)
#else
#define wifireserve1flag            (0x1<<3)
#endif
#define wifireserve2flag            (0x1<<4)
#define wifireserve3flag            (0x1<<5)
#define wifireserve4flag            (0x1<<6)
#define wifireserve5flag            (0x1<<7)


typedef union {
	struct {
		unsigned char ssid : 1;
		unsigned char pwd : 1;
		unsigned char channel : 1;
#if 1
		unsigned char mode : 1;		/* Wi-Fi authozization cfg change */
#else
		unsigned char bit3 : 1;
#endif
		unsigned char bit4 : 1;
		unsigned char bit5 : 1;
		unsigned char bit6 : 1;
		unsigned char bit7 : 1;
	} bits;
	unsigned char wifi_param;
} wifiparam ;



#define previewfpsflag                  (0x1<<0)
#define previewbpsflag                  (0x1<<1)
#define previewresolutionflag   		(0x1<<2)
#define previewreserve1flag             (0x1<<3)
#define previewreserve2flag             (0x1<<4)
#define previewreserve3flag             (0x1<<5)
#define previewreserve4flag             (0x1<<6)
#define previewreserve5flag             (0x1<<7)

typedef union {
	struct {
		unsigned char fps : 1;
		unsigned char bps : 1;
		unsigned char resolution : 1;
		unsigned char bit3 : 1;
		unsigned char bit4 : 1;
		unsigned char bit5 : 1;
		unsigned char bit6 : 1;
		unsigned char bit7 : 1;
	} bits;
	unsigned char preview_param;
} previewparam ;



#define recordcycle    	           (0x1<<0)
#define recordaudiovoice           (0x1<<1)
#define sdcardformatflag	       (0x1<<2)
#define recordreserve3flag	       (0x1<<3)
#define recordreserve4flag		   (0x1<<4)
#define recordreserve5flag	       (0x1<<5)
#define recordreserve6flag         (0x1<<6)
#define recordreserve7flag		   (0x1<<7)

typedef union {
	struct {
		unsigned char cycle : 1;
		unsigned char audiovoice : 1;
		unsigned char sdcardformat : 1;
		unsigned char bit3 : 1;
		unsigned char bit4 : 1;
		unsigned char bit5 : 1;
		unsigned char bit6 : 1;
		unsigned char bit7 : 1;
	} bits;
	unsigned char record_param;
} recordparam ;


//wifi
void set_ssid_bit(bool value);
void set_channel_bit(bool value);
void set_pwd_bit(bool value);
void set_mode_bit(bool value);
int  chk_ssid_bit(void);
int chk_channel_bit(void);
int chk_pwd_bit(void);
int chk_mode_bit(void);
int check_wifi_param_bit(void);
//preview
void set_preview_fps_bit(bool value);
void set_preview_bps_bit(bool value);
void set_preview_reso_bit(bool value);
int chk_preview_fps_bit(void);
int chk_preview_bps_bit(void);
int chk_preview_reso_bit(void);
int chk_preview_param_bit(void);
//record
void set_record_cycle_bit(bool value);
void set_record_audio_voice_bit(bool value);
int chk_record_cycle_bit(void);
int chk_record_audio_voice_bit(void);

#endif

