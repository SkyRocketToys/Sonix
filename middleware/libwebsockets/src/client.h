/**
 *  The MIT License (MIT)
 *
 *  Copyright (c) 2014 Jeremy Hahn
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in
 *  all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 *  THE SOFTWARE.
 */

#ifndef CWEBSOCKET_CLIENT_H
#define CWEBSOCKET_CLIENT_H

#include "lwip/tcpip.h"
#include "lwip/tcp.h"
#include "common.h"

#include "lwip/sockets.h"
#include "lwip/netdb.h"
#include <errno.h>

/**
 * @file
 * Websocket header file
 *
 * 
 */

 /** \defgroup func_libwebsockets libwebsockets function
 *  \ingroup mid_libwebsockets
 * @{
 */

#ifndef LWIP_PROVIDE_ERRNO
		#define LWIP_PROVIDE_ERRNO 1
#endif

#define WEBSOCKET_FLAG_AUTORECONNECT (1 << 1)

#ifdef __cplusplus
extern "C" {
#endif

//#define ENABLE_THREADS 1
//#define ENABLE_SSL 1	

#ifdef ENABLE_SSL
#include <cyassl/ssl.h>	
#endif	
	
typedef struct _cwebsocket {
	int fd;
	int retry;
	char *uri;
	char *hostname;
	unsigned short port;
	char *querystring;
	uint8_t flags;
	uint8_t state;
#ifdef ENABLE_SSL
	CYASSL_CTX *sslctx;
	CYASSL *ssl;
#endif	
#ifdef ENABLE_THREADS
	xTaskHandle thread;
	xSemaphoreHandle lock;
	xSemaphoreHandle write_lock;
#endif	
	size_t subprotocol_len;
	cwebsocket_subprotocol *subprotocol;
	cwebsocket_subprotocol *subprotocols[];
} cwebsocket_client;

typedef struct {
	cwebsocket_client *socket;
	cwebsocket_message *message;
} cwebsocket_client_thread_args;

// "public"

/** @brief Init function
 *  @param websocket cwebsocket_client structure
 *  @param subprotocols[] callback function array
 *  @param subprotocol_len funcion count
 * @return null
 */



void cwebsocket_client_init(cwebsocket_client *websocket, cwebsocket_subprotocol *subprotocols[], int subprotocol_len);

/** @brief Connect function
 *  @param websocket cwebsocket_client structurre
 * @return success : 0, fail : -1 
 */
int cwebsocket_client_connect(cwebsocket_client *websocket);

/** @brief read data function
 *  @param websocket cwebsocket_client structurre
 * @return reure data length from websocket, if len < 0 means fail.
 */
int cwebsocket_client_read_data(cwebsocket_client *websocket);
/** @brief write data function
 *  @param websocket cwebsocket_client structurre
 * @return reure length of bytes written to the websocket, if len < 0 means fail.
 */
int cwebsocket_client_write_data(cwebsocket_client *websocket, const char *data, uint32_t len, opcode code);
/** @brief close websocket
 *  @param websocket cwebsocket_client structurre
 * @return reure length of bytes written to the websocket, if len < 0 means fail.
 */
void cwebsocket_client_close(cwebsocket_client *websocket, uint16_t code, const char *reason);
void cwebsocket_client_listen(cwebsocket_client *websocket);

void cwebsocket_client_stop_read(cwebsocket_client *websocket);

// "private"
//void cwebsocket_client_parse_uri(cwebsocket_client *websocket, const char *uri, char *hostname, char *port, char *resource, char *querystring);
int cwebsocket_client_handshake_handler(cwebsocket_client *websocket, const char *handshake_response, char *seckey);
int cwebsocket_client_read_handshake(cwebsocket_client *websocket, char *seckey);
int cwebsocket_client_send_control_frame(cwebsocket_client *websocket, opcode opcode, const char *frame_type, uint8_t *payload, int payload_len);
void cwebsocket_client_create_masking_key(uint8_t *masking_key);
int cwebsocket_client_read(cwebsocket_client *websocket, void *buf, int len);
int cwebsocket_client_write(cwebsocket_client *websocket, void *buf, int len);
/** @brief When a websocket has opened
 *  @param websocket cwebsocket_client structurre
 */
void cwebsocket_client_onopen(cwebsocket_client *websocket);
/** @brief When a message has been received
 *  @param websocket cwebsocket_client structurre
 * @return reure length of bytes written to the websocket, if len < 0 means fail.
 */
void cwebsocket_client_onmessage(cwebsocket_client *websocket, cwebsocket_message *message);
/** @brief When a websocket has been closed
 *  @param websocket cwebsocket_client structurre
 * @return reure length of bytes written to the websocket, if len < 0 means fail.
 */
void cwebsocket_client_onclose(cwebsocket_client *websocket, int code, const char *message);
/** @brief When a error has been happened
 *  @param websocket cwebsocket_client structurre
 * @return reure length of bytes written to the websocket, if len < 0 means fail.
 */
void cwebsocket_client_onerror(cwebsocket_client *websocket, const char *error);

/** @} */

#ifdef __cplusplus
}
#endif

#endif
