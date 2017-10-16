#include <FreeRTOS.h>
#include <bsp.h>
#include <task.h>
#include <stdio.h>
#include <string.h>
#include <nonstdlib.h>
#include "lwip/sockets.h"
#include "lwip/ip_addr.h"
#include <libmid_fatfs/ff.h>
#include <libmid_fwupgrade/fwupgrade.h>
#include <libmid_nvram/snx_mid_nvram.h>
#include <generated/snx_sdk_conf.h>
#include "debug.h"
#ifndef CONFIG_APP_DRONE
#include "mcu.h"
#endif
#include "../daemon/json_cmd.h"
#include "../daemon/socket_ctrl.h"
#include "main_flow.h"
#include "online_fw_upgrade.h"
#include "utility.h"
#include "../errno.h"
#include <semphr.h>
#include <mcu_ctrl.h>

#define FW_PRINT(level, fmt, args...) print_q(level, "[fwupgr]: %s(%u): "fmt,__func__,__LINE__,##args)

static download_fw_info_t dwnlod_fw_info;
static xSemaphoreHandle online_fwupgrd_mutex = NULL;

int online_fwupgrd_semaphore_init(void)
{
    if( online_fwupgrd_mutex  == NULL ) {
        if( !(online_fwupgrd_mutex = xSemaphoreCreateMutex()) ) {
            FW_PRINT(SYS_ERR, "could not create mutex \n");
            return pdFAIL;
        }
    }

    return pdPASS;
}

int online_fwupgrd_semaphore_uninit(void)
{
    if( online_fwupgrd_mutex  != NULL ) {
        vSemaphoreDelete(online_fwupgrd_mutex);
        online_fwupgrd_mutex = NULL;
    }

    return pdPASS;
}

int get_download_fw_status(SocketItem_t *psocket)
{
    //FW_PRINT(SYS_DBG, "dwnlod_fw_info.fw_check_flag = %d\n",dwnlod_fw_info.fw_check_flag);
    if (dwnlod_fw_info.fw_check_flag == OK) {
        psocket->all_task_uninit = 1;
        dwnlod_fw_info.fwupgrade_flag = 1;
    }
    return dwnlod_fw_info.fw_check_flag;
}

