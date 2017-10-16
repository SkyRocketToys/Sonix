
/*********************************************************************************
* rec_fatfs.h
*
* Header file of  reading memory card space
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

#ifndef __SD_CARDFATFS_H__
#define __SD_CARDFATFS_H__

#include <libmid_fatfs/ff.h>
#include <stdbool.h>

int card_info_space(long long *total_space, long long *free_space);
long long card_total_space(void);
long long card_free_space(void);
int chk_card_allfolder(void);
long long get_curfolder_usedspace(int record_type);
int chk_root_folder(void);

#endif
