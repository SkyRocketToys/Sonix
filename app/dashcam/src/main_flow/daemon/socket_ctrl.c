/**
 * @file
 * this is application file for creating socket to receve cmd from APP
 * @author Algorithm Dept Sonix.
 */

#include <FreeRTOS.h>
#include <bsp.h>
#include <task.h>
#include <queue.h>
#include <nonstdlib.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <libmid_rtsp_server/rtsp_server.h>
#include <libmid_nvram/snx_mid_nvram.h>
#include "json_cmd.h"
#include "socket_ctrl.h"
#include "online_fw_upgrade.h"
#include "upload1.h"
#include "video_main.h"
#include <wifi/wifi_api.h>
#include "lwip/dhcps.h"

#include "../ArduPilot/dev_console_json.h"

#define GET_MAX(x,y) 		((x>=y)?(x):(y))
#define GET_MIM(x,y) 		((x>=y)?(y):(x))

#define SOCK_PRINT(level, fmt, args...) print_q(level, "[socket]%s: "fmt, __func__,##args)


SocketInfo_t SocketInfo;
void task_event_process( void *pvParameters );
void task_check_heartbeat( void *pvParameters );
void task_download_process( void *pvParameters );


static DownloadInfo_t dwnlod_info;
static int set_wifi_port_flag = 1;
/**
 * @brief interface function - create a new buffer
 * @return return pointer for created buffer
 */
buffer_t *buffer_new(void)
{
	buffer_t *buf = NULL;
	
	if (!(buf = pvPortMalloc(sizeof(buffer_t), GFP_KERNEL, MODULE_APP))) {
		SOCK_PRINT(SYS_ERR, "Create http buffer obj failed (size = %d)\n", sizeof(buffer_t));
		goto bail;
	}

	memset(buf, 0, sizeof(buffer_t));

	if (!(buf->data = pvPortMalloc(DEFAULT_BUF_SZ, GFP_KERNEL, MODULE_APP))) {
		SOCK_PRINT(SYS_ERR, "Create http buffer container failed (size = %d)\n", DEFAULT_BUF_SZ);
		goto bail;
	} else {
		buf->total_size = DEFAULT_BUF_SZ;
		buf->use_size = 0;
	}

	return buf;

bail:
	if (buf) {
		free(buf);
		buf = NULL;
	}

	return NULL;
}

/**
 * @brief interface function - delete buffer
 * @param pbuf pointer for buffer
 */
void buffer_free(buffer_t *pbuf)
{
	if (!pbuf)
		return;

	if (pbuf) {
		if (pbuf->data) {
			vPortFree(pbuf->data);
			pbuf->data = NULL;
		}

		vPortFree(pbuf);
		pbuf = NULL;
	}
}

/**
 * @brief interface function - add data to buffer
 * @param buf pointer for buffer
 * @param data data for add to buffer
 * @param size data size
 * @return return pdPASS if success
 */
int buffer_add(buffer_t *buf, const void *data, int size)
{
	int tsiz, usiz;
	char *tdat;
	//SOCK_PRINT("\n");
	if (!buf)
		return pdFAIL;

	tsiz = buf->total_size;		/* Total available size */
	usiz = buf->use_size;		/* Current used size */

	tdat = (char *)buf->data;

	/* check the buffer is big enough to store data */
	while (size >= (tsiz - usiz))
		tsiz *= 2;

	if (tsiz != buf->total_size) {
		if (!(buf->data = pvPortMalloc(tsiz, GFP_KERNEL, MODULE_APP))) {
			SOCK_PRINT(SYS_ERR, "Couldn't create new space (size = %d)\n", tsiz);
			buf->data = tdat;
			return pdFAIL;
		}
		/* clear new space */
		memset(buf->data, 0x0, tsiz);

		/* store data to new space */
		buf->total_size = tsiz;
		memcpy(buf->data, tdat, buf->use_size);

		// clear old space
		free(tdat);
		tdat = (char *)buf->data;
		buf->total_size = tsiz;
	}

	/* store new data append */
	tdat += buf->use_size;
	memcpy(tdat, data, size);
	buf->use_size += size;

	return pdPASS;
}

/**
 * @brief interface function - clear data in buffer
 * @param buf pointer for buffer
 */
void buffer_clear(buffer_t *pbuf)
{
	//memset(pbuf->data, 0, pbuf->total_size);
	pbuf->use_size = 0;
}

int check_dwnlodFont_task(void)
{
	//SOCK_PRINT(SYS_DBG, "dwnlod font task : %d\n",dwnlod_info.do_dwnlodFont_task);
	return dwnlod_info.do_dwnlodFont_task;
}


/**
 * @brief set socket non blocking
 * @param fd socket descriptor
 * @return success or not
 */
