#ifndef __UPLOAD1_H__
#define __UPLOAD1_H__

#define	MAX_CLIENT_NUM 5
#define	UPLOAD_CHUNK	(14600)
#define	UPLOAD_BG	0	//upload background
#define	UPLOAD_FG	1	//upload foreground
#define	MAX_IDLE_COUNT 250
#define TIME_OUT_SEND 5000 //5 sec

enum UPLOAD_TASK_STATUS {
	UPLOAD_UNSTART = 0,
	UPLOAD_START = 1,
	UPLOAD_IDLE = 2,
};

typedef struct STM {
	uint32_t	ipaddr;       // Network byte order
	char 		mac[20];	
	uint16_t	port;
	char		fn[256];
	int 		pos;
	int		newdata;    // 1: means new file need to send. 0: no file need to send.
	int		start;	     // 0:means stop send data, close socket.
	int sockfd;
	int accept_done; // 1 means upload_file_task has accepted the request from APP
	xTaskHandle upload_task;
	QueueHandle_t wait_port;
	uint32_t	clientid;	 //reserved
} STM;

int startfileupload(uint32_t ipaddr, char *mac, int upload_status, uint16_t *Port);
int add_file_upload(char *filename, uint32_t ipaddr, char *mac, int pos, int upload_status);
int stopfileupload(char *mac, int upload_status);
int getfileuploadstatus( int upload_status); //0:fileupload is not doing.1:fileupload is doing. -1:parameter is wrong.
STM *get_upload_info(int upload_status);
int is_current_upload_file(char *filename, int upload_status);
#endif
