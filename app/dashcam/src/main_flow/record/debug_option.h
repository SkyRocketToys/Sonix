#ifndef __DEBUG_OPT_H__
#define __DEBUG_OPT_H__

#include "debug.h"
#define CSTREAMER_ASSERT_FLAG	                 1
#define CSTREAMER_DEBUG_FLAG                     1

/*
   ---------------------------------------
   ---------- Debugging options ----------
   ---------------------------------------
*/
/**
 * CSTREAMER_DBG_MIN_LEVEL: After masking, the value of the debug is
 * compared against this value. If it is smaller, then debugging
 * messages are written.
 */
#ifndef CSTREAMER_DBG_MIN_LEVEL
#define CSTREAMER_DBG_MIN_LEVEL             CSTREAMER_DBG_LEVEL_ALL
#endif

/**
 * CSTREAMER_DBG_TYPES_ON: A mask that can be used to globally enable/disable
 * debug messages of certain types.
 */
#ifndef CSTREAMER_DBG_TYPES_ON
#define CSTREAMER_DBG_TYPES_ON              CSTREAMER_DBG_ON
#endif

/**
 * TEST_DEBUG: Enable debugging in debug.c.
 */
#ifndef TEST_DEBUG
#define TEST_DEBUG                          CSTREAMER_DBG_OFF
#endif

#ifndef TEST_DEBUG2
#define TEST_DEBUG2                         CSTREAMER_DBG_ON
#endif

#ifndef CS_MAIN_DEBUG
#define CS_MAIN_DEBUG                        CSTREAMER_DBG_ON
#endif

#ifndef CS_VIDEO_DEBUG
#define CS_VIDEO_DEBUG                       CSTREAMER_DBG_ON
#endif

#ifndef CS_AUDIO_DEBUG
#define CS_AUDIO_DEBUG                       CSTREAMER_DBG_ON
#endif

#ifndef CS_PLAYBACK_DEBUG
#define CS_PLAYBACK_DEBUG                    CSTREAMER_DBG_ON
#endif

#ifndef CS_SCAN_DEBUG
#define CS_SCAN_DEBUG                        CSTREAMER_DBG_ON
#endif

#ifndef CS_RECORD_DEBUG
#define CS_RECORD_DEBUG                        CSTREAMER_DBG_ON
#endif


#ifndef CS_PLAYBACK_DEBUG
#define CS_PLAYBACK_DEBUG                        CSTREAMER_DBG_ON
#endif


#endif /* __LWIP_OPT_H__ */

