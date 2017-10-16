/**
 * @file
 * this is audio error number file, include this file before use
 * audio_error.h
 * @author yanjie_yang
 */
#ifndef _AUDIO_ERROR_H_
#define _AUDIO_ERROR_H_
#include <errno.h>

#define AUDIO_ERROR_BASE	1000
/**
 * \defgroup AUDIO_ERROR The error number of audio
 * @{
 */
#define ECODEC			(AUDIO_ERROR_BASE + 1)
#define ECTRL			(AUDIO_ERROR_BASE + 2)
#define EBUF			(AUDIO_ERROR_BASE + 3)
#define EIRQ			(AUDIO_ERROR_BASE + 4)

#define ESTATUS			(AUDIO_ERROR_BASE + 30)

/** @} */

#endif
