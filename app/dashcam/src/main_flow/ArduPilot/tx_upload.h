#include "web_server/mavlink_core.h"

void tx_fw_upload_task_process(void *pvParameters);
void tx_upload_periodic(void);
void tx_upload_handle_data16(mavlink_data16_t *m);
void tx_upgrade(const uint8_t *image, unsigned size);

#define TX_UPLOAD_DATA_TYPE 42
#define TX_PLAY_DATA_TYPE 43

