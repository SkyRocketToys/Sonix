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

#include "client.h"
#include <FreeRTOS.h>
#include <task.h>
#include <bsp.h>

/** \defgroup mid_libwebsockets libwebsockets middleware modules
 * \n
 * @{
 */
/** @} */

/** \defgroup func_libwebsockets libwebsockets function
 *  \ingroup mid_libwebsockets
 * @{
 */

/**
 * @brief client.c
 *
 * pointers - websockets protocol
 */
 
 /**
* @brief interface function - encode callback
* @param codec structure for codec
* @param src source buffer pointer.
* @param src_size source buffer data size.
* @param dst destination buffer pointer.
* @param dst_size destination buffer data size.
* @return The size of compression output data
* @warning must return valid size
*/

/** @} */

#ifdef ENABLE_THREADS
  int webSocketInitMutex(xSemaphoreHandle* m)
	{
			int iReturn;

			*m = ( xSemaphoreHandle ) xSemaphoreCreateMutex();
			if( *m != NULL )
					iReturn = 0;
			else
					iReturn = -1;

			return iReturn;
	}

	int webSocketFreeMutex(xSemaphoreHandle* m)
	{
			vSemaphoreDelete( *m );
			return 0;
	}

	int webSocketLockMutex(xSemaphoreHandle* m)
	{
			/* Assume an infinite block, or should there be zero block? */
			xSemaphoreTake( *m, portMAX_DELAY );
			return 0;
	}

	int webSocketUnLockMutex(xSemaphoreHandle* m)
	{
			xSemaphoreGive( *m );
			return 0;
	}
#endif


cwebsocket_client *gWebsocket;
xTaskHandle ws_read_tsk;

//add by steven, 20150902
#define DefaultFrameLength 30000
char *mFrameBuf= NULL;
int mFrameBufLength = DefaultFrameLength;

//Read Task
void vReadTaskFunction( void *pvParameters )
{
	int readsByte;
	for (;;)
	{
		readsByte = cwebsocket_client_read_data(gWebsocket);
		vTaskDelay(1);
	}

	vTaskDelete(NULL);
}


void cwebsocket_client_init(cwebsocket_client *websocket, cwebsocket_subprotocol *subprotocols[], int subprotocol_len) {
	websocket->fd = 0;
	websocket->retry = 0;
	websocket->uri = '\0';
	websocket->hostname = "\0";
	websocket->port = "\0";
	websocket->querystring = "\0";
	websocket->flags = 0;
	websocket->state = WEBSOCKET_STATE_CLOSED;
	websocket->subprotocol_len = subprotocol_len;

    //add by steven, 2015/09/02
    mFrameBuf = NULL;
	mFrameBufLength = DefaultFrameLength;

	int i;
	for(i=0; i<subprotocol_len; i++) {
		websocket->subprotocols[i] = subprotocols[i];
	}



}
/*
void cwebsocket_client_parse_uri(cwebsocket_client *websocket, const char *uri,
		char *hostname, char *port, char *resource, char *querystring) {
	print_msg("\nIn parse uri ...\n");
	if(sscanf(uri, "ws://%[^:]:%[^/]%[^?]%s", hostname, port, resource, querystring) == 4) {
		print_msg("return 1 ...\n");
		return;
	}
	else if(sscanf(uri, "ws://%[^:]:%[^/]%s", hostname, port, resource) == 3) {
		strcpy(querystring, "");
		print_msg("return 2 ...\n");
		return;
	}
	else if(sscanf(uri, "ws://%[^:]:%[^/]%s", hostname, port, resource) == 2) {
		strcpy(resource, "/");
		strcpy(querystring, "");
		print_msg("return 3 ...\n");
		return;
	}
	else if(sscanf(uri, "ws://%[^/]%s", hostname, resource) == 2) {
		strcpy(port, "80");
		strcpy(querystring, "");
		print_msg("return 4 ...\n");
		return;
	}
	else if(sscanf(uri, "ws://%[^/]", hostname) == 1) {
		strcpy(port, "80");
		strcpy(resource, "/");
		strcpy(querystring, "");
		print_msg("return 5 ...");
		return;
	}
#ifdef ENABLE_SSL
	else if(sscanf(uri, "wss://%[^:]:%[^/]%[^?]%s", hostname, port, resource, querystring) == 4) {
		websocket->flags |= WEBSOCKET_FLAG_SSL;
		return;
	}
	else if(sscanf(uri, "wss://%[^:]:%[^/]%s", hostname, port, resource) == 3) {
		strcpy(querystring, "");
		websocket->flags |= WEBSOCKET_FLAG_SSL;
		return;
	}
	else if(sscanf(uri, "wss://%[^:]:%[^/]%s", hostname, port, resource) == 2) {
		strcpy(resource, "/");
		strcpy(querystring, "");
		websocket->flags |= WEBSOCKET_FLAG_SSL;
		return;
	}
	else if(sscanf(uri, "wss://%[^/]%s", hostname, resource) == 2) {
		strcpy(port, "443");
		strcpy(querystring, "");
		websocket->flags |= WEBSOCKET_FLAG_SSL;
		return;
	}
	else if(sscanf(uri, "wss://%[^/]", hostname) == 1) {
		strcpy(port, "443");
		strcpy(resource, "/");
		strcpy(querystring, "");
		websocket->flags |= WEBSOCKET_FLAG_SSL;
		return;
	}
#endif
	else if(strstr(uri, "wss://") > 0) {
		//syslog(LOG_CRIT, "cwebsocket_client_parse_uri: recompile with SSL support to use a secure connection");
		//exit(1);
		print_msg("parse uri fail 1 ...");
	}
	else {
		//syslog(LOG_CRIT, "cwebsocket_client_parse_uri: invalid websocket URL\n");
		//exit(1);
		print_msg("parse ip & port  ...\n");
		strcpy(hostname, "192.168.119.2");
		strcpy(port, "8080");


	}
}
*/