int check_fwupgrad_task(void)
{
    //FW_PRINT(SYS_DBG, "dwnlod_fw_info.do_fwupgrade_task = %d\n",dwnlod_fw_info.do_fwupgrade_task);
    return dwnlod_fw_info.do_fwupgrade_task;
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
        FW_PRINT(SYS_ERR, "Failed to set KEEPALIVE\n");
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
            FW_PRINT(SYS_ERR, "tcp socket error\n");
        }

    }
    /* udp */
    else if (sock_type == UDP_SOCK) {
        if ((sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
            FW_PRINT(SYS_ERR, "udp socket error\n");
        }
    } else {
        FW_PRINT(SYS_ERR, "Invaild prototype %d\n", sock_type);
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
}

/*
** @fn online_fb_fw_upgrade
**
** @see mcu_ctrl.h for interface details.
**
** Pass downloaded binary data to FB flash initialisation process.
*/
static int online_fb_fw_upgrade(unsigned char *addr, unsigned int size)
{
	HexFile *hex = read_hex_file(addr);

	if (hex != NULL)
	{
        if(hex->valid == 1 && hex->end_of_file == 1)
        {
            flash_init(hex, SoftwareBootloader, false);
        }
        else
        {
            FW_PRINT(SYS_ERR, "Invalid Hex File %02x, %02x, %02x, %02x, %02x, %02x\n",
                addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);
        }

        free_hex_file(hex);

		return 0;
	}

	return -1;
}

static int online_fw_upgrade(uint8_t *fir_image, uint32_t fsize)
{
    int ret = 0;
    int retry_count = 0;
    fwburning_info_t  *sfwb_info;

    ret = fw_upgrade_precheck(fir_image, fsize);
    if (ret != 0) {
        switch (ret) {
        case FW_PRECHECK_ERR_MALLOC_FAIL:
            dwnlod_fw_info.fw_check_flag = MEMORY_IS_NOT_ENOUGH;
            break;
        case FW_PRECHECK_ERR_PLATFORM_DISMATCH:
            dwnlod_fw_info.fw_check_flag = UPGRADE_FW_AND_PLATFORM_DISMATCH;
            break;
        case FW_PRECHECK_ERR_VERSION:
            dwnlod_fw_info.fw_check_flag = UPGRADE_VERSION_ERR;
            break;
        case FW_PRECHECK_ERR_MD5:
            dwnlod_fw_info.fw_check_flag = UPGRADE_MD5_ERR;
            break;
        }
        goto out;
    }
    else
        dwnlod_fw_info.fw_check_flag = OK;

    while (1) {
        /* Over max retry times, leave the while loop and close online firmware upgrade task*/
        if (retry_count >= MAX_UPGRD_RETYR_COUNT) {
            FW_PRINT(SYS_DBG, " - trace\n");
            break;
        }

        FW_PRINT(SYS_DBG, " - retry: %u\n", retry_count);
        FW_PRINT(SYS_DBG, " - fwupgrade_flag: %u\n", dwnlod_fw_info.fwupgrade_flag);

        /* Waiting for App's response to start fw upgrade */
        if (dwnlod_fw_info.fwupgrade_flag == 1) {

            xSemaphoreTake(online_fwupgrd_mutex, portMAX_DELAY);

            FW_PRINT(SYS_DBG, " - check_fwupgrad_task() = %u\n", check_fwupgrad_task());

            //avoid already release_fw_mem(), cause system crash.
            if (check_fwupgrad_task() == FWUPGRAD_TASK_RUNNING) {

                all_task_uinit(TASK_KEEP_NO_KEEP);
#ifndef CONFIG_APP_DRONE
                mcu_set_err_flag(FIRMWARE_UPGRADE);
#endif
                fw_upgrade(fir_image, fsize);
                sfwb_info = get_fwburning_info();
                xSemaphoreGive(online_fwupgrd_mutex);
            } else {
                xSemaphoreGive(online_fwupgrd_mutex);
                break;
            }

            FW_PRINT(SYS_INFO, "wait upgrading ...\n");
            if ( xSemaphoreTake(sfwb_info->FwburningMutex, portMAX_DELAY ) == pdTRUE ) {
                FW_PRINT(SYS_INFO, "reboot !!!\n");
#ifndef CONFIG_APP_DRONE
                mcu_clear_err_flag(FIRMWARE_UPGRADE);
                mcu_set_err_flag(ALL_RESET);
#endif
            }
        } else {
            //avoid already release_fw_mem(), cause system crash
            if (check_fwupgrad_task() == FWUPGRAD_TASK_STOP) {
                break;
            }
        }

        vTaskDelay( 2000 / portTICK_PERIOD_MS ); //sleep 2s
        retry_count++;
    }

out:
    //vPortFree(dev_ver);
    //vPortFree(version);
    return ret;
}

/**
* @brief task to download firmware and firmware upgrade.
* @param pvParameters pointer to socket descriptor
*/
void task_download_fw_process( void *pvParameters )
{
    download_fw_info_t *pDwnlodInfo = pvParameters;
    util_socket_t new_fd;
    struct sockaddr_in addr;
    int len = sizeof(struct sockaddr_in);
    char *buf_p = NULL;
    int rcv_total_size = 0;
    char rcv_buf[RCV_BUFFER_SZ];

    if ((new_fd = accept(pDwnlodInfo->dwnload_fw_fd, (struct sockaddr *)&addr, (socklen_t *)&len)) < 0) {
        FW_PRINT(SYS_ERR, "accept error %d\n", new_fd);
        goto destroy_dwnlod_fw_fd;
    }

    FW_PRINT(SYS_DBG, "accept new socket %d (%x)\n", new_fd, (uint32_t)addr.sin_addr.s_addr);

    if (socket_set_keepalive(new_fd) < 0 )
        FW_PRINT(SYS_ERR, "fd = %d set keepalive fail\n", new_fd);

    memset(rcv_buf, 0x0, RCV_BUFFER_SZ);

    FW_PRINT(SYS_DBG, " - trace\n");

    if (!(pDwnlodInfo->fw_buf = (char *) pvPortMalloc(pDwnlodInfo->firmware_size, GFP_DMA, MODULE_APP))) {
        FW_PRINT(SYS_ERR, "pvPortMalloc failed (size = %d)!\n", pDwnlodInfo->firmware_size);
        goto free_buf;
    }

    pDwnlodInfo->psocket->fw_upgrade = 1; //for heartbeat check
    buf_p = pDwnlodInfo->fw_buf;

    pDwnlodInfo->fw_check_flag = OK;
    get_download_fw_status(pDwnlodInfo->psocket);

    FW_PRINT(SYS_DBG, "pDwnlodInfo->firmware_size = %d\n", pDwnlodInfo->firmware_size);

    while ((len = recv(new_fd, rcv_buf, RCV_BUFFER_SZ, 0)) > 0) {
        if((rcv_total_size + len) > pDwnlodInfo->firmware_size ){
            FW_PRINT(SYS_ERR, "recv size (%d) is lager than allocated size(%d)\n",
                (rcv_total_size + len), pDwnlodInfo->firmware_size );
            pDwnlodInfo->fw_check_flag = UPGRADE_RECEIVE_SIZE_ERR;
            goto free_buf;
        }

        memcpy(buf_p, rcv_buf, len);
        buf_p += len;

        rcv_total_size += len;

        if(rcv_total_size == pDwnlodInfo->firmware_size)
        {
            break;
        }
    }

    FW_PRINT(SYS_DBG, "rcv_total_size = %d\n", rcv_total_size);

    if (len < 0) {
        FW_PRINT(SYS_ERR, "Receive failed,ret=%d errno %d\n", len, errno);
        goto free_buf;
    }

    if (rcv_total_size == pDwnlodInfo->firmware_size) {
        FW_PRINT(SYS_INFO, "Download file successed,%d bytes\n", rcv_total_size);
    } else {
        FW_PRINT(SYS_ERR, "Receive file size %d != file size %d.\n", rcv_total_size, pDwnlodInfo->firmware_size);
        pDwnlodInfo->fw_check_flag = UPGRADE_RECEIVE_SIZE_ERR;
        goto free_buf;
    }

    FW_PRINT(SYS_DBG, ": pDwnlodInfo->mode = %u\n", pDwnlodInfo->mode);
    FW_PRINT(SYS_DBG, ": pDwnlodInfo->do_fwupgrade_task = %u\n", pDwnlodInfo->do_fwupgrade_task);

    switch(pDwnlodInfo->mode)
    {
    case SBFWMODE:
        FW_PRINT(SYS_INFO, ": Sonix Board Firmware Upgrade...\n");
        if(online_fw_upgrade((uint8_t *)pDwnlodInfo->fw_buf, pDwnlodInfo->firmware_size) != 0)
        {
            FW_PRINT(SYS_ERR, "online_fw_upgrade fail.\n");
            goto free_buf;
        }

        break;
    case FBFWMODE:
        FW_PRINT(SYS_INFO, ": Flight Board Firmware Upgrade...\n");
        if(online_fb_fw_upgrade((unsigned char *)pDwnlodInfo->fw_buf, pDwnlodInfo->firmware_size) != 0)
        {
            FW_PRINT(SYS_ERR, "online_fb_fw_upgrade fail.\n");
            goto free_buf;
        }

        break;

    default:
        FW_PRINT(SYS_ERR, ": invalid upgrade mode\n");
        goto free_buf;

        break;
    }

free_buf:
    if (pDwnlodInfo->fw_buf != NULL) {
        vPortFree(pDwnlodInfo->fw_buf);
        pDwnlodInfo->fw_buf = NULL;
    }
//destroy_new_fd:
    destory_socket(new_fd);
destroy_dwnlod_fw_fd:
    destory_socket(pDwnlodInfo->dwnload_fw_fd);
    pDwnlodInfo->do_fwupgrade_task = FWUPGRAD_TASK_STOP;
    pDwnlodInfo->psocket = NULL;
    vTaskDelete(NULL);
}

/**
* @brief interface function - create socket to download firmware from APP
* @param port pointer to return port for new socket
* @return return pdPASS if success
*/
int socket_download_fw_create(uint16_t *port, int fsize, SocketItem_t *psocket, fwupgrade_mode_t mode)
{
    util_socket_t dwnlod_fw_fd ;
    struct sockaddr_in addr;
    unsigned long len = sizeof(addr);

    FW_PRINT(SYS_DBG, "socket_download_fw_create - %d\n", fsize);

    //init dwnlod_fw_info
    memset(&dwnlod_fw_info, 0, sizeof(download_fw_info_t));
    dwnlod_fw_info.fw_check_flag = FWUPFRAD_DEFAULT_STATUS;
    dwnlod_fw_info.do_fwupgrade_task = FWUPGRAD_TASK_RUNNING;
    dwnlod_fw_info.psocket = psocket;
    online_fwupgrd_semaphore_init();
    //create socket
    if ((dwnlod_fw_fd = create_socket(TCP_SOCK)) < 0)
        goto fail;
    FW_PRINT(SYS_DBG, "dwnlod_fw_fd = %d\n", dwnlod_fw_fd);

    if (socket_set_keepalive(dwnlod_fw_fd) < 0 )
        FW_PRINT(SYS_ERR, "fd = %d set keepalive fail\n", dwnlod_fw_fd);


    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(0);

    if (bind(dwnlod_fw_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
        goto fail;

    //get port
    getsockname(dwnlod_fw_fd, (struct sockaddr *)&addr, &len);
    *port = ntohs(addr.sin_port);
    FW_PRINT(SYS_DBG, "port = %d\n", *port);

    if (listen(dwnlod_fw_fd, MAX_CONNECT_NUM) < 0)
        goto fail;

    //set download firmware information
    dwnlod_fw_info.dwnload_fw_fd = dwnlod_fw_fd;
    dwnlod_fw_info.firmware_size = fsize;
    dwnlod_fw_info.mode = mode;

    if (pdPASS != xTaskCreate(task_download_fw_process, "socket_download_fw", STACK_SIZE_8K, &dwnlod_fw_info,
                              PRIORITY_TASK_APP_TEST03, NULL)) {
        FW_PRINT(SYS_ERR, "Could not create task download\n");
        goto fail;
    }


    return pdPASS;

fail:
    destory_socket(dwnlod_fw_fd);
    dwnlod_fw_info.do_fwupgrade_task = FWUPGRAD_TASK_STOP;
    return pdFAIL;
}

void release_fw_mem(void)
{
    FW_PRINT(SYS_DBG, "release firmware memory...\n");
    xSemaphoreTake(online_fwupgrd_mutex, portMAX_DELAY);
    if (dwnlod_fw_info.fw_buf != NULL) {
        free(dwnlod_fw_info.fw_buf);
        dwnlod_fw_info.fw_buf = NULL;
        dwnlod_fw_info.do_fwupgrade_task = FWUPGRAD_TASK_STOP;
    }
    xSemaphoreGive(online_fwupgrd_mutex);
}

#if 0
int fwdownload_test_main(int argc, char *argv[])
{
    int port = 9110;
    char *server_addr = "172.21.2.189";

    dhcp_wait();
    if (0 != getfwdownload(inet_addr(server_addr), port, 1000))
        print_msg("Download file from %s:%d failed.\n", server_addr, port);

    print_msg(" %d Do firmware upgrade 0x%x .........\n", __LINE__, pFile);
    //Do firmware upgrade ...

    if (pFile != NULL)
        vPortFree(pFile);
    return 0;
}
#endif

