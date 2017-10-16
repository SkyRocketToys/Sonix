/**
 * @file
 * this is application header file for snapshot, include this file before use
 * @author Algorithm Dept Sonix.
 */

/**
* @brief max number of queue item
*/

#include <libmid_rec/record.h>
#include "rec_common.h"

#define SHAPSHOT_MAX_QUEUE_ITEM 5
#define SNAPSHOT_PATH_LEN 34

/**
* @brief structure for quene item
*/
typedef struct _pic_info {
	unsigned char *addr;	/**<  address of saving image*/
	unsigned int  size;		/**<  size of saving image*/
	char *picname;
	unsigned int type;
	unsigned char *OriginAddr;
	unsigned int dev; //current storage, 1:mmc, 2:usbh
} pic_info_t;

/**
* @brief structure for snapshot infomation
*/
typedef struct _shapshot_info {
	xTaskHandle task_save_pic;	/**<  task to saving image to sd card*/
	xQueueHandle queue_pic;		/**<  queue to tansfer image info to task*/
	enum RECORDFILE_TYPE fileformat; // 0: timebase 1:count
	unsigned int snapshot_count;
	char snapshotpath[LEN_FILENAME];
} snapshot_info_t;

int mf_snapshot_init(void);
int mf_snapshot_uninit(void);
void mf_snapshot_get_pic(unsigned char *pPic, unsigned int size, const char *name, unsigned int type);
void mf_thumbnail_get_pic(unsigned char *pPic, unsigned int size, const char *name, unsigned int type);
int mf_snapshot_status(void);
void mf_snapshot_update_size(long long iUsedSize, long long iCanuseSize);
int mf_snapshot_get_isfull(void);
int mf_snapshot_get_queue_num(void);
int del_snapshot_file(char *fn);

int mf_thumbnail_init(void);
int mf_thumbnail_uninit(void);
void stop_thumbnail_access_sdcard(void);
void start_thumbnail_access_sdcard(void);
int mf_thumbnail_status(void);
int mf_set_thumbnail(void);
int get_filepath(char *szFilePath, int len);
#if defined(CONFIG_SYSTEM_PLATFORM_SN98660) || defined(CONFIG_SYSTEM_PLATFORM_SN98661) || defined(CONFIG_SYSTEM_PLATFORM_SN98671) || defined (CONFIG_SYSTEM_PLATFORM_SN98293)
int piclist_to_file(char *path);
#endif
