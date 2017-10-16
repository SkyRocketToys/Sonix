#include <FreeRTOS.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <nonstdlib.h>
#include "cmd_usbd.h"
#include <usb_device/usb_device.h>
#include <libmid_usbd/mid_usbd.h>
#include "cmd_gpio.h"
#include <gpio/gpio.h>

#define DS5_DEBUG			1

static int usbd_stop = 0;
static int usbd_class_mode = 0;

static void cmd_auto_test(void *pvParameters)
{
//	int usbd_class_mode = 0;
	while(1) {
		if(usbd_class_mode==0)
			usbd_class_mode=1;
		else
			usbd_class_mode=0;

		print_msg_queue("USB Change mode to %s\n",(usbd_class_mode)?"UVC":"MSC");

		usbd_mid_set_class_mode(usbd_class_mode, 0);

		vTaskDelay( 10000 / portTICK_RATE_MS);
		if(usbd_stop==1)
			break;
	}
}

static void cmd_usbd_hid_test(void)
{
		static char test_string[] 	= "Sonix Technology Co., Ltd.";
		uint8_t buf[255];
		uint32_t len=sizeof(test_string);
		strcpy(buf,test_string);	
		usbd_mid_hid_send_string(buf,len);
}
static void cmd_usbd_uvc_switch_preview_resource(void)
{
	print_msg_queue("UVC switch preview resource\n");
	usbd_mid_uvc_events_switch_preview_resource();
}

static int cmd_gpio_test(void)
{
	int num = -1,mode = 0, val = 1;
	int ret = 1 , press_cnt = 0;
	gpio_pin_info info;

	num = 3;

	print_msg_queue("<<test>><%d>\n",__LINE__);
	if (num < 0 || num >6  || (num == 5) )
	{
		print_msg_queue("Wrong PIN num (%d) or mode (%d)\n", num, mode);
		return pdFAIL;
	}

	snx_gpio_open();

	info.pinumber = num;
	info.mode = 1;
	info.value = 0;
	snx_gpio_write(info);


	info.pinumber = num;
	info.mode = 0;

		uint32_t reg;
		uint32_t bypass = 0;
		reg = gpio_get_bypass();
		reg &= ~(1 << info.pinumber);
		reg |= (bypass << info.pinumber);
		gpio_set_bypass(reg);

	snx_gpio_read(&info);
	if (info.value == 1)
	{
		mode = 2;  //set falling mode  for gpio button up read default value  is 1
		val = 1;  
	}
	else
	{
		mode = 1; //set rasing mode  for gpio button up read default value  is 0
		val = 0;
	}
	while(1) {
		snx_gpio_set_interrupt(info.pinumber, mode);
		while (ret != 0)
		{
			ret = snx_gpio_poll (info.pinumber,1000);
//			print_msg_queue("ret = %d\n",ret);
		}

		if (ret == 0 && snx_gpio_poll (info.pinumber,500) == 0) //double click for 0.5s
		{
			print_msg_queue ("double click exit test mode\n");
			break;
		}
		if(usbd_class_mode==0)
			usbd_class_mode=1;
		else
			usbd_class_mode=0;
		usbd_mid_set_class_mode(usbd_class_mode, 0);
		vTaskDelay( 1000 / portTICK_RATE_MS);
		ret = 1;

	}
	snx_gpio_close();

	return pdPASS;
}

int cmd_usbd_class_mode(int argc, char* argv[])
{

//	int usbd_class_mode = 0xFF;
	if (argc == 2) {
		usbd_class_mode = atoi(argv[1]);
	}
	
	
	if ((argc == 2) && ((usbd_class_mode == USBD_MODE_MSC) || (usbd_class_mode == USBD_MODE_UVC) || (usbd_class_mode == USBD_MODE_HID))) {
		print_msg_queue("[CMD-USBD] USB Device class mode change success.\n");
		usbd_mid_set_class_mode(usbd_class_mode, 0);
	} else if(usbd_class_mode==4) {
		if (pdPASS != xTaskCreate(cmd_auto_test, "cmd_auto_test", STACK_SIZE_8K, NULL, 40, NULL))
		{
			print_msg_queue("Fail test\n");
		}
	} else if(usbd_class_mode==5) {
		usbd_stop=1;
	} else if(usbd_class_mode==6) {
		cmd_gpio_test();
	} else if(usbd_class_mode==7) {
		cmd_usbd_hid_test();
	} else if(usbd_class_mode==8) {
		if (usbd_mid_get_class_mode() == USBD_MODE_UVC)
			cmd_usbd_uvc_switch_preview_resource();
		else
			print_msg_queue("[CMD-USBD] Current mode is not USB Video Class Mode.\n");

	} else if (argc == 1){
		usbd_class_mode = usbd_mid_get_class_mode();
		if (usbd_class_mode == USBD_MODE_MSC) {
			print_msg_queue("[CMD-USBD] Current mode is USB Mass-Storage Class Mode.\n");
		} else if (usbd_class_mode == USBD_MODE_UVC) {
			print_msg_queue("[CMD-USBD] Current mode is USB Video Class Mode.\n");
		} else if (usbd_class_mode == USBD_MODE_HID) {
			print_msg_queue("[CMD-USBD] Current mode is USB Human Interface Device Mode.\n");
		} else {
			print_msg_queue("[CMD-USBD] Current mode unknown!\n");
		}
		print_msg_queue("\n");
		print_msg_queue("Usage: mode [Option]\n");
		print_msg_queue("Available options are\n");
		print_msg_queue("%d:  USB Mass-Storage Class Mode\n", (int)USBD_MODE_MSC);
		print_msg_queue("%d:  USB Video Class Mode.\n", (int)USBD_MODE_UVC);
		print_msg_queue("%d:  USB HID Mode.\n", (int)USBD_MODE_HID);		
		print_msg_queue("4:  Start Switch Test Mode.\n");
		print_msg_queue("5:  Stop Switch Test Mode.\n");
		print_msg_queue("6:  GPIO Switch Test Mode.\n");
		print_msg_queue("7:  Send Data to PC with HID Mode.\n");		
		print_msg_queue("8:  Switch USBD preview resource.\n");



	}
	
	return 0;
}
