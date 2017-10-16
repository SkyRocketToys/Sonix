/*
  support transmitter OTA firmware update
  Andrew Tridgell, May 2017
 */

#include <FreeRTOS.h>
#include <bsp.h>
#include <task.h>
#include <nonstdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "socket_ctrl.h"
#include <generated/snx_sdk_conf.h>
#include "mavlink_wifi.h"
#include "dev_console.h"
#include "tx_upload.h"
#include "talloc.h"

// firmware image
static uint8_t *fw_image;
static uint16_t fw_len;
static uint32_t acked_to;
static long long last_send_time;
static long long upload_start;

/*
  start a TX upgrade
 */
void tx_upgrade(const uint8_t *image, unsigned size)
{
    if (fw_image && fw_image != image) {
        talloc_free(fw_image);
    }
    fw_image = talloc_memdup(NULL, image, size);
    fw_len = size;
    acked_to = 0;
    set_upload_progress(0);
    
    upload_start = get_sys_seconds_boot();
    console_printf("Started TX upgrade with image size %u\n", size);
}

/*
  task for uploading transmitter firmware over TCP
 */
void tx_fw_upload_task_process(void *pvParameters)
{
    int tcp_sock;

    struct sockaddr_in addr;

    if ((tcp_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
        goto end;
    }

    memset(&addr, 0x0, sizeof(addr));

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(TX_UPLOAD_PORT);

    if (bind(tcp_sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        goto end;
    }

    if (listen(tcp_sock, 2) < 0) {
        goto end;
    }

    while (1)
    {
        mdelay(100);

        int len = sizeof(struct sockaddr_in);
        int fd = accept(tcp_sock, (struct sockaddr *)&addr, (socklen_t *)&len);
        if (fd == -1) {
            continue;
        }

        // attempt to acquire mutex early so we don't allocate memory
        // if someone else is in the middle of an upgrade:
        if (!any_fw_upgrade_mutex ||
            xSemaphoreTake(any_fw_upgrade_mutex, 100) != pdTRUE) {
            console_printf("Failed to acquire any_fw_upgrade_mutex\n");
            close(fd);
            continue;
        }

        console_printf("Receiving new TX firmware\n");

        if (fw_image) {
            talloc_free(fw_image);
        }
        fw_len = 0;
        
        if (!(fw_image = talloc_zero_size(NULL, MAX_TX_FW_SIZE))) {
            console_printf("Failed to allocate %u bytes\n", MAX_TX_FW_SIZE);
            close(fd);
            xSemaphoreGive(any_fw_upgrade_mutex);
            continue;
        }
        
        uint32_t fwlen = 0;
        while (1) {
            unsigned n;
            n = read(fd, fw_image+fwlen, MAX_TX_FW_SIZE-fwlen);
            if (n <= 0) {
                break;
            }
            fwlen += n;
        }
        close(fd);

        console_printf("Received %u bytes\n", fwlen);
        mdelay(100);

        if (fwlen < 0x1000 || fwlen > MAX_TX_FW_SIZE) {
            talloc_free(fw_image);
            xSemaphoreGive(any_fw_upgrade_mutex);
            fw_image = NULL;
            console_printf("Bad TX fw size %u\n", fwlen);
            continue;
        }

        tx_upgrade(fw_image, fw_len);
    } // end while(1)

end:
    vTaskDelete(NULL);
}

/*
  send a DATA96 block
 */
static void send_block(void)
{
    if (fw_image == NULL || fw_len == 0 || acked_to >= fw_len) {
        // nothing to do
        return;
    }
    uint16_t len = fw_len - acked_to;
    if (len > 92) {
        len = 92;
    }
    uint8_t data[96];
    memcpy(&data[0], &acked_to, 4);
    memcpy(&data[4], &fw_image[acked_to], len);

    //console_printf("Sending %u at %u\n", len, acked_to);
    
    mavlink_msg_data96_send(MAVLINK_COMM_FC,
                            TX_UPLOAD_DATA_TYPE,
                            len+4,
                            data);
    last_send_time = get_sys_seconds_boot();
}

/*
  check for sending a new TX firmware block
 */
void tx_upload_periodic(void)
{
    if (fw_image == NULL || fw_len == 0 || acked_to >= fw_len) {
        // nothing to do
        return;
    }
    long long now = get_sys_seconds_boot();
    if (now - last_send_time < 2) {
        return;
    }
    send_block();
}


/*
  handle a DATA16 message for fw upload
 */
void tx_upload_handle_data16(mavlink_data16_t *m)
{
    if (m->type != TX_UPLOAD_DATA_TYPE || m->len != 4) {
        return;
    }
    if (fw_len == 0) {
        return;
    }
    uint8_t last_percent = (unsigned)(100*acked_to/fw_len);
    uint32_t ack_to=0;
    memcpy(&ack_to, &m->data[0], 4);
    acked_to = ack_to;
    uint8_t percent = (unsigned)(100*acked_to/fw_len);
    if (percent != last_percent) {
        console_printf("Sent %u%%\n", percent);
        set_upload_message("Transferring to transmitter");
        set_upload_progress(percent);
    }

    if (acked_to == fw_len) {
        set_upload_progress(100);
        set_upload_message("Finished transfer to transmitter");
        console_printf("TX upload took %u seconds\n", (unsigned)(get_sys_seconds_boot() - upload_start));
        fw_len = 0;
    }
    
    send_block();
}