int cwebsocket_client_start_read(cwebsocket_client *websocket){

	//add by steven, 20150810
	//add by steven
	gWebsocket = websocket;
	static unsigned char ucParameterToPass;
	if (pdPASS != xTaskCreate(vReadTaskFunction, "ReadTask", STACK_SIZE_4K, (void*) &ucParameterToPass,	105, &ws_read_tsk))
		print_msg("Websocket Err : Create Websockets Read Task Fail !!");

	return 0;
}

void cwebsocket_client_stop_read(cwebsocket_client *websocket)
{
	vTaskDelete(ws_read_tsk);
}

int cwebsocket_client_connect(cwebsocket_client *websocket) {

	int status = 0;

	if(websocket->state & WEBSOCKET_STATE_CONNECTED) {
		print_msg("Websocket Err : connection fail AA\n");
		//syslog(LOG_CRIT, "cwebsocket_client_connect: socket already connected");
		return -1;
	}

	if(websocket->state & WEBSOCKET_STATE_CONNECTING) {
		//syslog(LOG_CRIT, "cwebsocket_client_connect: socket already connecting");
		print_msg("Websocket Err : connection fail BB\n");
		return -1;
	}

	if(websocket->state & WEBSOCKET_STATE_OPEN) {
		//syslog(LOG_CRIT, "cwebsocket_client_connect: socket already open");
		print_msg("Websocket Err : connection fail CC\n");
		return -1;
	}

#ifdef ENABLE_THREADS
	if(webSocketInitMutex(&websocket->lock) != 0) {
		//syslog(LOG_ERR, "cwebsocket_client_connect: unable to initialize websocket mutex: %s\n", strerror(errno));
		//serial_printf("----- connection fail DD\n");
		cwebsocket_client_onerror(websocket, strerror(errno));
		
		return -1;
	}
	if(webSocketInitMutex(&websocket->write_lock) != 0) {
		//syslog(LOG_ERR, "cwebsocket_client_connect: unable to initialize websocket write mutex: %s\n", strerror(errno));
		//serial_printf("----- connection fail EE\n");
		cwebsocket_client_onerror(websocket, strerror(errno));
		
		return -1;
	}
	webSocketLockMutex(&websocket->lock);
	websocket->state = WEBSOCKET_STATE_CONNECTING;
	webSocketUnLockMutex(&websocket->lock);
#else
	websocket->state = WEBSOCKET_STATE_CONNECTING;
#endif


	//add by steven, 2015/09/02
	mFrameBuf = NULL;
	mFrameBuf = malloc(mFrameBufLength);
	if(mFrameBuf == NULL){
	   cwebsocket_client_onerror(websocket, "Malloc mFrameBuf fail !!");
	   return -1;
	}
	memset(mFrameBuf, 0x00, sizeof(char)*mFrameBufLength);
	print_msg("+++ Malloc mFrameBuf !!");
   //


	char hostname[100] = {0};
	char port[6] = {0};
	char* resource = malloc(256);
	char* querystring = malloc(256);

	memset(resource, 0x00, sizeof(char)*256);
	memset(querystring, 0x00, sizeof(char)*256);

	strcpy(hostname, websocket->hostname);
	snprintf(port, sizeof(port), "%d", websocket->port);
	strcpy(querystring, websocket->querystring);

	//cwebsocket_client_parse_uri(websocket, websocket->uri, hostname, port, resource, querystring);
	print_msg("MyWebSocket cwebsocket_client_connect: hostname=%s, port=%s, resource=%s, querystring=%s, secure=%i\n", hostname, port, resource, querystring, (websocket->flags & WEBSOCKET_FLAG_SSL));


	char* handshake = malloc(1024);
    struct addrinfo hints;
	struct addrinfo *servinfo;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	//srand(time(NULL));
	//char nonce[16];
	//static const char alphanum[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVXYZabcdefghijklmnopqrstuvwxyz";
	int i;
	//for(i = 0; i < 16; i++) {
		//nonce[i] = alphanum[rand() % 61];
	//}
	//char *seckey = cwebsocket_base64_encode((const unsigned char *)nonce, sizeof(nonce));
	char *seckey = "AlNFfO2m3GrJLr0ZGHdz5A==";

	snprintf(handshake, 1024,
		      "GET %s%s HTTP/1.1\r\n"
		      "Host: %s:%s\r\n"
		      "Upgrade: WebSocket\r\n"
		      "Connection: Upgrade\r\n"
		      "Sec-WebSocket-Key: %s\r\n"
		      "Sec-WebSocket-Version: 13\r\n"
			  ,resource, querystring, hostname, port, seckey);

print_msg("Device Send : \nGET %s%s HTTP/1.1\r\n"
		      "Host: %s:%s\r\n"
		      "Upgrade: WebSocket\r\n"
		      "Connection: Upgrade\r\n"
		      "Sec-WebSocket-Key: %s\r\n"
		      "Sec-WebSocket-Version: 13\r\n"
			  ,resource, querystring, hostname, port, seckey);


	if(websocket->subprotocol_len > 0) {
		strcat(handshake, "Sec-WebSocket-Protocol: ");
		for(i=0; i<websocket->subprotocol_len; i++) {
			strcat(handshake, websocket->subprotocols[i]->name);
			if(i<websocket->subprotocol_len) {
				strcat(handshake, " ");
			}
			else {
				strcat(handshake, "\r\n");
			}
		}
	}

	strcat(handshake, "\r\n");

	if(getaddrinfo(hostname, port, &hints, &servinfo) != 0 ) {
		freeaddrinfo(servinfo);
		const char *errmsg = "invalid hostname or IP";
		cwebsocket_client_onerror(websocket, errmsg);
		//serial_printf("----- connection fail 11\n");
		status = -1;
		goto end;
	}

	websocket->fd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
	print_msg("create socket fd =%i\n", websocket->fd);
	if(websocket->fd < 0) {
		freeaddrinfo(servinfo);
		cwebsocket_client_onerror(websocket, "fd is null");//strerror(errno));
		print_msg("----- connection fail 22\n");
		status = -1;
		goto end;
	}

	if(connect(websocket->fd, servinfo->ai_addr, servinfo->ai_addrlen) != 0 ) {
		cwebsocket_client_onerror(websocket, "connect fail");//strerror(errno));
		websocket->state = WEBSOCKET_STATE_CLOSED;
		if(websocket->retry > 0) {
			vTaskDelay(websocket->retry);
			cwebsocket_client_connect(websocket);
		}
		print_msg("----- connection fail 33\n");
		status = -1;
		goto end;
	}
	
	//set recv timeout
  int nNetTimeout=2000;
  setsockopt(websocket->fd,SOL_SOCKET,SO_SNDTIMEO,(char *)&nNetTimeout,sizeof(int));
  setsockopt(websocket->fd,SOL_SOCKET,SO_RCVTIMEO,(char *)&nNetTimeout,sizeof(int));
	
	
	freeaddrinfo(servinfo);

    //int optval = 1;
    //if(setsockopt(websocket->fd, SOL_SOCKET, SO_KEEPALIVE, &optval, sizeof optval) == -1) {
		//cwebsocket_client_onerror(websocket, strerror(errno));
		//return -1;
   // }

#ifdef ENABLE_SSL

    websocket->ssl = NULL;
    websocket->sslctx = NULL;

	if(websocket->flags & WEBSOCKET_FLAG_SSL) {

	   websocket->sslctx = CyaSSL_CTX_new(CyaTLSv1_2_client_method());
	   if(websocket->sslctx == NULL) {
			status = -1;
			goto end;
	   }

	   websocket->ssl = CyaSSL_new(websocket->sslctx);
	   if(websocket->ssl == NULL) {
			status = -1;
			goto end;
	   }

	   if(!CyaSSL_set_fd(websocket->ssl, websocket->fd)) {
			status = -1;
			goto end;
	   }

	   if(CyaSSL_connect(websocket->ssl) != 1) {
			status = -1;
			goto end;
	   }
	}
#endif

#ifdef ENABLE_THREADS
	webSocketLockMutex(&websocket->lock);
	websocket->state = WEBSOCKET_STATE_CONNECTED;
	webSocketUnLockMutex(&websocket->lock);
#else
	print_msg("----- connection step BBBB\n");
	websocket->state = WEBSOCKET_STATE_CONNECTED;
#endif

	if(cwebsocket_client_write(websocket, handshake, strlen(handshake)) == -1) {
		//cwebsocket_client_onerror(websocket, strerror(errno));
		print_msg("----- connection fail 44\n");
		status = -1;
		goto end;
	}

	if(cwebsocket_client_read_handshake(websocket, seckey) == -1) {
		//cwebsocket_client_onerror(websocket, strerror(errno));
		print_msg("----- connection fail 55\n");
		status = -1;
		goto end;
	}

#ifdef ENABLE_THREADS
	webSocketLockMutex(&websocket->lock);
	websocket->state = WEBSOCKET_STATE_OPEN;
	webSocketUnLockMutex(&websocket->lock);
#else
	websocket->state = WEBSOCKET_STATE_OPEN;
#endif
	print_msg("====>Call onopen: fd=%i\n", websocket->fd);
	cwebsocket_client_onopen(websocket);




	end:
	free(handshake);
	free(querystring);
	free(resource);
	return status;
}

