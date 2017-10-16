/**
 * @file
 * this is application header file for  creating socket to receve cmd from APP , include this file before use
 * @author Algorithm Dept Sonix.
 */
#ifndef __MF_SOCKET_CTRL_H__
#define __MF_SOCKET_CTRL_H__
#include "lwip/sockets.h"
#include "task.h"
#include "rec_common.h"

/**
* @brief max connection from APP
*/
#define MAX_CONNECT_NUM 5
#define DEFAULT_BUF_SZ 512	/**< default buffer size for buffer structure*/
#define SOKET_SERVER_PORT 8080
#define TCP_SOCK 	1
#define UDP_SOCK	2
#define RCV_BUFFER_SZ 256

typedef int util_socket_t;

/**
* @brief structure for buffer infomation
*/
typedef struct _buffer {
	int total_size;	/**< buffer size*/
	int use_size;	/**< current used size*/
	void *data;		/**< pointer for allocated buffer*/
} buffer_t;


enum {
	SOCKET_READ 	= 1 << 0,
	SOCKET_WRITE 	= 1 << 1,
};

/**
* @brief struct for socket infomation
*/
typedef struct _SocketItem {
	util_socket_t fd;				/**<  socket descriptor*/
	struct sockaddr_in addr;		/**<  addr infomation for connected socket*/
	char mac[20];				    /**<  Mac addr*/
	char flag;						/**<  set SOCKET_READ to check read event, set SOCKET_WRITE to check write event*/
	char rtsp_fname[LEN_FILENAME];	/**<  file name to playback throught rtsp*/
	buffer_t *precv_buf;			/**<  buffer to save receiving message*/
	TickType_t HeartTick;			/**<  last tick to recevie heartbeat command from APP*/
	struct _SocketItem *next;		/**<  point to next connected socket*/
	char fw_upgrade;           	 	/**<  set firmware upgrade*/
	char all_task_uninit;       	/**<  close all task, plese don't release fw buffer memory*/
	int status;                 	/**<  socket is handling process. 0:else 1:Download 2:Rtsp Playback*/
}SocketItem_t;


/**
* @brief struct for socket control infomation
*/
typedef struct _SocketInfo_t {
	SocketItem_t *socket_list;	/**<  list of connected socket*/
	int socket_count;			/**<  num of connected socket*/
	xTaskHandle task_heartbeat;	/**<  task to check heartbeat*/
	xTaskHandle task_process;	/**<  task to check connection and receive message*/

} SocketInfo_t;

/**
* @brief struct for download infomation
*/
typedef struct _DownloadInfo_t {
	util_socket_t fd;           /**<  socket download file descriptor*/
	int file_size;              /**<  download file sieze*/
	int do_dwnlodFont_task;     /**<  check if download_task running*/
	void *dwnlod_font_buf;		/**<  pointer to downloading font file buffer*/
} DownloadInfo_t;

enum SOCKET_DELET_TYPE {
	SOCKETDEL_EXCEPT_RTSP = 0,
	SOCKETDEL_HEARTBEAT,
	SOCKETDEL_FORCE,
};



buffer_t *buffer_new(void);
void buffer_free(buffer_t *pbuf);
int buffer_add(buffer_t *buf, const void *data, int size);
void buffer_clear(buffer_t *pbuf);
int socket_init(void);
void socket_uninit(void);
int socket_server_create(int port);
int socket_download_font_create(uint16_t *port, int size);
int socket_get_cur_connect_num(void);
void socket_update_tick(SocketItem_t *psocket);
int check_dwnlodFont_task(void);
void socket_delete_all(void);
#endif	//__MF_SOCKET_CTRL_H__