static int sokcet_set_nonblocking(util_socket_t fd)
{
	int flags;
	if ((flags = fcntl(fd, F_GETFL, 0)) < 0) {
		SOCK_PRINT(SYS_ERR, "fcntl(%d, F_GETFL)", fd);
		return pdFAIL;
	}

	if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
		SOCK_PRINT(SYS_ERR, "fcntl(%d, F_SETFL)", fd);
		return pdFAIL;
	}
	return pdPASS;
}

/**
 * @brief set socket keep alive
 * @param fd socket descriptor
 * @return success or not
 */
static int socket_set_keepalive(util_socket_t fd)
{
	int on = 1;
	if (setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, (void *)&on, sizeof(on)) < 0) {
		SOCK_PRINT(SYS_ERR, "Failed to set KEEPALIVE\n");
		return pdFAIL;
	}

	return pdPASS;
}

/**
 * @brief set socket re-useable
 * @param fd socket descriptor
 * @return success or not
 */
static int socket_set_reuse(util_socket_t fd)
{
	int on = 1;
	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (void *)&on, sizeof(on)) < 0) {
		SOCK_PRINT(SYS_ERR, "Failed to set REUSE\n");
		return pdFAIL;
	}
	return pdPASS;
}

/**
 * @brief create socket
 * @param sock_type socket type (tcp:udp)
 * @return socket descriptor
 */
