/*********************************************************************************
* fm_user_control.h
*
* Header file of fm_user_control
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


#ifndef __FM_USERCONTROL_H__
#define __FM_USERCONTROL_H__
#include <libmid_fatfs/ff.h>
#include "core_common.h"

enum FILE_TYPE;

struct fm_node;


struct usr_config {      //see config.c setting
	char root_path[root_len];
	char rec_path[folder_len];
	char timelapse_path[folder_len];
	char protect_path[folder_len];
	char pic_path[folder_len];
	char thumbnail_path[folder_len];
	char rec_prefix[prefix_len];
	char timelapse_prefix[prefix_len];
	char protect_prefix[prefix_len];
	char pic_prefix[prefix_len];
	char thumbnail_prefix[prefix_len];
	char rec_tn_path[folder_len];
	char protect_tn_path[folder_len];
	char ts_tn_path[folder_len];
	char media_type[mediatype_len];      //avi,mp4
	char count_index[countindx_len];
	int root_folder_len;
	int rec_folder_len;
	int protect_folder_len;
	int pic_folder_len;
	int timelapse_folder_len;
	int thumbnail_folder_len;
	int set_config_ready;
	int skip;
};

int snx_fm_init(enum FILE_FORMAT fileformat, struct usr_config *usrconfig);
int snx_fm_add_filetype(enum FILE_TYPE filetype, bool add_thumbnail, void * cb);
int snx_fm_update_count(enum FILE_TYPE filetype, void *arg);
void snx_fm_uinit(void);
int snx_fm_print_filetype(void);
int snx_fm_create_filelist(enum FILE_TYPE filetype, void *arg);
int snx_fm_release_filelist(enum FILE_TYPE filetype, void *arg);
int snx_fm_get_cor_file(enum FILE_TYPE filetype, const char *parent_name, char *ref_fn, int len, int index, void *arg);
int snx_fm_get_cor_num(enum FILE_TYPE filetype, const char *fn, int *cor_num, void *arg);
int snx_fm_get_filelist(enum FILE_TYPE filetype, struct fm_node **file_list, void *arg);
int snx_fm_check_list_is_finish(enum FILE_TYPE filetype);
int snx_fm_node_to_file(enum FILE_TYPE filetype, char *ab_fn, int len, void *arg);
int snx_fm_node_to_buffer_create(enum FILE_TYPE filetype, char ***listbuffer, int *listsize, void *arg);
int snx_fm_node_to_buffer_delete(enum FILE_TYPE filetype, char **listbuffer , int listsize, void *arg);
int snx_fm_del_file_node(enum FILE_TYPE filetype, const char *fn, void *arg);
int snx_fm_del_all_file_node(enum FILE_TYPE filetype, void *arg);
int snx_fm_del_oldest_file_node(enum FILE_TYPE filetype, char *del_fn, void *arg);
int snx_fm_get_file_amount(enum FILE_TYPE filetype, int *amount, void *arg);
int snx_fm_get_file_pos(enum FILE_TYPE filetype, const char *fn, int *pos, void *arg);
int snx_fm_get_file_size(enum FILE_TYPE filetype, const char *fn, long long *size, void *arg);
int snx_fm_get_total_fsize(enum FILE_TYPE filetype, long long *size, void *arg);
int snx_fm_get_file_mode(enum FILE_TYPE filetype, const char *file, unsigned char *mode, void *arg);
int snx_fm_set_file_mode(enum FILE_TYPE filetype, const char *file, unsigned char mode, void *arg);
int snx_fm_create_file(enum FILE_TYPE filetype, char *filename, int len, void *arg);
int snx_fm_create_thumbnail_file(enum FILE_TYPE filetype, const char *fn, char *tn_fn, int len, int index, int interval, void *arg);
int snx_fm_add_filenode(enum FILE_TYPE filetype, FILINFO fno, void *arg); //test for here
int snx_fm_get_cur_file(enum FILE_TYPE filetype, char *curfile, int len, void *arg);
int snx_fm_get_next_file(enum FILE_TYPE filetype, char *nextfile, int len, void *arg);
int snx_fm_get_prev_file(enum FILE_TYPE filetype, char *prevfile, int len, void *arg);
int snx_fm_get_first_file(enum FILE_TYPE filetype, char *firstfile, int len, void *arg);
int snx_fm_get_last_file(enum FILE_TYPE filetype, char *lastfile, int len, void *arg);
int snx_fm_set_cur_file(enum FILE_TYPE filetype, char *setcurfile, void *arg);
int snx_fm_get_cur_folder_used_space(enum FILE_TYPE filetype, long long *size);
int snx_fm_add_thumbnail_node(enum FILE_TYPE filetype,const char* name, void *arg); 
int snx_fm_del_thumbnail_node(enum FILE_TYPE filetype,const char* parent_name, void *arg); 
int snx_fm_check_file_legal(const char * filename);

#endif
