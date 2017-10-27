/*
  simple web server for sonix board

  based on tserver:

  https://github.com/tridge/junkcode/tree/master/tserver
*/
#include "web_server.h"
#include "includes.h"
#include "web_files.h"
#include <libmid_nvram/snx_mid_nvram.h>

static int num_sockets_open;
static int debug_level;

// public web-site that will be allowed. Can be edited with NVRAM editor
static const char *public_origin = "fly.sky-viper.com";

void web_server_set_debug(int level)
{
    debug_level = level;
}

void web_debug(int level, const char *fmt, ...)
{
    if (level > debug_level) {
        return;
    }
    va_list ap;
    va_start(ap, fmt);
    console_vprintf(fmt, ap);
    va_end(ap);
}

/*
  destroy socket buffer, writing any pending data
 */
static int sock_buf_destroy(struct sock_buf *sock)
{
    if (sock->add_content_length) {
        /*
          support dynamic content by delaying the content-length
          header. This is needed to keep some anti-virus programs
          (eg. AVG Free) happy. Without a content-length the load of
          dynamic json can hang
         */
        uint32_t body_size = talloc_get_size(sock->buf) - sock->header_length;
        write(sock->fd, sock->buf, sock->header_length);
        char *clen = print_printf(sock, "Content-Length: %u\r\n\r\n", body_size);
        if (clen) {
            write(sock->fd, clen, talloc_get_size(clen));
        }
        write(sock->fd, sock->buf + sock->header_length, body_size);
    } else {
        size_t size = talloc_get_size(sock->buf);
        if (size > 0) {
            write(sock->fd, sock->buf, size);
        }
    }
    web_debug(3,"closing fd %d num_sockets_open=%d\n", sock->fd, num_sockets_open);
    num_sockets_open--;
    close(sock->fd);
    return 0;
}

/*
  write to sock_buf
 */
ssize_t sock_write(struct sock_buf *sock, const char *s, size_t size)
{
    size_t current_size = talloc_get_size(sock->buf);
    ssize_t ret;
    if (!sock->add_content_length &&
        (size >= 1000 || (size >= 200 && current_size == 0))) {
        if (current_size > 0) {
            ret = write(sock->fd, sock->buf, current_size);
            if (ret != current_size) {
                return -1;
            }
            talloc_free(sock->buf);
            sock->buf = NULL;
        }
        ret = write(sock->fd, s, size);
    } else {
        sock->buf = talloc_realloc_size(sock, sock->buf, current_size + size);
        if (sock->buf) {
            memcpy(sock->buf + current_size, s, size);
        }
        ret = size;
    }
    return ret;
}

/*
  print to socket buffer
 */
void sock_printf(struct sock_buf *sock, const char *fmt, ...)
{
    if (strchr(fmt, '%') == NULL) {
        // simple string
        sock_write(sock, fmt, strlen(fmt));
        return;
    }
    va_list ap;
    va_start(ap, fmt);
    char *buf2 = print_vprintf(sock, fmt, ap);
    sock_write(sock, buf2, talloc_get_size(buf2));
    talloc_free(buf2);
    va_end(ap);
}


/*
  destroy a connection
*/
void connection_destroy(struct connection_state *c)
{
    talloc_free(c);
#ifdef SYSTEM_FREERTOS
    vTaskDelete(NULL);
#else
    pthread_exit(0);
#endif
}


#ifndef SYSTEM_FREERTOS
/*
  write some data to the flight controller
 */
void mavlink_fc_write(const uint8_t *buf, size_t size)
{
    if (serial_port_fd != -1) {
        write(serial_port_fd, buf, size);
    }
    if (fc_udp_in_fd != -1) {
        if (fc_addrlen != 0) {
            sendto(fc_udp_in_fd, buf, size, 0, (struct sockaddr*)&fc_addr, fc_addrlen);
        }
    }
}

/*
  send a mavlink message to flight controller
 */
void mavlink_fc_send(mavlink_message_t *msg)
{
    if (serial_port_fd != -1) {
        _mavlink_resend_uart(MAVLINK_COMM_FC, msg);
    }
    if (fc_udp_in_fd != -1) {
        uint8_t buf[600];
        uint16_t len = mavlink_msg_to_send_buffer(buf, msg);
        mavlink_fc_write(buf, len);
    }
}

/*
  send a mavlink msg over WiFi
 */
static void mavlink_broadcast(int fd, mavlink_message_t *msg)
{
    uint8_t buf[300];
    uint16_t len = mavlink_msg_to_send_buffer(buf, msg);
    if (len > 0) {
        struct sockaddr_in addr;
        memset(&addr, 0x0, sizeof(addr));

        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = htonl(INADDR_BROADCAST);
        addr.sin_port = htons(14550);
        
        sendto(fd, buf, len, 0, (struct sockaddr*)&addr, sizeof(addr));
    }
}
#endif