int cwebsocket_client_handshake_handler(cwebsocket_client *websocket, const char *handshake_response, char *seckey) {
	uint8_t flags = 0;
	char *ptr = NULL, *token = NULL;



		print_msg("HTTP Response===>\n %s", handshake_response);
	print_msg("\n");



	for(token = strtok((char *)handshake_response, "\r\n"); token != NULL; token = strtok(NULL, "\r\n")) {
		if(*token == 'H' && *(token+1) == 'T' && *(token+2) == 'T' && *(token+3) == 'P') {
			ptr = strchr(token, ' ');
			ptr = strchr(ptr+1, ' ');
			*ptr = '\0';

			if(strcmp(token, "HTTP/1.1 101") != 0 && strcmp(token, "HTTP/1.0 101") != 0) {
				cwebsocket_client_onerror(websocket, "cwebsocket_client_handshake_handler: invalid HTTP status response code");
				return -1;
			}

		} else {
			ptr = strchr(token, ' ');
			if(ptr == NULL) {
				cwebsocket_client_onerror(websocket, "invalid HTTP header sent");
				return -1;
			}
			*ptr = '\0';
			if(strcasecmp(token, "Upgrade:") == 0) {
				if(strcasecmp(ptr+1, "websocket") != 0) {
					cwebsocket_client_onerror(websocket, "cwebsocket_client_handshake_handler: invalid HTTP upgrade header");
					return -1;
				}
				flags |= CWS_HANDSHAKE_HAS_UPGRADE;
			}
			if(strcasecmp(token, "Connection:") == 0) {
				if(strcasecmp(ptr+1, "upgrade") != 0) {
					cwebsocket_client_onerror(websocket, "cwebsocket_client_handshake_handler: invalid HTTP connection header");
					return -1;
				}
				flags |= CWS_HANDSHAKE_HAS_CONNECTION;
			}
			if(strcasecmp(token, "Sec-WebSocket-Protocol:") == 0) {
				int i;
				for(i=0; i<websocket->subprotocol_len; i++) {
					if(strcasecmp(ptr+1, websocket->subprotocols[i]->name) == 0) {
						websocket->subprotocol = websocket->subprotocols[i];
					}
				}
			}
			if(strcasecmp(token, "Sec-WebSocket-Accept:") == 0) {
#if 0
    			char* response = cwebsocket_create_key_challenge_response(seckey);
				if(strcmp(ptr+1, response) != 0) {
					free(seckey);
					if(websocket->subprotocol->onerror != NULL) {
						char errmsg[255];
						sprintf(errmsg, "cwebsocket_client_handshake_handler: Sec-WebSocket-Accept header does not match computed sha1/base64 response. expected=%s, actual=%s", response, ptr+1);
						cwebsocket_client_onerror(websocket, errmsg);
						free(response);
					    return -1;
					}
					return -1;
				}
				free(response);				
				free(seckey);
#endif				
				flags |= CWS_HANDSHAKE_HAS_ACCEPT;
			}
		}
	}
	if(((flags & CWS_HANDSHAKE_HAS_UPGRADE) == 0) || ((flags & CWS_HANDSHAKE_HAS_CONNECTION) == 0) ||
				((flags & CWS_HANDSHAKE_HAS_ACCEPT) == 0)) {
		// TODO send http error code (500?)

		cwebsocket_client_close(websocket, 1002, "invalid websocket HTTP headers");
		return -1;
	}
	return 0;
}

