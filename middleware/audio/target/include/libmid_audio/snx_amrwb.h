#ifndef _SNX_AMRWB_H_
#define _SNX_AMRWB_H_

#include "FreeRTOS.h"

/**
* @brief the bit rate setting for AMR-WB codec
* BR660   =  6.60  k bit / s
* BR885   =  8.85  k bit / s
* BR1265  = 12.65  k bit / s
* BR1425  = 14.25  k bit / s
* BR1585  = 15.85  k bit / s
* BR1825  = 18.25  k bit / s
* BR1985  = 19.85  k bit / s
* BR2305  = 23.05  k bit / s
* BR2385  = 23.85  k bit / s
*/
enum AMRWB_BR {
	AMRWB_BR_660 = 0,
	AMRWB_BR_885,
	AMRWB_BR_1265,
	AMRWB_BR_1425,
	AMRWB_BR_1585,
	AMRWB_BR_1825,
	AMRWB_BR_1985,
	AMRWB_BR_2305,
	AMRWB_BR_2385,
};

/**
* @brief the type setting for AMR-WB codec
* AMRWB_CAP = capture bit stream
* AMRWB_PLY = play bit stream
*/
enum AMRWB_TYPE {
	AMRWB_CAP = 0,
	AMRWB_PLY,
};

/**
* @brief initial values for codecs parameters
*/
struct snx_amrwb_params_st {
	enum AMRWB_TYPE type;
	uint32_t samplerate;
	int16_t dtx;
	enum AMRWB_BR bitrate;
};

struct snx_amrwb_info_st {
	uint32_t samplerate;
	uint32_t bitrate;
};

struct snx_amrwb_st;

/**
* @brief interface function - open
* @param params is structure of snx_amrwb_params_st for user
* @param amrwb is structure of snx_amrwb_st for internal codec
* @warning must return success or fail
*/
int32_t snx_amrwb_open (struct snx_amrwb_params_st *params, struct snx_amrwb_st **amrwb);

/**
* @brief interface function - close
* @param amrwb is structure of snx_amrwb_st for internal codec
* @warning must return success or fail
*/
int32_t snx_amrwb_close(struct snx_amrwb_st *amrwb);

/**
* @brief interface function - encode
* @param amrwb is structure of snx_amrwb_st for internal codec
* @param p_src is the pointer of source stream
* @param p_dst is the pointer of source stream
* @param src_bytes is the size of source stream
* @param P_dst_bytes is the pointer the size encoded actually
* @warning must return success or fail
*/
int32_t snx_amrwb_encode(struct snx_amrwb_st *amrwb, uint8_t *p_src, uint8_t *p_dst, int32_t src_bytes, int32_t *p_dst_bytes);

/**
* @brief interface function - decode
* @param amrwb is structure of snx_amrwb_st for internal codec
* @param p_src is the pointer of source stream
* @param p_dst is the pointer of source stream
* @param src_bytes is the size of source stream
* @param P_dst_bytes is the pointer the size decoded actually
* @warning must return success or fail
*/
int32_t snx_amrwb_decode(struct snx_amrwb_st *amrwb, uint8_t *p_src, uint8_t *p_dst, int32_t src_bytes, int32_t *p_dst_bytes);

/**
* @brief interface function - get information
* @param amrwb is structure of snx_amrwb_st for internal codec
* @param amrwb_info is structure of snx_amrwb_info_st for user
* @warning must return success or fail
*/
int32_t snx_amrwb_get_info (struct snx_amrwb_st *amrnb, struct snx_amrwb_info_st *amrwb_info);

#endif
