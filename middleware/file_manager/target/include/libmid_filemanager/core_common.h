/*********************************************************************************
* fm_core_common.h
*
* Header file of fm_core_common.c
*
* History:
*    2016/08/02- [Allen_Chang] created file
*
* Copyright (C) 1996-2015, Sonix, Inc.
*
* All rights reserved. No Part of this file may be reproduced, stored
* in a retrieval system, or transmitted, in any form, or by any means,
* electronic, mechanical, photocopying, recording, or otherwise,
* without the prior consent of Sonix, Inc.

*********************************************************************************/

#ifndef __FM_CORE_COMMON_H__
#define __FM_CORE_COMMON_H__


#define mediatype_len 8
#define root_len 64
#define prefix_len 16
#define countindx_len 16
#define folder_len 64
#define file_path_len 128
#define list_node_data_len 128

enum FILE_FORMAT {
	FORMAT_TIME = 1,
	FORMAT_COUNT = 2,
	UNKNOWN = 0x80,
};

enum FILE_TYPE {
	T_AUDIO = 1,
	T_RECORD = 2,
	T_TIMELAPSE = 3,
	T_PROTECT = 4,
	T_SNAPSHOT = 5,
	T_THUMBNAIL = 6,
	T_UNKNOWN = 0x80,
};

enum LIST_STATE {
	LIST_WAIT = 0,
	LIST_ON_GOING = 1,
	LIST_FINISH = 2,
	LIST_FAIL = 3,
};


#endif
