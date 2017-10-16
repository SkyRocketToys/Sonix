/*********************************************************************************
* fm.h
*
* Header file of error code
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
#ifndef __FM_ERROR_H__
#define __FM_ERROR_H__

#define FM_OK  	        (0)
#define FM_NOOPS  	    (-3001)
#define FM_FILETYPE     (-3002)
#define FM_FILEFORMAT   (-3003)
#define FM_FILEMEM      (-3004)
#define FM_CONFIG       (-3005)
#define FM_OPENFILE     (-3006)
#define FM_LISTONGOING  (-3007)
#define FM_LISTFINISH   (-3008)
#define FM_LISTWAIT     (-3009)
#define FM_NOTHISNODE   (-3010)
#define FM_NODEFAULTPTR (-3011)
#define FM_PTREXIST     (-3012)
#define FM_FILETYPELIST (-3013)
#define FM_TYPENOTEXIST (-3014)
#define FM_ALLOCATEFAIL (-3015)
#define FM_CREATEMUTEX  (-3016)
#define FM_OPENDIR      (-3017)
#define FM_LASTNODE     (-3018)
#define FM_FIRSTNODE    (-3019)
#define FM_CURINDEX     (-3020)
#define FM_THUBNAILINDEX  (-3021)
#define FM_NOTHUMBNAL	(-3022)


#define FM_ERRNO_MAP(XX)                                                     	\
	XX(NOOPS,           "ops does not exist ")                                  \
	XX(FILETYPE,        "file type is unknown")                                 \
	XX(FILEFORMAT,      "file format is unknown")                               \
	XX(FILEMEM,         "length of file  is not enough")                        \
	XX(CONFIG,          "config of system does not exist")                      \
	XX(OPENFILE,        "open file is error")								   	\
	XX(LISTONGOING,     "getting list of file is on going")                     \
	XX(LISTFINISH,      "list of file already exist")                           \
	XX(LISTWAIT,        "list of file does not exist")                          \
	XX(NOTHISNODE,      "does not find the node") 							  	\
	XX(NODEFAULTPTR,    "no default user ptr") 							      	\
	XX(PTREXIST,        "default ptr already exist") 				           	\
	XX(FILETYPELIST,    "fmmanager list of filetype already exist")             \
	XX(TYPENOTEXIST,    "does not find this filetype in fmmanager list")	   	\
	XX(ALLOCATEFAIL,    "locate memory fail")					               	\
	XX(CREATEMUTEX,	  	"create semaphore fail")	                           	\
	XX(OPENDIR,	      	"open dir fail")	                                   	\
	XX(LASTNODE,        "the node is lastnode")								 	\
	XX(FIRSTNODE,       "the node is firstnode")								\
	XX(CURINDEX,		"no current file index")								\
	XX(THUBNAILINDEX,   "thumbnail index is too large")							\
	XX(NOTHUMBNAL,   	"no support thumbnail")

#endif
