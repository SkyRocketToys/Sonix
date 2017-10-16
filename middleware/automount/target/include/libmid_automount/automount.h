#include <libmid_fatfs/ff.h>

typedef struct _automount_info{
    xSemaphoreHandle SdFwupgradeMutex;
    xSemaphoreHandle SdRecordMutex;
    xSemaphoreHandle SdProtectMutex;
	xSemaphoreHandle SdTimeLapseMutex;
    volatile int cd_pin;
    volatile int mt_status;
    FRESULT fs_status;
    volatile int the_first_time;
 }automount_info_t;

int automount_init(void);
void automount_uninit(void);
automount_info_t* get_automount_info(void);
int get_sd_umount_err(void);
int mid_sd_remount(void);
void mid_sd_refresh_mutex(void);
void unlock_protect_mutex(void);
void unlock_record_mutex(void);
void unlock_timelapse_mutex(void);
int mid_phydrv_detect(void);

FRESULT get_sd_status(void);
