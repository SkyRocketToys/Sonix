#ifndef _SNX_PCM_G726_H_
#define _SNX_PCM_G726_H_

#include "FreeRTOS.h"

/**
* @brief initial values for codecs parameters
*/
struct snx_g726_params_st {
	int32_t type;
	uint32_t samplerate;
	uint32_t bytes_per_frame;
	uint32_t pcm_bytes_per_frame;
	uint32_t compress_ratio;
};

struct snx_g726_info_st {
	int32_t type;
	uint32_t samplerate;
	uint32_t bitrate;
	uint32_t bytes_per_frame;
	uint32_t pcm_bytes_per_frame;
	uint32_t compress_ratio;
};

struct snx_g726_st;

/**
* @brief interface function - open
* @param params is structure of snx_g726_params_st for user
* @param g726 is structure of snx_g726_st for internal codec
* @warning must return success or fail
*/
int32_t snx_g726_open (struct snx_g726_params_st *params, struct snx_g726_st **g726);

/**
* @brief interface function - close
* @param g726 is structure of snx_g726_st for internal codec
* @warning must return success or fail
*/
int32_t snx_g726_close(struct snx_g726_st *g726);

/**
* @brief interface function - encode
* @param g726 is structure of snx_g726_st for internal codec
* @param p_src is the pointer of source stream
* @param p_dst is the pointer of source stream
* @param src_bytes is the size of source stream
* @param P_dst_bytes is the pointer the size encoded actually
* @warning must return success or fail
*/
int32_t snx_g726_encode(struct snx_g726_st *g726, uint8_t *p_src, uint8_t *p_dst, int32_t src_bytes, int32_t *p_dst_bytes);

/**
* @brief interface function - decode
* @param g726 is structure of snx_mulaw_st for internal codec
* @param p_src is the pointer of source stream
* @param p_dst is the pointer of source stream
* @param src_bytes is the size of source stream
* @param P_dst_bytes is the pointer the size decoded actually
* @warning must return success or fail
*/
int32_t snx_g726_decode(struct snx_g726_st *g726, uint8_t *p_src, uint8_t *p_dst, int32_t src_bytes, int32_t *p_dst_bytes);

/**
* @brief interface function - get information
* @param g726 is structure of snx_alaw_st for internal codec
* @param g726_info is structure of snx_alaw_info_st for user
* @warning must return success or fail
*/
int32_t snx_g726_get_info (struct snx_g726_st *g726, struct snx_g726_info_st *g726_info);

#endif
