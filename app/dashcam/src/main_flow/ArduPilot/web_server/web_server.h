#pragma once

#include <FreeRTOS.h>
#include <bsp.h>
#include <task.h>
#include "../dev_console.h"

struct connection_state;

#include "cgi.h"

#define WEB_SERVER_PORT 80

/*
  structure for output buffering
 */
struct sock_buf {
    bool add_content_length;
    uint32_t header_length;
    char *buf;
    int fd;
};


/*
  state of one connection
 */
struct connection_state {
    xTaskHandle task;
    struct sock_buf *sock;
    struct cgi_state *cgi;
};

void web_server_task_process(void *pvParameters);
void connection_destroy(struct connection_state *c);
int32_t sock_write(struct sock_buf *sock, const char *s, size_t size);
void sock_printf(struct sock_buf *sock, const char *fmt, ...) FMT_PRINTF(2,3);
void web_server_set_debug(int debug);
void web_debug(int level, const char *fmt, ...);
void mavlink_fc_write(const uint8_t *buf, size_t len);
void mavlink_rc_write(const uint8_t *buf, uint32_t len);

