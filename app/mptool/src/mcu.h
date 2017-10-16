#ifndef _MCU_H_
#define _MCU_H_

#include <generated/snx_sdk_conf.h>

#ifndef CONFIG_MIDDLEWARE_MCU_CTRL
#define MCU_APP_INIT()
#else
#define MCU_APP_INIT()	mcu_app_init()
#endif

#endif
