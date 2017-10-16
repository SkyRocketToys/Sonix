#include <generated/snx_sdk_conf.h>
#include "rec_schedule.h"
#include "snapshot.h"

void stop_access_sdcard(void)
{
	diable_rec();
#ifndef CONFIG_APP_DRONE
	disable_reclapse();
#endif
	stop_thumbnail_access_sdcard();
}

void start_access_sdcard(void)
{
	enable_rec();
#ifndef CONFIG_APP_DRONE
	enable_reclapse();
#endif
	start_thumbnail_access_sdcard();
}
