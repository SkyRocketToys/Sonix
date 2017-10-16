#ifndef __SNX_JSON_USBD_H__
#define __SNX_JSON_USBD_H__


#define JSON_CMD_BUF_SIZE	256


int snx_usbd_create_json_stream_video_start_cmd(void);
int snx_usbd_create_json_stream_video_stop_cmd(void);
int snx_usbd_create_json_set_video_parameters_cmd(int br, int res_idx, int fps);
int snx_usbd_create_json_set_record_parameters_cmd(int br, int res_idx, int fps);



#endif /* __SNX_JSON_USBD_H__ */
