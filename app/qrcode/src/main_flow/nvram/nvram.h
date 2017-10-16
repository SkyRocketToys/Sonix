/*********************************************************************************
* nvram.h
*
* Header file of  nvram param 
*
* History:
*    2016/06/28 - [Allen_Chang] created file
*
* Copyright (C) 1996-2015, Sonix, Inc.
*
* All rights reserved. No Part of this file may be reproduced, stored
* in a retrieval system, or transmitted, in any form, or by any means,
* electronic, mechanical, photocopying, recording, or otherwise,
* without the prior consent of Sonix, Inc.

*********************************************************************************/

#ifndef __M_NVRAM_H__
#define __M_NVRAM_H__



#ifdef __cplusplus
extern "C" {
#endif


typedef enum{
	DT_STRING = 0,	    //!< String (char)
	DT_BIN_RAW,		    //!< Binary (file)
	DT_INT,			    //!< integer (4 Bytes)
	DT_FLOAT,			//!< float (4 Bytes)
	DT_UINT,			//!< unsigned integer (4 Bytes)
	DT_UCHAR,			//!< unsigned char
} nvram_data_type_t;


typedef struct nvram_info_s
{    
	 char* nvram_pkg_name;
	 char* nvram_ckg_name;
	 nvram_data_type_t  data_type;
	 void* data;
	 struct nvram_info_s *ptr_to_next;    
	 struct nvram_info_s *ptr_to_previous;
}nvram_info_t;


typedef struct nvram_handle_s     
{    
   nvram_info_t *nvram_list;
   xSemaphoreHandle  nvram_mutex;
}nvram_handle_t;



void nvram_list_init();
void nvram_list_uinit();
int snx_nvram_integer_set_to_ddr(char *pkg_name, char *cfg_name, int data);
int snx_nvram_integer_get_from_ddr(char *pkg_name, char *cfg_name, int *data);
int snx_nvram_unsign_integer_set_to_ddr(char *pkg_name, char *cfg_name, unsigned int data);
int snx_nvram_unsign_integer_get_from_ddr(char *pkg_name, char *cfg_name, unsigned int *data);
int snx_nvram_string_set_to_ddr(char *pkg_name, char *cfg_name,char *data);
int snx_nvram_string_get_from_ddr(char *pkg_name, char *cfg_name, char *data);

#ifdef __cplusplus
}
#endif

#endif