/*
  process input on a connection
*/
static void connection_process(struct connection_state *c)
{
    if (!c->cgi->setup(c->cgi)) {
        connection_destroy(c);
        return;
    }
    c->cgi->load_variables(c->cgi);
    web_debug(3, "processing '%s' on %d num_sockets_open=%d\n", c->cgi->pathinfo, c->cgi->sock->fd, num_sockets_open);
    c->cgi->download(c->cgi, c->cgi->pathinfo);
    web_debug(3, "destroying '%s' fd=%d\n", c->cgi->pathinfo, c->cgi->sock->fd);
    connection_destroy(c);
}

/*
  check origin header to prevent attacks by javascript in other web pages
 */
static bool check_origin(const char *origin)
{
    if (strcmp(origin, "http://192.168.99.1") == 0) {
        // always accept
        return true;
    }

    // could be a different local IP
    char local_ip[16];
    get_local_ip(local_ip, sizeof(local_ip));
    if (strncmp(origin, "http://", 7) == 0 &&
        strcmp(origin+7, local_ip) == 0) {
        return true;
    }
    
    // also allow file:// URLs which produce a 'null' origin
    if (strcmp(origin, "null") == 0) {
        return true;
    }
    char *allowed_origin;
    unsigned origin_length = 0;
    if (snx_nvram_get_data_len("SkyViper", "AllowedOrigin", &origin_length) != NVRAM_SUCCESS) {
        return false;
    }
    allowed_origin = talloc_zero_size(NULL, origin_length);
    if (allowed_origin == NULL) {
        return false;
    }
    if (snx_nvram_string_get("SkyViper", "AllowedOrigin", allowed_origin) != NVRAM_SUCCESS) {
        talloc_free(allowed_origin);
        return false;
    }
    // check for wildcard allowed origin
    if (strcmp(allowed_origin, "*") == 0) {
        talloc_free(allowed_origin);
        return true;
    }

    // allow for http:// or https://
    if (strncmp(origin, "http://", 7) == 0) {
        origin += 7;
    } else if (strncmp(origin, "https://", 8) == 0) {
        origin += 8;
    } else {
        console_printf("Denied origin protocol: [%s]\n", origin);
        talloc_free(allowed_origin);
        return false;
    }
    
    if (strcmp(allowed_origin, origin) != 0) {
        console_printf("Denied origin: [%s] allowed: [%s]\n", origin, allowed_origin);
        talloc_free(allowed_origin);
        return false;
    }
    talloc_free(allowed_origin);
    return true;
}

/*
  setup AllowedOrigin if not set already
 */
static void setup_origin(const char *origin)
{
    unsigned origin_length = 0;
    if (snx_nvram_get_data_len("SkyViper", "AllowedOrigin", &origin_length) != NVRAM_SUCCESS ||
        origin_length == 0) {
        snx_nvram_string_set("SkyViper", "AllowedOrigin", __DECONST(char *,origin));
    }
}

/*
  task for web_server
*/
static void web_server_connection_process(void *pvParameters)
{
    struct connection_state *c = pvParameters;
    c->cgi = cgi_init(c, c->sock);
    if (!c->cgi) {
        connection_destroy(c);
        return;
    }

    c->cgi->check_origin = check_origin;

    connection_process(c);
}

/*
  task for web_server
*/
void web_server_task_process(void *pvParameters)
{
    int listen_sock;

    struct sockaddr_in addr;

    // setup default allowed origin
    setup_origin(public_origin);
    
    if ((listen_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
        goto end;
    }

    memset(&addr, 0x0, sizeof(addr));

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(WEB_SERVER_PORT);

    if (bind(listen_sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        goto end;
    }

    if (listen(listen_sock, 20) < 0) {
        goto end;
    }

    while (1)
    {
        fd_set fds;
        struct timeval tv;
        int numfd = listen_sock+1;

        FD_ZERO(&fds);
        FD_SET(listen_sock, &fds);

        tv.tv_sec = 1;
        tv.tv_usec = 0;

        int res = select(numfd, &fds, NULL, NULL, &tv);
        if (res <= 0) {
            continue;
        }

        if (FD_ISSET(listen_sock, &fds)) {
            // new connection
            struct sockaddr_in addr;
            int len = sizeof(struct sockaddr_in);
            int fd = accept(listen_sock, (struct sockaddr *)&addr, (socklen_t *)&len);
            if (fd != -1) {
                struct connection_state *c = talloc_zero(NULL, struct connection_state);
                if (c == NULL) {
                    close(fd);
                    continue;
                }
                c->sock = talloc_zero(c, struct sock_buf);
                if (!c->sock) {
                    talloc_free(c);
                    close(fd);
                    continue;
                }
                c->sock->fd = fd;
                num_sockets_open++;
                talloc_set_destructor(c->sock, sock_buf_destroy);
                xTaskCreate(web_server_connection_process, "http_connection", STACK_SIZE_4K, c, 10, &c->task);
            }
        }
    }

end:
    vTaskDelete(NULL);
}
