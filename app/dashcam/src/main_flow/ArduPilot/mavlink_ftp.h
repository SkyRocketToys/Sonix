#pragma once

/*
  MAVLink FTP (FILE_TRANSFER_PROTOCOL) for the SD card, served by the
  camera component to GCSes on WiFi.

  The MAVLink thread hands requests in with mavlink_ftp_request(), and
  takes replies out with mavlink_ftp_get_reply() to pack and send; the
  card is only touched by the FTP task, so a slow card can't hold up
  the flight controller link.
 */

#include <stdbool.h>
#include <stdint.h>
#include "lwip/sockets.h"
#include <mavlink_core.h>

#define MAVLINK_FTP_PAYLOAD_LEN 251

struct mavlink_ftp_reply {
    struct sockaddr_in addr;    // the GCS that asked, and only it, gets the reply
    uint32_t sysid;             // its system and component IDs
    uint8_t compid;
    uint8_t payload[MAVLINK_FTP_PAYLOAD_LEN];
};

void mavlink_ftp_request(const mavlink_message_t *msg, const struct sockaddr_in *from);
bool mavlink_ftp_get_reply(struct mavlink_ftp_reply *reply);
bool mavlink_ftp_busy(void);
void mavlink_ftp_status(void);
