
/*********************************************************************************
* sdcard_sizecontrol.h
*
* Header file of  memory card size control
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
#ifndef __SD_CARDSIZECONTROL_H__
#define __SD_CARDSIZECONTROL_H__


struct card_size_s {
	long long total_sd_size;				/**<memory card total size*/
	long long total_sd_free_size;         	/**<memory card free  size*/

	long long schedfolder_usedsize;         /**<schedule record  folder size have been used*/
#ifndef CONFIG_APP_DRONE
	long long timelapsefolder_usedsize;     /**<timelpase record folder size have been used*/
	long long protectfolder_usedsize;       /**<protected record folder size have been used*/
#endif
	long long picturefolder_usedsize;       /**<picture folder size have been used*/
	long long otherfolder_usedsize;         /**< ohter folder size have been used*/

	long long dashcamfolder_canusesize;
	long long dashcamschedfolder_canusesize;
#ifndef CONFIG_APP_DRONE
	long long dashcamtimelapsefolder_canusesize;
	long long dashcamprotectfolder_canusesize;
#endif
	long long dashcampicturefolder_canusesize;
	int readcard_finish;
	int remind_format;
};

typedef struct cardsize_s card_size_t;


void chk_cardsize(void);
void cardsize_init(void);
void cardsize_uinit(void);

int get_readcard_finish(void);
long long get_schedrec_usedsize(void);
long long get_schedfolder_canusesize(void);
#ifndef CONFIG_APP_DRONE
long long get_protectrec_usedsize(void);
long long get_protectfolder_canusesize(void);
long long get_timelapserec_usedsize(void);
long long get_timelapsefolder_canusesize(void);
#endif
int cardsize_toosmall(void);

#endif
