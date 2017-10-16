#ifndef __DOWNLOAD1_H__
#define __DOWNLOAD1_H__

#define FWUPGRAD_TASK_STOP      0
#define FWUPGRAD_TASK_RUNNING   1
#define MAX_UPGRD_RETYR_COUNT  100
#define FWUPFRAD_DEFAULT_STATUS -1

#define UPLOAD_PORT 2018
#define MAX_FW_SIZE 1300000

typedef enum
{
    FBFWMODE,
    SBFWMODE
} fwupgrade_mode_t;

typedef struct download_fw_info {
	util_socket_t dwnload_fw_fd;
	int firmware_size;
	int fwupgrade_flag;
	int fw_check_flag; //download ok=0, size error=-1, version error=-2, md5 error=-3
	int do_fwupgrade_task;
	char *fw_buf;
	SocketItem_t *psocket;
    fwupgrade_mode_t mode;
} download_fw_info_t;

int socket_download_fw_create(uint16_t *port, int fsize, SocketItem_t *psocket, fwupgrade_mode_t mode);
int get_download_fw_status(SocketItem_t *psocket);
int check_fwupgrad_task(void);
void release_fw_mem(void);
int fw_upgrade_proc_init(void);

#endif

