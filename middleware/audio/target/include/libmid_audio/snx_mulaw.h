#ifndef _SNX_PCM_MULAW_H_
#define _SNX_PCM_MULAW_H_

#include "FreeRTOS.h"

/**
* @brief initial values for codecs parameters
*/
struct snx_mulaw_params_st {
	int32_t type;
	uint32_t samplerate;
};

struct snx_mulaw_info_st {
	uint32_t samplerate;
	uint32_t bitrate;
};

struct snx_mulaw_st;

/**
* @brief interface function - open
* @param params is structure of snx_mulaw_params_st for user
* @param mulaw is structure of snx_mulaw_st for internal codec
* @warning must return success or fail
*/
int32_t snx_mulaw_open (struct snx_mulaw_params_st *params, struct snx_mulaw_st **mulaw);

/**
* @brief interface function - close
* @param mulaw is structure of snx_mulaw_st for internal codec
* @warning must return success or fail
*/
int32_t snx_mulaw_close(struct snx_mulaw_st *mulaw);

/**
* @brief interface function - encode
* @param mulaw is structure of snx_mulaw_st for internal codec
* @param p_src is the pointer of source stream
* @param p_dst is the pointer of source stream
* @param src_bytes is the size of source stream
* @param P_dst_bytes is the pointer the size encoded actually
* @warning must return success or fail
*/
int32_t snx_mulaw_encode(struct snx_mulaw_st *mulaw, uint8_t *p_src, uint8_t *p_dst, int32_t src_bytes, int32_t *p_dst_bytes);

/**
* @brief interface function - decode
* @param mulaw is structure of snx_mulaw_st for internal codec
* @param p_src is the pointer of source stream
* @param p_dst is the pointer of source stream
* @param src_bytes is the size of source stream
* @param P_dst_bytes is the pointer the size decoded actually
* @warning must return success or fail
*/
int32_t snx_mulaw_decode(struct snx_mulaw_st *mulaw, uint8_t *p_src, uint8_t *p_dst, int32_t src_bytes, int32_t *p_dst_bytes);

/**
* @brief interface function - get information
* @param mulaw is structure of snx_mulaw_st for internal codec
* @param mulaw_info is structure of snx_mulaw_info_st for user
* @warning must return success or fail
*/
int32_t snx_mulaw_get_info (struct snx_mulaw_st *mulaw, struct snx_mulaw_info_st *mulaw_info);
#endif