int cwebsocket_client_read_handshake(cwebsocket_client *websocket, char *seckey) {

	int byte, tmplen = 0;
	uint32_t bytes_read = 0;
	uint8_t data[CWS_HANDSHAKE_BUFFER_MAX];
	memset(data, 0, CWS_HANDSHAKE_BUFFER_MAX);

	//print_msg("HandShakr 1 !!\n");
	while(bytes_read <= CWS_HANDSHAKE_BUFFER_MAX) {
		//print_msg("HandShakr while loop !!\n");
		byte = cwebsocket_client_read(websocket, data+bytes_read, 1);
		//print_msg("HandShakr 2 !!\n");
		if(byte == 0) return -1;
		//print_msg("HandShakr 3 !!\n");
		if(byte == -1) {
			//cwebsocket_client_onerror(websocket, strerror(errno));
			return -1;
		}
		//print_msg("HandShakr 4 !!\n");
		if(bytes_read == CWS_HANDSHAKE_BUFFER_MAX) {
			cwebsocket_client_onerror(websocket, "handshake response too large");
			return -1;
		}
		//print_msg("HandShakr 5 !!\n");
		if((data[bytes_read] == '\n' && data[bytes_read-1] == '\r' && data[bytes_read-2] == '\n' && data[bytes_read-3] == '\r')) {
			break;
		}
		bytes_read++;
	}

	tmplen = bytes_read - 3;
	char buf[tmplen+1];
	memcpy(buf, data, tmplen);
	buf[tmplen+1] = '\0';

	return cwebsocket_client_handshake_handler(websocket, buf, seckey);
}

void cwebsocket_client_listen(cwebsocket_client *websocket) {
	while(websocket->state & WEBSOCKET_STATE_OPEN) {
		cwebsocket_client_read_data(websocket);
	}
}

#ifdef ENABLE_THREADS
void *cwebsocket_client_onmessage_thread(void *ptr) {
	cwebsocket_client_thread_args *args = (cwebsocket_client_thread_args *)ptr;
	cwebsocket_client_onmessage(args->socket, args->message);
	//free(args->message->payload);
	free(args->message);
	free(ptr);
	return NULL;
}
#endif

