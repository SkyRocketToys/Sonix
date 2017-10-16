#ifndef _SNX_AMRNB_H_
#define _SNX_AMRNB_H_

#include "FreeRTOS.h"

/**
* @brief the bit rate setting for AMR-NB codec
* BR475 = 4.75  k bit / s
* BR512 = 5.15  k bit / s
* BR59  = 5.90  k bit / s
* BR67  = 6.70  k bit / s
* BR74  = 7.40  k bit / s
* BR795 = 7.95  k bit / s
* BR102 = 10.20 k bit / s
* BR122 = 12.20 k bit / s
*/
enum AMRNB_BR {
	BR475 = 0,
	BR515,
	BR59,
	BR67,
	BR74,
	BR795,
    BR102,
	BR122,
};

/**
* @brief the type setting for AMR-NB codec
* AMRNB_CAP = capture bit stream
* AMRNB_PLY = play bit stream
*/
enum AMRNB_TYPE {
	AMRNB_CAP = 0,
	AMRNB_PLY,
};

/**
* @brief initial values for codecs parameters
*/
struct snx_amrnb_params_st {
	enum AMRNB_TYPE type;
	uint32_t samplerate;
	int16_t dtx;
	enum AMRNB_BR bitrate;
};

struct snx_amrnb_info_st {
	uint32_t samplerate;
	uint32_t bitrate;
};

struct snx_amrnb_st;

/**
* @brief interface function - open
* @param params is structure of snx_amrnb_params_st for user
* @param amrnb is structure of snx_amrnb_st for internal codec
* @warning must return success or fail
*/
int32_t snx_amrnb_open (struct snx_amrnb_params_st *params, struct snx_amrnb_st **amrnb);

/**
* @brief interface function - close
* @param amrnb is structure of snx_amrnb_st for internal codec
* @warning must return success or fail
*/
int32_t snx_amrnb_close(struct snx_amrnb_st *amrnb);

/**
* @brief interface function - encode
* @param amrnb is structure of snx_amrnb_st for internal codec
* @param p_src is the pointer of source stream
* @param p_dst is the pointer of source stream
* @param src_bytes is the size of source stream
* @param P_dst_bytes is the pointer the size encoded actually
* @warning must return success or fail
*/
int32_t snx_amrnb_encode(struct snx_amrnb_st *amrnb, uint8_t *p_src, uint8_t *p_dst, int32_t src_bytes, int32_t *p_dst_bytes);

/**
* @brief interface function - decode
* @param amrnb is structure of snx_amrnb_st for internal codec
* @param p_src is the pointer of source stream
* @param p_dst is the pointer of source stream
* @param src_bytes is the size of source stream
* @param P_dst_bytes is the pointer the size decoded actually
* @warning must return success or fail
*/
int32_t snx_amrnb_decode(struct snx_amrnb_st *amrnb, uint8_t *p_src, uint8_t *p_dst, int32_t src_bytes, int32_t *p_dst_bytes);

/**
* @brief interface function - get information
* @param amrnb is structure of snx_amrnb_st for internal codec
* @param amrnb_info is structure of snx_amrnb_info_st for user
* @warning must return success or fail
*/
int32_t snx_amrnb_get_info (struct snx_amrnb_st *amrnb, struct snx_amrnb_info_st *amrnb_info);
#endif

