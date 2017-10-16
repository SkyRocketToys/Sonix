
/*********************************************************************************
* rec_seed.h
*
* Header file of  record seek
*
* History:
*    2016/06/06 - [Allen_Chang] created file
*
* Copyright (C) 1996-2015, Sonix, Inc.
*
* All rights reserved. No Part of this file may be reproduced, stored
* in a retrieval system, or transmitted, in any form, or by any means,
* electronic, mechanical, photocopying, recording, or otherwise,
* without the prior consent of Sonix, Inc.

*********************************************************************************/



#ifndef __REC_SEED_H__
#define __REC_SEED_H__


int  rec_seed(void);
int set_seed_to_file(int seed);
int get_seed_from_file(int *seed);
void start_new_seed(void);
void update_seed(void);
void  seed_inttostring(char *firtstring, char *secondstring);
int check_update_seed(system_date_t *dev_time);


#endif