int cwebsocket_client_send_control_frame(cwebsocket_client *websocket, opcode code, const char *frame_type, uint8_t *payload, int payload_len) {
	if(websocket->fd <= 0) return -1;
	int bytes_written;
	int header_len = 6;
	int frame_len = header_len + payload_len;
	uint8_t control_frame[frame_len];
	memset(control_frame, 0, frame_len);
	uint8_t masking_key[4];
	cwebsocket_client_create_masking_key(masking_key);
	control_frame[0] = (code | 0x80);
	control_frame[1] = (payload_len | 0x80);
	control_frame[2] = masking_key[0];
	control_frame[3] = masking_key[1];
	control_frame[4] = masking_key[2];
	control_frame[5] = masking_key[3];
	if(code & CLOSE) {
		uint16_t close_code = 1000;
		if(payload_len >= 2) {
		   if(payload_len > 2) {
			  char parsed_payload[payload_len];
			  memcpy(parsed_payload, &payload[0], payload_len);
			  parsed_payload[payload_len] = '\0';
			  close_code = (control_frame[6] << 8) + control_frame[7];
			  int i;
			  for(i=0; i<payload_len; i++) {
				  control_frame[6+i] = (parsed_payload[i] ^ masking_key[i % 4]) & 0xff;
			  }

		   }
		   else {

		   }
		}
		else {

		}
	}
	else {
		int i;
		for(i=0; i<payload_len; i++) {
			control_frame[header_len+i] = (payload[i] ^ masking_key[i % 4]) & 0xff;
		}
	}
	bytes_written = cwebsocket_client_write(websocket, control_frame, frame_len);
	if(bytes_written == 0) {
		return 0;
	}
	else if(bytes_written == -1) {
		//cwebsocket_client_onerror(websocket, strerror(errno));
		return -1;
	}
	else {
	}
	return bytes_written;
}

