/**
 * @file
 * this is internal header file for paring cmd,include this file before use
 * @author Algorithm Dept Sonix.
 */
#ifndef __JSON_CMD_INTERNAL_H__
#define __JSON_CMD_INTERNAL_H__

#include "libmid_json/snx_json.h"

#define JSON_PRINT(level,fmt,args...) print_q(level,"[json]%s(%u): "fmt,__func__,__LINE__,##args)

#define JSON_PRINT_QUEUE(fmt,args...) // ffffnnn print_msg_queue("[mf_json]%s: "fmt,__func__,##args)
#define JSON_PRINT_QUEUE_ERR(fmt,args...) print_msg_queue("[mf_json]%s:(error) "fmt,__func__,##args)

#define DAEMON_ERRNO_CLASS 0x01	/**< errno for daemon*/
#define DAEMON_ERRNO(id,errno) ((DAEMON_ERRNO_CLASS<<16) | (((id)&0xff)<<8) | ((errno)&0xff))

/**
* @brief infomation for cmd
*/
typedef struct _ActionHandler
{
	const char *cmd;
    buffer_t *(* handler)(snx_json_t *, void *);
} ActionHandler_t;

#endif // __JSON_CMD_INTERNAL_H__
