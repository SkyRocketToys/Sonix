#ifndef _AUDIO_CODEC_H_
#define _AUDIO_CODEC_H_

#ifdef __cplusplus
extern "C" {
#endif



#define AUD_CODEC_SUCCESS			0

#define AUD_CODEC_ERR_BASE			0x100
#define AUD_CODEC_ERR_INVALID_FORMAT		(-(AUD_CODEC_ERR_BASE + 1))
#define AUD_CODEC_ERR_INVALID_PARAMETER		(-(AUD_CODEC_ERR_BASE + 2))
#define AUD_CODEC_ERR_INSUFFICIENT_RESOUCE	(-(AUD_CODEC_ERR_BASE + 3))
#define AUD_CODEC_ERR_MEM_ALIGN			    (-(AUD_CODEC_ERR_BASE + 4))
#define AUD_CODEC_ERR_PROCESS_FAIL			(-(AUD_CODEC_ERR_BASE + 5))
#define AUD_CODEC_ERR_MOREDATA   			(-(AUD_CODEC_ERR_BASE + 5))

enum codec_type
{
	AUD_CODEC_ENCODER = 0x100,
	AUD_CODEC_DECODER
};



#ifdef __cplusplus
}
#endif

#endif