int cwebsocket_client_read_data(cwebsocket_client *websocket) {

	int header_length = 2, bytes_read = 0;
	const int header_length_offset = 2;
	const int extended_payload16_end_byte = 4;
	const int extended_payload64_end_byte = 10;
	uint32_t payload_length = 0;

	uint8_t *data = malloc(CWS_DATA_BUFFER_MAX);
	if(data == NULL) {
		//perror("out of memory");
		//exit(-1);
	}
	memset(data, 0, CWS_DATA_BUFFER_MAX);

	cwebsocket_frame frame;
	memset(&frame, 0, sizeof(frame));

	uint32_t frame_size = header_length;
	while(bytes_read < frame_size && (websocket->state & WEBSOCKET_STATE_OPEN)) {

		if(bytes_read >= CWS_DATA_BUFFER_MAX) {
			cwebsocket_client_close(websocket, 1009, "frame too large");
			return -1;
		}

		int byte = cwebsocket_client_read(websocket, data+bytes_read, 1);

		if(byte == 0) {
		   char *errmsg = "server closed the connection";
		   //mark by steven, 20150828
		   cwebsocket_client_onerror(websocket, errmsg);
		   cwebsocket_client_close(websocket, 1006, errmsg);
		   return -1;
		}



		if(byte == -1) {
		   //cwebsocket_client_onerror(websocket, strerror(errno));
		   return -1;
		}
		bytes_read++;

		if(bytes_read == header_length_offset) {

		   frame.fin = (data[0] & 0x80) == 0x80 ? 1 : 0;
		   frame.rsv1 = (data[0] & 0x40) == 0x40 ? 1 : 0;
		   frame.rsv2 = (data[0] & 0x20) == 0x20 ? 1 : 0;
		   frame.rsv3 = (data[0] & 0x10) == 0x10 ? 1 : 0;
		   frame.opcode = (data[0] & 0x7F);
		   frame.mask = data[1] & 0x80;
		   frame.payload_len = (data[1] & 0x7F);

		   if(frame.mask == 1) {
			   const char *errmsg = "received masked frame from server";
			   cwebsocket_client_onerror(websocket, errmsg);
			   return -1;
		   }

		   payload_length = frame.payload_len;
		   frame_size = header_length + payload_length;

		}

		if(frame.payload_len == 126 && bytes_read == extended_payload16_end_byte) {
			print_msg("\nRead, payload_len = 126\n");
			header_length += 2;

			uint16_t extended_payload_length = 0;
			extended_payload_length |= ((uint8_t) data[2]) << 8;
			extended_payload_length |= ((uint8_t) data[3]) << 0;

			payload_length = extended_payload_length;
			frame_size = header_length + payload_length;

		}
		else if(frame.payload_len == 127 && bytes_read == extended_payload64_end_byte) {
			print_msg("\nRead, payload_len = 127\n");
			header_length += 6;

			uint64_t extended_payload_length = 0;
			extended_payload_length |= ((uint64_t) data[2]) << 56;
			extended_payload_length |= ((uint64_t) data[3]) << 48;
			extended_payload_length |= ((uint64_t) data[4]) << 40;
			extended_payload_length |= ((uint64_t) data[5]) << 32;
			extended_payload_length |= ((uint64_t) data[6]) << 24;
			extended_payload_length |= ((uint64_t) data[7]) << 16;
			extended_payload_length |= ((uint64_t) data[8]) << 8;
			extended_payload_length |= ((uint64_t) data[9]) << 0;

			payload_length = extended_payload_length;
			frame_size = header_length + payload_length;


		}
	}

	if(frame.fin && frame.opcode == TEXT_FRAME) {

		char *payload = malloc(sizeof(char) * payload_length+1);
		if(payload == NULL) {
			//perror("out of memory");
			//exit(-1);
			//serial_printf("out of memory\n");
		}

		memcpy(payload, &data[header_length], payload_length);
		payload[payload_length] = '\0';
		free(data);

		size_t utf8_code_points = 0;
		if(utf8_count_code_points((uint8_t *)payload, &utf8_code_points)) {
			cwebsocket_client_onerror(websocket, "received malformed utf8 payload");
			return -1;
		}

		if(websocket->subprotocol != NULL && websocket->subprotocol->onmessage != NULL) {

#ifdef ENABLE_THREADS
			cwebsocket_message *message = malloc(sizeof(cwebsocket_message));
			if(message == NULL) {
				//perror("out of memory");
				//exit(-1);
			}
			memset(message, 0, sizeof(cwebsocket_message));
			message->opcode = frame.opcode;
			message->payload_len = frame.payload_len;
			message->payload = payload;

		    cwebsocket_client_thread_args *args = malloc(sizeof(cwebsocket_client_thread_args));
		    if(args == NULL) {
				//perror("out of memory");
				//exit(-1);
			}
		    memset(args, 0, sizeof(cwebsocket_client_thread_args));
		    args->socket = websocket;
		    args->message = message;

		    //if(pthread_create(&websocket->thread, NULL, cwebsocket_client_onmessage_thread, (void *)args) == -1) {
		    	//cwebsocket_client_onerror(websocket, strerror(errno));
		    	//return -1;
		    //}
			  	xTaskCreate(cwebsocket_client_onmessage_thread,
					(signed char*)"websocket",
					256,
					(void *)args,
					11,
					&websocket->thread); 
					
		    return bytes_read;
#else
		    cwebsocket_message message = {0};
			message.opcode = frame.opcode;
			message.payload_len = payload_length;//frame.payload_len; //modify by steven, 20150827
			message.payload = payload;
		    cwebsocket_client_onmessage(websocket, &message);
			//free(payload);
		    return bytes_read;
#endif
		}

		return bytes_read;
	}
	else if(frame.fin && frame.opcode == BINARY_FRAME) {
//serial_printf("----- websocket read data cccc\n");
		char *payload = malloc(sizeof(char) * payload_length);
		if(payload == NULL) {
			//perror("out of memory");
			//exit(-1);
		}

		memcpy(payload, &data[header_length], payload_length);
		free(data);
//print_msg("----- websocket read data, length : %d\n", payload_length);
		if(websocket->subprotocol->onmessage != NULL) {

#ifdef ENABLE_THREADS
			cwebsocket_message *message = malloc(sizeof(cwebsocket_message));
			if(message == NULL) {
				//perror("out of memory");
				//exit(-1);
			}
			message->opcode = frame.opcode;
			message->payload_len = frame.payload_len;
			message->payload = payload;
//serial_printf("----- websocket read data eeee\n");
			cwebsocket_client_thread_args *args = malloc(sizeof(cwebsocket_client_thread_args));
			args->socket = websocket;
			args->message = message;

			//if(pthread_create(&websocket->thread, NULL, cwebsocket_client_onmessage_thread, (void *)args) == -1) {
				//cwebsocket_client_onerror(websocket, strerror(errno));
				//return -1;
			//}
			xTaskCreate(cwebsocket_client_onmessage_thread,
					(signed char*)"websocket",
					256,
					(void *)args,
					11,
					&websocket->thread);
					
			return bytes_read;
#else
           //print_msg("----- websocket read data abcd\n");

			cwebsocket_message message;
			message.opcode = frame.opcode;
			message.payload_len = payload_length;//frame.payload_len; //modify by steven, 20150827
			message.payload = payload;
			websocket->subprotocol->onmessage(websocket, &message);
			free(payload);
			return bytes_read;
#endif
		}
		return bytes_read;
	}
	else if(frame.opcode == CONTINUATION) {
		return 0;
	}
	else if(frame.opcode == PING) {
		if(frame.fin == 0) {
			cwebsocket_client_close(websocket, 1002, "control message must not be fragmented");
		}
		if(frame.payload_len > 125) {
			cwebsocket_client_close(websocket, 1002, "control frames must not exceed 125 bytes");
			return -1;
		}
		uint8_t payload[payload_length];
		memcpy(payload, &data[header_length], payload_length);
		payload[payload_length] = '\0';
		free(data);
		return cwebsocket_client_send_control_frame(websocket, 0x0A, "PONG", payload, payload_length);
	}
	else if(frame.opcode == PONG) {
		return 0;
	}
	else if(frame.opcode == CLOSE) {
		//serial_printf("----- websocket read data ffff\n");
		if(frame.payload_len > 125) {
			cwebsocket_client_close(websocket, 1002, "control frames must not exceed 125 bytes");
			return -1;
		}
		int code = 0;
		if(payload_length > 2) {
		   code = (data[header_length] << 8) + (data[header_length+1]);
		   header_length += 2;
		   payload_length -= 2;
		}
		uint8_t payload[payload_length];
		memcpy(payload, &data[header_length], (payload_length) * sizeof(uint8_t));
		payload[payload_length] = '\0';
		free(data);
		cwebsocket_client_close(websocket, code, NULL);
		return 0;
	}
//serial_printf("----- websocket read data gggg\n");
	free(data);
	char closemsg[50];
	sprintf(closemsg, "received unsupported opcode: %#04x", frame.opcode);
	//cwebsocket_print_frame(&frame);
	cwebsocket_client_onerror(websocket, closemsg);
	cwebsocket_client_close(websocket, 1002, closemsg);
	return -1;
}

