/*********************************************************************************
* /Sysinfo.c
*
* Implementation of schedule for recording internal APIs
*
* History:
*    2015/08/11 - [Allen_Chang] created file
*                                               copy lwip debug.h
*
* Copyright (C) 1996-2015, Sonix, Inc.
*
* All rights reserved. No Part of this file may be reproduced, stored
* in a retrieval system, or transmitted, in any form, or by any means,
* electronic, mechanical, photocopying, recording, or otherwise,
* without the prior consent of Sonix, Inc.
*
*********************************************************************************/



#ifndef __DEBUG_H__
#define __DEBUG_H__

#include "debug_option.h"
#include <nonstdlib.h>


/**CSTREAMER_DBG_LEVEL_ALL  lower two bits indicate debug level
 * - 0 all
 * - 1 warning
 * - 2 serious
 * - 3 severe
 */


#define CSTREAMER_DBG_LEVEL_ALL     0x00
#define CSTREAMER_DBG_LEVEL_OFF     LWIP_DBG_LEVEL_ALL /* compatibility define only */
#define CSTREAMER_DBG_LEVEL_WARNING 0x01               /* bad checksums, dropped packets, ... */
#define CSTREAMER_DBG_LEVEL_SERIOUS 0x02               /* memory allocation failures, ... */
#define CSTREAMER_DBG_LEVEL_SEVERE  0x03
#define CSTREAMER_DBG_MASK_LEVEL    0x03

/** flag for CSTREAMER_DEBUGF to enable that debug message */
#define CSTREAMER_DBG_ON            0x80U
/** flag for CSTREAMER_DEBUGF to disable that debug message */
#define CSTREAMER_DBG_OFF           0x00U




/** flag for CSTREAMER_DEBUGF indicating a tracing message (to follow program flow) */
#define CSTREAMER_DBG_TRACE         0x40U
/** flag for CSTREAMER_DEBUGF indicating a state debug message (to follow module states) */
#define CSTREAMER_DBG_STATE         0x20U
/** flag for CSTREAMER_DEBUGF indicating newly added code, not thoroughly tested yet */
#define CSTREAMER_DBG_FRESH         0x10U
/** flag for CSTREAMER_DEBUGF to halt after printing this debug message */
#define CSTREAMER_DBG_HALT          0x08U




#define CSTREAMER_PLATFORM_DIAG(x)   do {print_msg_queue x;} while(0)
#define CSTREAMER_PLATFORM_ASSERT(x) do {print_msg_queue("Assertion \"%s\" failed at line %d in %s\n", \
	        x, __LINE__, __FILE__); } while(0)



#ifdef CSTREAMER_ASSERT_FLAG
#define CSTREAMER_ASSERT(message, assertion) do { if(!(assertion)) \
			CSTREAMER_PLATFORM_ASSERT(message); } while(0)
#else  /* CSTREAMER_NOASSERT */
#define CSTREAMER_ASSERT(message, assertion)
#endif /* CSTREAMER_NOASSERT */

/** if "expression" isn't true, then print "message" and execute "handler" expression */
#ifndef CSTREAMER_ERROR
#define CSTREAMER_ERROR(message, expression, handler) do { if (!(expression)) { \
			CSTREAMER_PLATFORM_ASSERT(message); handler;}} while(0)
#endif /* CSTREAMER_ERROR */


#ifdef CSTREAMER_DEBUG_FLAG
/** print debug message only if debug message type is enabled...
 *  AND is of correct type AND is at least CSTREAMER_DBG_LEVEL
 */
#define CSTREAMER_DEBUGF(debug, message) do { \
		if ( \
		        ((debug) & CSTREAMER_DBG_ON) && \
		        ((debug) & CSTREAMER_DBG_TYPES_ON) && \
		        ((unsigned short)((debug) & CSTREAMER_DBG_MASK_LEVEL) >= CSTREAMER_DBG_MIN_LEVEL)) { \
			CSTREAMER_PLATFORM_DIAG(message); \
			if ((debug) & CSTREAMER_DBG_HALT) { \
				while(1); \
			} \
		} \
	} while(0)

#else  /* LWIP_DEBUG */
#define CSTREAMER_DEBUGF(debug, message)
#endif /* LWIP_DEBUG */




#endif /* __LWIP_DEBUG_H__ */