static util_socket_t create_socket(int sock_type)
{
	util_socket_t sock = -1;

	/* tcp */
	if (sock_type == TCP_SOCK) {
		if ((sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
			SOCK_PRINT(SYS_ERR, "tcp socket error\n");
		}

	}
	/* udp */
	else if (sock_type == UDP_SOCK) {
		if ((sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
			SOCK_PRINT(SYS_ERR, "udp socket error\n");
		}
	} else {
		SOCK_PRINT(SYS_ERR, "Invaild prototype %d\n", sock_type);
	}

	return sock;
}

/**
 * @brief destory socket
 * @param sock socket descriptor
 * @return NULL
 */
static void destory_socket(util_socket_t sock)
{
	closesocket(sock);
	SOCK_PRINT(SYS_DBG, "close the socket fd = %d\n", sock);
}


static void set_socket_port_to_wifi_heandle(int port)
{

	if (WiFi_QueryAndSet(SET_PACKET_TYPE_PROTECT, (unsigned char *)&port, 0) != 1) {
		SOCK_PRINT(SYS_ERR, "port number exceed 5\n");
	} else {
		SOCK_PRINT(SYS_DBG, "set socket server port to wifi.\n");
		set_wifi_port_flag = 0;
	}
	return;
}


/**
 * @brief create a new socket
 * @param pSocketInfo pointer for socket control struct
 * @param fd socket descriptor
 * @param paddr pointer for address info
 * @param flag event type(SOCKET_READ, SOCKET_WRITE)
 * @return  return pdPASS if success
 */
static int socket_new(SocketInfo_t *pSocketInfo, util_socket_t fd, char *mac, struct sockaddr_in *paddr, char flag)
{
	SocketItem_t *pNewItem, *pPreItem;
	int i = 0;
	pNewItem = pvPortMalloc(sizeof(SocketItem_t), GFP_KERNEL, MODULE_APP);
	memset(pNewItem, 0, sizeof(SocketItem_t));
	if (!pNewItem) {
		SOCK_PRINT(SYS_ERR, "SocketInfo_t alloc fail (size = %d)\n", sizeof(SocketItem_t));
		return pdFAIL;
	}

	if (sokcet_set_nonblocking(fd) < 0)
		SOCK_PRINT(SYS_WARN, "fd = %d set nonblocking fail\n", fd);
	if (socket_set_keepalive(fd) < 0 )
		SOCK_PRINT(SYS_WARN, "fd = %d set keepalive fail\n", fd);

	pNewItem->fd = fd;
	pNewItem->flag = flag;
	pNewItem->HeartTick = xTaskGetTickCount();
	memcpy(&pNewItem->addr, paddr, sizeof(struct sockaddr_in));
	memcpy(&pNewItem->mac, mac, sizeof(pNewItem->mac));


	//add mutex
	if (pSocketInfo->socket_count == 0) {
		pSocketInfo->socket_list = pNewItem;
	} else {
		//find last item
		pPreItem = pSocketInfo->socket_list;
		//SOCK_PRINT(SYS_DBG, "socket original fd[%d] = %d\n", i,pPreItem->fd);
		i = 1;
		while (pPreItem->next) {
			pPreItem = pPreItem->next;
			//SOCK_PRINT(SYS_DBG, "socket original fd[%d] = %d\n", i,pPreItem->fd);
			i++;
		}
		pPreItem->next = pNewItem;
	}
	if (i != pSocketInfo->socket_count)
		SOCK_PRINT(SYS_WARN, "socket count incorrect(%d != %d)\n", i, pSocketInfo->socket_count);
	pSocketInfo->socket_count ++;
	//add mutex
	return pdPASS;

}

void release_download_font_mem(void)
{
	if (dwnlod_info.dwnlod_font_buf != NULL) {
		free(dwnlod_info.dwnlod_font_buf);
		dwnlod_info.dwnlod_font_buf = NULL;
	}
	memset(&dwnlod_info, 0, sizeof(DownloadInfo_t));
}


/**
 * @brief delete a socket
 * @param pSocketInfo pointer for socket control struct
 * @param pDelItem socket descriptor
 */
static void socket_delete(SocketInfo_t *pSocketInfo, SocketItem_t *pDelItem, int type)
{
	SocketItem_t *pItem, *pPreItem = NULL;

	pItem = pSocketInfo->socket_list;
	if (!pDelItem) {
		SOCK_PRINT(SYS_WARN, "pDelItem is null\n");
		return;
	}
	
	while (pItem) {
		if (pItem == pDelItem)
			break;
		pPreItem = pItem;
		pItem = pItem->next;
	}
	if (pItem) { //find item in list
		if (pPreItem)
			pPreItem->next = pItem->next;
		else {
			//delet first item in list
			if (pItem->next) {
				SOCK_PRINT(SYS_WARN, "socket_list still has other client socket\n");
				return;
			}
		}

		//add mutex

		//notice json to disconnet
		json_disconnect(pDelItem->rtsp_fname, pDelItem->mac, type);

		//check if fw_upgrad is alive
		if (pDelItem->fw_upgrade == 1) {
			if (pDelItem->all_task_uninit != 1) {
				release_fw_mem();
			}
		}

		//check if download font is alive
		if (dwnlod_info.do_dwnlodFont_task == 1) {
			release_download_font_mem();
			//SOCK_PRINT(SYS_DBG, "font_task : %d\n", dwnlod_info.do_dwnlodFont_task);
		}

#if LIMIT_PLAYBACK_AND_DOWNLOAD_CONNECTION
		//check playback and download connection
		if (pDelItem->status != 0) {
			del_dwnlod_and_pb_conn_num();
			pDelItem->status = 0;
		}
		delete_conn_from_pb_and_dwnlod(pDelItem->mac);
#endif

		destory_socket(pDelItem->fd);
		buffer_free(pDelItem->precv_buf);
		if (pDelItem) {
			vPortFree(pDelItem);
			pDelItem = NULL;
		}
		pSocketInfo->socket_count --;
		//add mutex
	} else {
		SOCK_PRINT(SYS_ERR, "fd = %d not found\n", pDelItem->fd);
		return;
	}

	SOCK_PRINT(SYS_DBG, "exist socket = %d\n", pSocketInfo->socket_count);
	pItem = pSocketInfo->socket_list;

	while (pItem) {
		SOCK_PRINT(SYS_DBG, "\t fd = %d(%s)\n", pItem->fd, pItem->mac);
		pItem = pItem->next;
	}
}

/**
 * @brief delete all socket
 * @param pSocketInfo pointer for socket control struct
 */
void socket_delete_all(void)
{
	SOCK_PRINT(SYS_DBG, "======================\n");
	SOCK_PRINT(SYS_DBG, "delete all socket\n");
	SOCK_PRINT(SYS_DBG, "======================\n");
	
	SocketItem_t *pItem = NULL, *pDelItem = NULL;
	SocketInfo_t *pSocketInfo = &SocketInfo;

	pItem = pSocketInfo->socket_list->next;
	pSocketInfo->socket_list->next = NULL; 

	while (pItem) {
		pDelItem = pItem;
		pItem = pItem->next;
		
		SOCK_PRINT(SYS_DBG, "delete socket mac addr %s\n", pDelItem->mac);
		//notice json to disconnet
		json_disconnect(pDelItem->rtsp_fname, pDelItem->mac, SOCKETDEL_FORCE);

		//check if fw_upgrad is alive
		if (pDelItem->fw_upgrade == 1) {
			if (pDelItem->all_task_uninit != 1) {
				release_fw_mem();
			}
		}

		//check if download font is alive
		if (dwnlod_info.do_dwnlodFont_task == 1) {
			release_download_font_mem();
			//SOCK_PRINT(SYS_DBG, "font_task : %d\n", dwnlod_info.do_dwnlodFont_task);
		}

#if LIMIT_PLAYBACK_AND_DOWNLOAD_CONNECTION
		//check playback and download connection
		if (pDelItem->status != 0) {
			del_dwnlod_and_pb_conn_num();
			pDelItem->status = 0;
		}
		delete_conn_from_pb_and_dwnlod(pDelItem->mac);
#endif

		destory_socket(pDelItem->fd);
		buffer_free(pDelItem->precv_buf);
		if (pDelItem) {
			vPortFree(pDelItem);
			pDelItem = NULL;
		}
		pSocketInfo->socket_count --;

	}
}

/**
 * @brief accept a new connect
 */
static void socket_accept(void)
{
	util_socket_t new_fd;
	SocketItem_t *pSocket = NULL;
	struct sockaddr_in addr;
	int len = sizeof(struct sockaddr_in);
	unsigned char *pMac = NULL;
	char new_mac[20] = {0};

	//set the port to wifi for special handling data in this port.
	if (set_wifi_port_flag)
		set_socket_port_to_wifi_heandle(SOKET_SERVER_PORT);

	if ((new_fd = accept(SocketInfo.socket_list->fd, (struct sockaddr *)&addr, (socklen_t *)&len)) < 0) {
		SOCK_PRINT(SYS_ERR, "accept error %d\n", new_fd);
		return;
	}
	
	if (SocketInfo.socket_count >= (MAX_CONNECT_NUM + 1)) {
		//connection is full
		SOCK_PRINT(SYS_WARN, "over max connect num(>%d)\n", MAX_CONNECT_NUM);
		destory_socket(new_fd);
		return;
	}

	pMac = get_mac_by_ip((uint32_t)addr.sin_addr.s_addr);
	if(pMac == NULL)
	{
		SOCK_PRINT(SYS_DBG, "================================================\n");
		SOCK_PRINT(SYS_DBG, "================================================\n");
		SOCK_PRINT(SYS_DBG, "================================================\n");
		SOCK_PRINT(SYS_DBG, "				get MAC Failed\n");
		SOCK_PRINT(SYS_DBG, "================================================\n");
		SOCK_PRINT(SYS_DBG, "================================================\n");
	}
	sprintf(new_mac, "%02X-%02X-%02X-%02X-%02X-%02X", pMac[0], pMac[1], pMac[2], pMac[3], pMac[4], pMac[5]);
	SOCK_PRINT(SYS_DBG, "accept new MAC %s, socket fd=%d (%s, port = %x)\n",
	                   new_mac,
					   new_fd,
	                   inet_ntoa(addr.sin_addr.s_addr),
	                   addr.sin_port);


	if (SocketInfo.socket_list)
		pSocket  = SocketInfo.socket_list->next;

	//find exist socket which has the same MAC addr
	while (pSocket) {
		if (strcmp(new_mac, pSocket->mac) == 0)
			break;
		pSocket = pSocket->next;
	}

	if (pSocket) {
		SOCK_PRINT(SYS_WARN, "find another socket has the same Mac(%s)\n", new_mac);
		SOCK_PRINT(SYS_WARN, "delete older socket(fd = %d)\n", pSocket->fd);
		//don't check to disconnect
		socket_delete(&SocketInfo , pSocket, SOCKETDEL_EXCEPT_RTSP);
	} else {
		//clear old upload task
		stopfileupload(new_mac, UPLOAD_BG); //UPLOAD_BG
		stopfileupload(new_mac, UPLOAD_FG); //UPLOAD_FG
	}
	socket_new(&SocketInfo, new_fd, new_mac, &addr, SOCKET_READ | SOCKET_WRITE);

}

static void write_data(SocketItem_t *psocket, buffer_t *resp_buf)
{
	int rc;
	int max_transfer = 1024;
	int used_size = resp_buf->use_size;
	int offset = 0;
	system_date_t  time;
#if JSON_DUMP_TO_FILE
        json_dump_sent_data(resp_buf->data, resp_buf->use_size);
#endif
	while (used_size) {
		if ((rc = send(psocket->fd, resp_buf->data + offset, GET_MIM(used_size, max_transfer), 0)) < 0) {
			SOCK_PRINT(SYS_ERR, "Socket Send error!(rc=%d)\n", rc);
			break;
		}
		offset += GET_MIM(used_size, max_transfer);
		used_size -= GET_MIM(used_size, max_transfer);
	}

	get_date(&time);
	SOCK_PRINT(SYS_DBG, "fd = %d, size = %d, cur_time = %04d_%02d_%02d_%02d_%02d_%02d\n", psocket->fd, resp_buf->use_size, time.year, time.month, time.day, time.hour, time.minute, time.second);
	*(char *)(resp_buf->data + resp_buf->use_size) = 0;
	if (resp_buf->use_size < 100)
		SOCK_PRINT(SYS_DBG, "%s\n", resp_buf->data);
}

static void read_data(SocketItem_t *psocket)
{
	char rcv_buffer[RCV_BUFFER_SZ];
	buffer_t *resp_buf = NULL;
	int len = 0;
	memset(rcv_buffer, 0x0, RCV_BUFFER_SZ);
	system_date_t  time;

	while ((len = recv(psocket->fd, rcv_buffer, RCV_BUFFER_SZ, 0)) > 0) {
		get_date(&time);
		SOCK_PRINT(SYS_DBG, "cmd len = %d, cur_time = %04d_%02d_%02d_%02d_%02d_%02d\n", len, time.year, time.month, time.day, time.hour, time.minute, time.second);
		if (!psocket->precv_buf) {
			psocket->precv_buf = buffer_new();
		}
		buffer_add(psocket->precv_buf, rcv_buffer, len);
		//SOCK_PRINT(SYS_DBG, "copy finish...\n");
	}

	if (psocket->precv_buf->use_size < psocket->precv_buf->total_size)
		*(char *)(psocket->precv_buf->data + psocket->precv_buf->use_size) = 0;     //set string end

        if (len) {
#if JSON_DUMP_TO_FILE
                json_dump_received_data(psocket->precv_buf->data, psocket->precv_buf->use_size);
#endif
		//parse cmd
		void *ptr = psocket->precv_buf->data;
		int offset = 0;
		buffer_t tmp_buf;
#if 0
		if (psocket->precv_buf->use_size > 5 && (strncmp(psocket->precv_buf->data, "SMARP", 5) == 0)) {
			socket_update_tick(psocket);
		} else
#endif
			if (psocket->precv_buf->use_size) {
				while ( (ptr = strchr(ptr, '{'))) {
					offset = psocket->precv_buf->data - ptr;
					tmp_buf.data = ptr;
					tmp_buf.use_size = psocket->precv_buf->use_size - offset;
					tmp_buf.total_size = psocket->precv_buf->total_size - offset;

					resp_buf = parse_json_cmd(&tmp_buf, (void *)psocket);
					if (resp_buf) {
						write_data(psocket, resp_buf);
						buffer_clear(resp_buf);
					}
					ptr += 1;
				}
				set_wifi_param();
			} else {
				SOCK_PRINT(SYS_DBG, "buf size = 0\n");
			}

		//clear buffer
		buffer_clear(psocket->precv_buf);
	} else {
		SOCK_PRINT(SYS_DBG, "close fd %d\n", psocket->fd);
		socket_delete(&SocketInfo, psocket, SOCKETDEL_EXCEPT_RTSP);
		return;
	}
}

/**
* @brief interface function - socket control initialization
* @return return pdPASS if success
*/
int socket_init(void)
{
	//set rtsp max connections

	SOCK_PRINT(SYS_DBG, " - trace\n");

	set_max_connections(MAX_CONNECT_NUM);
	memset(&SocketInfo, 0, sizeof(SocketInfo));
	json_init();
#if JSON_DUMP_TO_FILE
        json_dump_init();
#endif

	SOCK_PRINT(SYS_DBG, " - trace\n");

	return pdPASS;
}

/**
* @brief interface function - socket control uninitialization
*/
void socket_uninit(void)
{
	SocketItem_t *pItem, *pDeleteItem;
	//remove client socket
	if (SocketInfo.task_heartbeat) {
		vTaskDelete(SocketInfo.task_heartbeat);
		SocketInfo.task_heartbeat = NULL;
	}
	if (SocketInfo.task_process) {
		vTaskDelete(SocketInfo.task_process);
		SocketInfo.task_process = NULL;
	}

	if (SocketInfo.socket_list) {
		pItem = SocketInfo.socket_list->next;
		while (pItem) {
			pDeleteItem = pItem;
			pItem = pDeleteItem->next;
			SOCK_PRINT(SYS_DBG, "del %d fd\n", pDeleteItem->fd);
			socket_delete(&SocketInfo, pDeleteItem, SOCKETDEL_FORCE);
		}
		//remove server socket
		SOCK_PRINT(SYS_DBG, "del %d fd\n", SocketInfo.socket_list->fd);
		socket_delete(&SocketInfo, SocketInfo.socket_list, SOCKETDEL_FORCE);
		SocketInfo.socket_list = NULL;
	}

	SocketInfo.socket_count = 0;
#if JSON_DUMP_TO_FILE
        json_dump_uninit();
#endif
	json_uninit();
}

/**
* @brief interface function - create server socket
* @param port port num for server socket
* @return return pdPASS if success
*/
int socket_server_create(int port)
{
	util_socket_t fd;
	struct sockaddr_in addr;
	unsigned char *pMac = NULL;
	char srv_mac[20] = {0};

	SOCK_PRINT(SYS_DBG, " - trace\n");

	if ((fd = create_socket(TCP_SOCK)) < 0) {
                print_msg_queue("sss: create_socket failed (port=%d)\n", port);
		goto fail;
        }
	SOCK_PRINT(SYS_DBG, " - trace\n");

	SOCK_PRINT(SYS_DBG, "fd = %d\n", fd);

	if (sokcet_set_nonblocking(fd) < 0) {
                print_msg_queue("sss: set_nonblocking failed (port=%d)\n", port);
		goto fail1;
        }

	if (socket_set_keepalive(fd) < 0 ) {
                print_msg_queue("sss: set_keepalive failed (port=%d)\n" , port);
		goto fail1;
        }

	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(port);

	if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
                print_msg_queue("sss: bind failed (port=%u)\n", port);
		goto fail1;
        }

	SOCK_PRINT(SYS_DBG, "max connect = %d\n", MAX_CONNECT_NUM);
	if (listen(fd, MAX_CONNECT_NUM) < 0) {
                print_msg_queue("sss: listen failed (port=%d) (sock=%u\n", port, fd);
		goto fail1;
	}

	pMac = wlan_get_get_mac_addr();
	sprintf(srv_mac, "%02X-%02X-%02X-%02X-%02X-%02X", pMac[0], pMac[1], pMac[2], pMac[3], pMac[4], pMac[5]);
	SOCK_PRINT(SYS_DBG, "socket server mac %s\n", srv_mac);	

	socket_new(&SocketInfo, fd, srv_mac, &addr, SOCKET_READ);

	SOCK_PRINT(SYS_DBG, " - trace\n");

	if (pdPASS != xTaskCreate(task_event_process, "socket", STACK_SIZE_4K, &SocketInfo,
	                          PRIORITY_TASK_APP_CMDDAEMON, &SocketInfo.task_process)) {
		SOCK_PRINT(SYS_ERR, "Could not create task socket event\n");
		goto fail1;
	}

	if (pdPASS != xTaskCreate(task_check_heartbeat, "socket_heart", STACK_SIZE_2K, &SocketInfo,
	                          PRIORITY_TASK_APP_HEARTBEAT, &SocketInfo.task_heartbeat)) {
		SOCK_PRINT(SYS_ERR, "Could not create task socket event\n");
		goto fail2;
	}

	return pdPASS;
fail2:
	vTaskDelete(SocketInfo.task_process);
fail1:
	destory_socket(fd);
fail:
        print_msg_queue("########### socket_server_create failed\n");
	return pdFAIL;

}


static int set_sokcet(SocketInfo_t *pSocketInfo, fd_set *rdfds, fd_set *wrfds)
{
	int maxfds = 0;
	SocketItem_t *pItem = pSocketInfo->socket_list;

	while (pItem) {

		if (pItem->flag & SOCKET_READ) {
			FD_SET(pItem->fd, rdfds);
			maxfds = GET_MAX(maxfds, pItem->fd);
		}

		if (pItem->flag & SOCKET_WRITE) {
			FD_SET(pItem->fd, wrfds);
			maxfds = GET_MAX(maxfds, pItem->fd);
		}
		pItem = pItem->next;
	}
	//SOCK_PRINT(SYS_DBG, "fds = %x %x %d\n", rdfds->fd_bits[0], wrfds->fd_bits[0], maxfds);
	return maxfds;
}

/**
* @brief task for socket event process
* @param pvParameters pointer for socket control structure
*/
void task_event_process( void *pvParameters )
{
	int res = 0, max_fds = 0, fd;
	short ev_trigger = 0;
	fd_set rdfds, wrfds;
	struct timeval tv;
	SocketInfo_t *pSocketInfo = (SocketInfo_t *)pvParameters;
	SocketItem_t *pItem;

	vTaskDelay(1000 / portTICK_RATE_MS );

	while (1) {

		FD_ZERO(&rdfds);
		FD_ZERO(&wrfds);

		max_fds = set_sokcet(pSocketInfo, &rdfds, &wrfds);
		max_fds++;

		memset(&tv, 0x0, sizeof(struct timeval));
		tv.tv_sec = 0;
		tv.tv_usec = 50000;
		//res = select(max_fds, &rdfds, &wrfds, NULL, &tv);
		res = select(max_fds, &rdfds, &wrfds, NULL, NULL);

		if (res < 0) {
			SOCK_PRINT(SYS_ERR, "select error\n");
			continue;
		}
		//else if (res == 0) {
		//	SOCK_PRINT(SYS_DBG, "no sock io ready\n");
		//	continue;
		//}

		//SOCK_PRINT(SYS_DBG, "max_fds = %d......\n", max_fds);
		pItem = pSocketInfo->socket_list;
		while (pItem) {
			fd = pItem->fd;
			//SOCK_PRINT(SYS_DBG, "check fd = %d......\n", fd);
            
			ev_trigger = 0;

//			SOCK_PRINT(SYS_DBG, ": fd = %u; bits = %02x\n", fd, rdfds.fd_bits[fd / 8]);

			if (FD_ISSET(fd, &rdfds)) {
				ev_trigger |= SOCKET_READ;
			}

			if (FD_ISSET(fd, &wrfds)) {
				ev_trigger |= SOCKET_WRITE;
			}

			if (ev_trigger != 0) {
				if (ev_trigger & SOCKET_READ) {
					if (fd == pSocketInfo->socket_list->fd) {
						socket_accept();
					} else {
						
						//SOCK_PRINT(SYS_DBG, "fd = %d, tig = %d\n", fd, ev_trigger);
						read_data(pItem);
					}
				}
			}
			pItem = pItem->next;

		}
		vTaskDelay(100 / portTICK_RATE_MS );
	}

	vTaskDelete(NULL);

}

/**
* @brief interface function - create socket to download fond data from APP
* @param port pointer to return port for new socket
* @return return pdPASS if success
*/
int socket_download_font_create(uint16_t *port, int size)
{
	struct sockaddr_in addr;
	unsigned long len = sizeof(addr);

	memset(&dwnlod_info, 0, sizeof(DownloadInfo_t));
	dwnlod_info.file_size = size;
	dwnlod_info.do_dwnlodFont_task = 1;

	if ((dwnlod_info.fd = create_socket(TCP_SOCK)) < 0) {
		SOCK_PRINT(SYS_ERR, "create socket failed.\n");
		goto fail;
	}
	SOCK_PRINT(SYS_DBG, "download_fd = %d\n", dwnlod_info.fd);

	//if (sokcet_set_nonblocking(dwnlod_info.fd) < 0)
	//	SOCK_PRINT(SYS_DBG, "fd = %d set nonblocking fail\n", dwnlod_info.fd);

	if (socket_set_keepalive(dwnlod_info.fd) < 0 )
		SOCK_PRINT(SYS_WARN, "fd = %d set keepalive fail\n", dwnlod_info.fd);

	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(0);

	if (bind(dwnlod_info.fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		SOCK_PRINT(SYS_ERR, "bind socket failed.\n");
		goto fail;
	}

	//get port
	getsockname(dwnlod_info.fd, (struct sockaddr *)&addr, &len);
	*port = ntohs(addr.sin_port);
	SOCK_PRINT(SYS_DBG, "port = %d\n", *port);

	if (listen(dwnlod_info.fd, MAX_CONNECT_NUM) < 0) {
		SOCK_PRINT(SYS_ERR, "listen failed.\n");
		goto fail;
	}

	if (pdPASS != xTaskCreate(task_download_process, "socket_download", STACK_SIZE_4K, &dwnlod_info,
	                          PRIORITY_TASK_APP_TEST03, NULL)) {
		SOCK_PRINT(SYS_ERR, "Could not create task download\n");
		goto fail;
	}

	return pdPASS;

fail:
	destory_socket(dwnlod_info.fd);
	dwnlod_info.do_dwnlodFont_task = 0;
	return pdFAIL;

}

/*
int wait_fd(util_socket_t fd)
{
	int res;
	fd_set rdfds;
	FD_ZERO(&rdfds);
	FD_SET(fd, &rdfds);
	res = select(fd+1, &rdfds, NULL, NULL, NULL);
	if (res < 0) {
		SOCK_PRINT(SYS_ERR, "select error\n");
		return pdFAIL;
	}
	else
		return pdPASS;

}*/

/**
* @brief task to download font data and write to nvram
* @param pvParameters pointer to socket descriptor
*/
void task_download_process( void *pvParameters )
{
	DownloadInfo_t *download_info = pvParameters;
	util_socket_t new_fd;
	struct sockaddr_in addr;
	void *buf = NULL;
	void *buf_p = NULL;
	int rcv_total_size = 0;
	int len = sizeof(struct sockaddr_in);
	char rcv_buf[RCV_BUFFER_SZ];
	static char *nvram_pack = "app_osd_ctrl_font";
	static char *font_config_16 = "FontFile_16.bin";
	static char *font_config_48 = "FontFile_48.bin";
	int nvram_err = 0;
	void *pdata;
	nvram_data_info_t nvram;

#define HDR_LEN 14
	if ((new_fd = accept(download_info->fd, (struct sockaddr *)&addr, (socklen_t *)&len)) < 0) {
		SOCK_PRINT(SYS_ERR, "accept error %d\n", new_fd);
		goto destroy_download_fd;
	}
	SOCK_PRINT(SYS_DBG, "accept new socket %d (%x)\n", new_fd, (uint32_t)addr.sin_addr.s_addr);

	//if (sokcet_set_nonblocking(new_fd) < 0)
	//	SOCK_PRINT(SYS_DBG, "fd = %d set nonblocking fail\n", new_fd);
	if (socket_set_keepalive(new_fd) < 0 )
		SOCK_PRINT(SYS_WARN, "fd = %d set keepalive fail\n", new_fd);

	memset(rcv_buf, 0x0, RCV_BUFFER_SZ);

	if (!(buf = (char *) pvPortMalloc(download_info->file_size, GFP_DMA, MODULE_APP))) {
		SOCK_PRINT(SYS_ERR, "buf allocate fail (size = %d)\n", download_info->file_size);
		goto destroy_new_fd;
	}
	download_info->dwnlod_font_buf = buf;
	buf_p = buf;

	//SOCK_PRINT(SYS_DBG, "recv data\n");
	while ((len = recv(new_fd, rcv_buf, RCV_BUFFER_SZ, 0)) > 0) {
		if((rcv_total_size + len) > download_info->file_size ){
			SOCK_PRINT(SYS_ERR, "recv size (%d) is lager than allocated size(%d)\n", (rcv_total_size + len), download_info->file_size );
			goto end;
		}
		memcpy(buf_p, rcv_buf, len);
		buf_p += len;
		rcv_total_size += len;
	}

	SOCK_PRINT(SYS_DBG, "recevie size = %d\n", rcv_total_size);

	//save font 16
	pdata = buf;
	nvram.data = pdata + HDR_LEN;
	nvram.data_len = (*(char *)(pdata + 13) << 24) | (*(char *)(pdata + 2) << 16) | (*(char *)(pdata + 11) << 8) | (*(char *)(pdata + 10));
	nvram.data_type = NVRAM_DT_BIN_RAW;
	if (((*(char *)(pdata + 1)) == 16) && (nvram.data_len + HDR_LEN) <= rcv_total_size) {
		SOCK_PRINT(SYS_DBG, "save font 16 size = %d\n", nvram.data_len);
		if ((nvram_err = snx_nvram_set_immediately(nvram_pack, font_config_16, &nvram))) {
			SOCK_PRINT(SYS_ERR, "Save osd string to Flash Fail(%d)\n", nvram_err);
		}
	}

	//save font 48
	pdata = buf + HDR_LEN + nvram.data_len;
	nvram.data = pdata + HDR_LEN;
	nvram.data_len = (*(char *)(pdata + 13) << 24) | (*(char *)(pdata + 2) << 16) | (*(char *)(pdata + 11) << 8) | (*(char *)(pdata + 10));
	nvram.data_type = NVRAM_DT_BIN_RAW;
	if ((*(char *)(pdata + 1)) == 48  && ((pdata - buf)  + nvram.data_len + HDR_LEN) <= rcv_total_size) {
		SOCK_PRINT(SYS_DBG, "save font 48 size = %d\n", nvram.data_len);
		if ((nvram_err = snx_nvram_set_immediately(nvram_pack, font_config_48, &nvram))) {
			SOCK_PRINT(SYS_ERR, "Save osd string to Flash Fail(%d)\n", nvram_err);
		}
	}

	// osd enable
	//mf_osd_enable();
	if (chk_preview_use_isp0dup() == 1) {
		osd_preview_is_dup1_setting();
		SOCK_PRINT(SYS_DBG, "osd:chk_preview_use_isp0dup, socket_fd=%d\n", new_fd);
	} else {
		osd_preview_is_isp1_setting();
		SOCK_PRINT(SYS_DBG, "osd:CheckPreviewUseIsp1, socket_fd=%d\n", new_fd);
	}

end:

	if (buf != NULL) {
		free(buf);
		buf = NULL;
		download_info->dwnlod_font_buf = NULL;
	}
destroy_new_fd:
	destory_socket(new_fd);
destroy_download_fd:
	destory_socket(download_info->fd);
	download_info->do_dwnlodFont_task = 0;
	vTaskDelete(NULL);
}

/**
* @brief task to check heartbeat
* @param pvParameters pointer for socket control structure
*/
void task_check_heartbeat( void *pvParameters )
{
	SocketInfo_t *pSocketInfo = (SocketInfo_t *)pvParameters;
	SocketItem_t *pItem;
	TickType_t CurTick, TickInterval;



	while (1) {
		pItem = pSocketInfo->socket_list;

		while (pItem->next) {
			pItem = pItem->next;
			CurTick = xTaskGetTickCount();;
			if (CurTick >= pItem->HeartTick) {	//check overflow
				TickInterval = CurTick - pItem->HeartTick;
				if (TickInterval > (60000 / portTICK_RATE_MS) ) {	//over 1 min
					SOCK_PRINT(SYS_INFO, "delete socket(fd = %d)\n", pItem->fd);
					socket_delete(pSocketInfo, pItem, SOCKETDEL_HEARTBEAT);
				} else {
					SOCK_PRINT(SYS_DBG, "fd = %d interval = %d\n", pItem->fd, TickInterval);
				}
			} else
				pItem->HeartTick = CurTick;

		}
		vTaskDelay(15000 / portTICK_RATE_MS);	//delay 15 seconds
	}
	vTaskDelete(NULL);


}

/**
* @brief interface function - get current connection num
* @return return connection num
*/
int socket_get_cur_connect_num(void)
{
	if (SocketInfo.socket_count)
		return SocketInfo.socket_count - 1;
	else
		return 0;
}


/**
* @brief interface function - update tick count
* @param psocket pointer for socket structure
*/
void socket_update_tick(SocketItem_t *psocket)
{
	psocket->HeartTick = xTaskGetTickCount();
}