void cwebsocket_client_create_masking_key(uint8_t *masking_key) {
	uint8_t mask_bit;
	//struct timeval tv;
	//gettimeofday(&tv, NULL);
	//srand(tv.tv_usec * tv.tv_sec);
	srand(xTaskGetTickCount());
	mask_bit = rand();
	memcpy(masking_key, &mask_bit, 4);
}

int cwebsocket_client_write_data(cwebsocket_client *websocket, const char *data, uint32_t payload_len, opcode code) {

	if((websocket->state & WEBSOCKET_STATE_OPEN) == 0) {
		cwebsocket_client_onerror(websocket, "websocket closed");
		return -1;
	}

	uint32_t header_length = 6 + (payload_len > 125 ? 2 : 0) + (payload_len > 0xffff ? 8 : 0);
	uint8_t masking_key[4];
	uint8_t header[header_length];
	int bytes_written;
	//char *framebuf = NULL;

	cwebsocket_client_create_masking_key(masking_key);
	header[0] = (code | 0x80);

	if(payload_len <= 125) {

		header[1] = (payload_len | 0x80);
		header[2] = masking_key[0];
		header[3] = masking_key[1];
		header[4] = masking_key[2];
		header[5] = masking_key[3];
	}
	else if(payload_len > 125 && payload_len <= 0xffff) { // 125 && 65535

		uint16_t len16 = htons(payload_len);
		header[1] = (126 | 0x80);
		memcpy(header+2, &len16, 2);
		header[4] = masking_key[0];
		header[5] = masking_key[1];
		header[6] = masking_key[2];
		header[7] = masking_key[3];
	}
	else if(payload_len > 0xffff && payload_len <= 0xffffffffffffffffLL) {  // 65535 && 18446744073709551615

		char len64[8] = htonl64(payload_len);
		header[1] = (127 | 0x80);
		memcpy(header+2, len64, 8);
		header[10] = masking_key[0];
		header[11] = masking_key[1];
		header[12] = masking_key[2];
		header[13] = masking_key[3];
	}
	else {
		cwebsocket_client_close(websocket, 1009, "frame too large");
		return -1;
	}

	int frame_length = header_length + payload_len;

	//print_msg("Write Data ...mFrameBufLength : %d\n", mFrameBufLength);

	if(frame_length > mFrameBufLength || mFrameBuf == NULL){
		print_msg("\nWebsockets : Realloc memory ...frame_length : %d, mFrameBufLength : %d\n", frame_length, mFrameBufLength);
       if(mFrameBuf)
         free(mFrameBuf);

       mFrameBuf = NULL;
	   mFrameBuf = malloc(frame_length);

	   if(mFrameBuf == NULL){
	      mFrameBufLength = 0;
	      cwebsocket_client_onerror(websocket, "Malloc mFrameBuf Fail!!");
	      return -1;
	   }
	   mFrameBufLength = frame_length;
	}
	//print_msg("write to websocket ...");
	memset(mFrameBuf, 0, mFrameBufLength);
	memcpy(mFrameBuf, header, header_length);
	memcpy(&mFrameBuf[header_length], data, payload_len);
	int i;
	for(i=0; i<payload_len; i++) {
	    mFrameBuf[header_length+i] ^= masking_key[i % 4] & 0xff;
	}
	bytes_written = cwebsocket_client_write(websocket, mFrameBuf, frame_length);

	if(bytes_written == -1) {
	   cwebsocket_client_onerror(websocket, "websocket write fail !!");
	   return -1;
	}
    return bytes_written;


	/*
	if (!(framebuf = malloc(frame_length))) {
		print_msg("framebuf malloc failed!!\n");
		return -1;
	}
	//char framebuf[frame_length];
	memset(framebuf, 0, frame_length);
	memcpy(framebuf, header, header_length);
	memcpy(&framebuf[header_length], data, payload_len);

	int i;
	for (i = 0; i < payload_len; i++) {
		framebuf[header_length+i] ^= masking_key[i % 4] & 0xff;
	}

	bytes_written = cwebsocket_client_write(websocket, framebuf, frame_length);

	if(bytes_written == -1) {
		cwebsocket_client_onerror(websocket, "websocket write fail !!");
		return -1;
	}

	if (framebuf)
		free(framebuf);

	return bytes_written;
	*/
}

