#ifndef _SNX_PCM_ALAW_H_
#define _SNX_PCM_ALAW_H_

#include "FreeRTOS.h"

/**
* @brief initial values for codecs parameters
*/
struct snx_alaw_params_st {
	int32_t type;
	uint32_t samplerate;
};

struct snx_alaw_info_st {
	uint32_t samplerate;
	uint32_t bitrate;
};

struct snx_alaw_st;

/**
* @brief interface function - open
* @param params is structure of snx_alaw_params_st for user
* @param alaw is structure of snx_alaw_st for internal codec
* @warning must return success or fail
*/
int32_t snx_alaw_open (struct snx_alaw_params_st *params, struct snx_alaw_st **alaw);

/**
* @brief interface function - close
* @param alaw is structure of snx_alaw_st for internal codec
* @warning must return success or fail
*/
int32_t snx_alaw_close(struct snx_alaw_st *alaw);

/**
* @brief interface function - encode
* @param alaw is structure of snx_alaw_st for internal codec
* @param p_src is the pointer of source stream
* @param p_dst is the pointer of source stream
* @param src_bytes is the size of source stream
* @param P_dst_bytes is the pointer the size encoded actually
* @warning must return success or fail
*/
int32_t snx_alaw_encode(struct snx_alaw_st *alaw, uint8_t *p_src, uint8_t *p_dst, int32_t src_bytes, int32_t *p_dst_bytes);

/**
* @brief interface function - decode
* @param alaw is structure of snx_alaw_st for internal codec
* @param p_src is the pointer of source stream
* @param p_dst is the pointer of source stream
* @param src_bytes is the size of source stream
* @param P_dst_bytes is the pointer the size decoded actually
* @warning must return success or fail
*/
int32_t snx_alaw_decode(struct snx_alaw_st *alaw, uint8_t *p_src, uint8_t *p_dst, int32_t src_bytes, int32_t *p_dst_bytes);

/**
* @brief interface function - get information
* @param alaw is structure of snx_alaw_st for internal codec
* @param alaw_info is structure of snx_alaw_info_st for user
* @warning must return success or fail
*/
int32_t snx_alaw_get_info (struct snx_alaw_st *alaw, struct snx_alaw_info_st *alaw_info);
#endif

