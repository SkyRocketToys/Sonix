#include <FreeRTOS.h>
#include <nonstdlib.h>
#include <libmid_mcu/mcu_ctrl.h>
#include "mcu.h"

unsigned char mcu_sync(unsigned char *syspara)
{
	*syspara = 0x00;

	return 0x00;
}

unsigned char mcu_recstart(void)
{

	return 0;
}

unsigned char mcu_usb_detection(unsigned char a)
{
	unsigned char ret = 0;

	if (a == 0x02) { // High
		usbd_set_ext_hotplug_state(1);
		print_msg_queue("USB Detection High\n");

		ret = 0x00;
	} else if (a == 0x04) {	// Low
		usbd_set_ext_hotplug_state(0);
		print_msg_queue("USB Detection Low\n");

		ret = 0x00;
	}

	return ret;
}

mcu_commands_func_t cmd_t = {
	.cmd_sync = mcu_sync,
	.cmd_record_start = mcu_recstart,
	.cmd_usb_detection = mcu_usb_detection
};

int mcu_app_init()
{

	uart2_init(MCU_UART2_BAUD);
	mcu_ctrl_init();
	mcu_cmds_register(&cmd_t);

	print_msg("MCU_init done.\n");
	mcu_connect_notask();

	return 0;
}