void cwebsocket_client_close(cwebsocket_client *websocket, uint16_t code, const char *message) {
	//print_msg("Websocket close AA!!\n");
	if((websocket->state & WEBSOCKET_STATE_OPEN) == 0 || websocket->fd < 0) {

		if((websocket->state & WEBSOCKET_STATE_OPEN) == 0)
			print_msg("Webscoket close Err : websocket->state is not open ==> 0x%x!!\n", websocket->state);
		if((websocket->fd < 0))//modify by steven, 20150807
			print_msg("Webscoket close Err : websocket->fd is null !!\n");
		return;
	}

#ifdef ENABLE_THREADS
	webSocketLockMutex(&websocket->lock);
	websocket->state = WEBSOCKET_STATE_CLOSING;
	webSocketUnLockMutex(&websocket->lock);
#else
	//print_msg("Websocket close BB!!\n");
	websocket->state = WEBSOCKET_STATE_CLOSING;
#endif

	int code32 = 0;
	if(code > 0) {
		code = code ? htons(code) : htons(1005);
		int message_len = (message == NULL) ? 0 : strlen(message) + 2;
		uint8_t close_frame[message_len];
		close_frame[0] = code & 0xFF;
		close_frame[1] = (code >> 8);
		code32 = (close_frame[0] << 8) + (close_frame[1]);
		int i;
		for(i=0; i<message_len; i++) {
			close_frame[i+2] = message[i];
		}
		cwebsocket_client_send_control_frame(websocket, CLOSE, "CLOSE", close_frame, message_len);
	}
	else {
		cwebsocket_client_send_control_frame(websocket, CLOSE, "CLOSE", NULL, 0);
	}
	//print_msg("Websocket close CC!!\n");
#ifdef ENABLE_SSL
	if(websocket->ssl != NULL) {
     CyaSSL_shutdown(websocket->ssl);
	   CyaSSL_free(websocket->ssl);
    }
	if(websocket->sslctx != NULL) {
	    CyaSSL_CTX_free(websocket->sslctx);
	}
#else
	//print_msg("Websocket close DD!!\n");
	if(shutdown(websocket->fd, SHUT_WR) == -1) {
		print_msg("Webscoket close Err : shutdown fail !! !!\n");
		//syslog(LOG_ERR, "cwebsocket_client_close: unable to shutdown websocket: %s", strerror(errno));
	}
	char buf[1];
	while(read(websocket->fd, buf, 1) > 0) { buf[0] = '\0'; } //mark by steven, 20150828

	if(close(websocket->fd) == -1) {
		//syslog(LOG_ERR, "cwebsocket_client_close: error closing websocket: %s\n", strerror(errno));
		print_msg("Webscoket close Err : close fd fail !!\n");
		cwebsocket_client_onclose(websocket, 1011, "close fd fail !!");//strerror(errno));
	}
	websocket->fd = 0;
#endif
	//print_msg("Websocket close ee!!\n");

	cwebsocket_client_onclose(websocket, code32, message);

#ifdef ENABLE_THREADS
	webSocketLockMutex(&websocket->lock);
	websocket->state = WEBSOCKET_STATE_CLOSED;
	webSocketUnLockMutex(&websocket->lock);
	vTaskDelete(websocket->thread);
#else
	//print_msg("Websocket close FF!!\n");
	websocket->state = WEBSOCKET_STATE_CLOSED;
#endif
	websocket->state = 0;

	 //add by steven, 2015/09/02
     if(mFrameBuf){
        free(mFrameBuf);
        mFrameBuf = NULL;
     }


	if(websocket->flags & WEBSOCKET_FLAG_AUTORECONNECT) {
		print_msg("close step will autoconnect!!\n");
		cwebsocket_client_connect(websocket);
	}




	print_msg("Websocket close OK!!\n");
}

int cwebsocket_client_read(cwebsocket_client *websocket, void *buf, int len) {
#ifdef ENABLE_SSL
	return (websocket->flags & WEBSOCKET_FLAG_SSL) ?
			CyaSSL_read(websocket->ssl, buf, len) :
			read(websocket->fd, buf, len);
#else
	return read(websocket->fd, buf, len);
#endif
}

int cwebsocket_client_write(cwebsocket_client *websocket, void *buf, int len) {
#ifdef ENABLE_THREADS
	int bytes_written;
	webSocketLockMutex(&websocket->write_lock);
	#ifdef ENABLE_SSL
		bytes_written = (websocket->flags & WEBSOCKET_FLAG_SSL) ?
				CyaSSL_write(websocket->ssl, buf, len) :
				write(websocket->fd, buf, len);
	#else
		bytes_written = write(websocket->fd, buf, len);
	#endif
	webSocketUnLockMutex(&websocket->write_lock);
	return bytes_written;
#else
	#ifdef ENABLE_SSL
		return (websocket->flags & WEBSOCKET_FLAG_SSL) ?
				CyaSSL_write(websocket->ssl, buf, len) :
				write(websocket->fd, buf, len);
	#else
		return write(websocket->fd, buf, len);
	#endif
#endif
}

void cwebsocket_client_onopen(cwebsocket_client *websocket) {
	if(websocket->subprotocol != NULL && websocket->subprotocol->onopen != NULL) {
		websocket->subprotocol->onopen(websocket);
	}
}

void cwebsocket_client_onmessage(cwebsocket_client *websocket, cwebsocket_message *message) {
	if(websocket->subprotocol != NULL && websocket->subprotocol->onmessage != NULL) {
		websocket->subprotocol->onmessage(websocket, message);
	}
}

void cwebsocket_client_onclose(cwebsocket_client *websocket, int code, const char *message) {
	if(websocket->subprotocol != NULL && websocket->subprotocol->onclose != NULL) {
		websocket->subprotocol->onclose(websocket, code, message);
	}
}

void cwebsocket_client_onerror(cwebsocket_client *websocket, const char *error) {
	if(websocket->subprotocol != NULL && websocket->subprotocol->onerror != NULL) {
		websocket->subprotocol->onerror(websocket, error);
	}
}






