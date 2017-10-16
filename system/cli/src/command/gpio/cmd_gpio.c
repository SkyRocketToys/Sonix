#include <FreeRTOS.h>
#include <task.h>
#include <bsp.h>
#include <stdio.h>
#include "cmd_gpio.h"
#include "cmd_debug.h"
#include "printlog.h"
#include <nonstdlib.h>
#include <gpio/gpio.h>
#include <string.h>
#include <nonstdlib.h>
#include <generated/snx_sdk_conf.h>

#define DEBUG 1
#define printf(fmt, args...) if(DEBUG) print_msg_queue((fmt), ##args)

int cmd_gpio_ctrl(int argc, char* argv[])
{
	int num = -1,mode = 0, val = 1;
	gpio_pin_info info;

	if (argc < 3) {
		printf(" Usage: gpio_ctrl [num] [mode] [value]\n");
		printf(" num: 0, 1, 2, 3, 4, 6\n");
		printf(" mode:\n");
		printf("      0: Input mode \n");
		printf("      1: Output mode\n");

		return pdFAIL;
	}

	num = simple_strtoul(argv[1], NULL, 10);
	mode = simple_strtoul(argv[2], NULL, 10);

	if (num < 0 || num >6  || (num == 5) || mode <0 || mode >1 )
	{
		printf("Wrong PIN num (%d) or mode (%d)\n", num, mode);
		return pdFAIL;
	}

	if (mode == 1) {
		if (argc > 3)
			val = simple_strtoul(argv[3], NULL, 10);
		else {
			printf("NO write value parameter\n");
			return pdFAIL;
		}
	}

	if (val)
		val = 1;
	else
		val = 0;


	/*
		Start to Control GPIO:
		1. OPEN GPIO
		2. config its info structure
		3. write opt for output mode
		4. read opt for input mode
		5. close GPIO
	*/
	snx_gpio_open();

	info.pinumber = num;
	info.mode = mode;
	info.value = val;

	{
		uint32_t reg;
		uint32_t bypass = 0;
		reg = gpio_get_bypass();
		reg &= ~(1 << info.pinumber);
		reg |= (bypass << info.pinumber);
		gpio_set_bypass(reg);
	}

	if (mode == 1)
		snx_gpio_write(info);

	if(mode == 0)
	{
		snx_gpio_read(&info);
	}

#if CONFIG_APP_MPTOOL // customer mptool request
	printf("GPIO%d %s\n", num, info.value ? "HIGH" : "LOW");
#else
	printf("GPIO Pin(%d), Mode(%d), val(%d)\n", num, mode, info.value);
#endif
	
	snx_gpio_close();

	return pdPASS;   
}

int cmd_pwm_gpio_ctrl(int argc, char* argv[])
{
	int num = -1,mode = 0, val = 1;
	gpio_pin_info info;

	if (argc < 3) {
		printf(" Usage: pwm_gpio_ctrl [num] [mode] [value]\n");
		printf(" num: 0, 1, 2\n");
		printf(" mode:\n");
		printf("      0: Input mode \n");
		printf("      1: Output mode\n");

		return pdFAIL;
	}

	num = simple_strtoul(argv[1], NULL, 10);
	mode = simple_strtoul(argv[2], NULL, 10);

	if (num < 0 || num > 2 || mode <0 || mode >1 )
	{
		printf("Wrong PIN num (%d) or mode (%d)\n", num, mode);
		return pdFAIL; 
	}

	if (mode == 1) {
		if (argc > 3)
			val = simple_strtoul(argv[3], NULL, 10);
		else {
			printf("NO write value parameter\n");
			return pdFAIL;
		}
	}

	if (val)
		val = 1;
	else
		val = 0;

	/*
		Start to Control GPIO:
		1. OPEN GPIO
		2. config its info structure
		3. write opt for output mode
		4. read opt for input mode
		5. close GPIO
	*/
	snx_pwm_gpio_open();

	info.pinumber = num;
	info.mode = mode;
	info.value = val;

	if (mode == 1)
		snx_pwm_gpio_write(info);

	if(mode == 0)
	{
		snx_pwm_gpio_read(&info);
	}

#if CONFIG_APP_MPTOOL // customer mptool request
	printf("PWM%d %s\n", num, info.value ? "HIGH" : "LOW");
#else
	printf("PWM Pin(%d), Mode(%d), val(%d)\n", num, mode, info.value);
#endif
	
	snx_pwm_gpio_close();

	return pdPASS;   
}

int cmd_spi_gpio_ctrl(int argc, char* argv[])
{
	int num = -1,mode = 0, val = 1;
	gpio_pin_info info;

	if (argc < 3) {
		printf(" Usage: spi_gpio_ctrl [num] [mode] [value]\n");
		printf(" num:\n");
		printf("      0: SPICLK PIN\n");
		printf("      1: SPIFS PIN\n");
		printf("      2: SPITX PIN\n");
		printf("      3: SPIRX PIN\n");
		printf(" mode:\n");
		printf("      0: Input mode \n");
		printf("      1: Output mode\n");

		return pdFAIL;
	}

	num = simple_strtoul(argv[1], NULL, 10);
	mode = simple_strtoul(argv[2], NULL, 10);

	if (num < 0 || num >  3 || mode <0 || mode >1 )
	{
		printf("Wrong PIN num (%d) or mode (%d)\n", num, mode);
		return pdFAIL;
	}

	if (mode == 1) {
		if (argc > 3)
			val = simple_strtoul(argv[3], NULL, 10);
		else {
			printf("NO write value parameter\n");
			return pdFAIL;
		}
	}

	if (val)
		val = 1;
	else
		val = 0;

	/*
		Start to Control GPIO:
		1. OPEN GPIO
		2. config its info structure
		3. write opt for output mode
		4. read opt for input mode
		5. close GPIO
	*/
	snx_spi_gpio_open();

	info.pinumber = num;
	info.mode = mode;
	info.value = val;

	if (mode == 1)
		snx_spi_gpio_write(info);

	if(mode == 0)
	{
		snx_spi_gpio_read(&info);
	}

#if CONFIG_APP_MPTOOL // customer mptool request
	printf("SPI%d %s\n", num, info.value ? "HIGH" : "LOW");
#else
	printf("SPI Pin(%d), Mode(%d), val(%d)\n", num, mode, info.value);
#endif
	
	snx_spi_gpio_close();

	return pdPASS;   
}

int cmd_ms1_gpio_ctrl(int argc, char* argv[])
{
	int num = -1,mode = 0, val = 1;
	gpio_pin_info info;

	if (argc < 3) {
		printf(" Usage: ms1_gpio_ctrl [num] [mode] [value]\n");
		printf(" num:\n");
		printf(" mode:\n");
		printf("      0: Input mode \n");
		printf("      1: Output mode\n");

		return pdFAIL;
	}

	num = simple_strtoul(argv[1], NULL, 10);
	mode = simple_strtoul(argv[2], NULL, 10);

	if (num < 0 || num >  5 || mode <0 || mode >1 )
	{
		printf("Wrong PIN num (%d) or mode (%d)\n", num, mode);
		return pdFAIL;
	}

	if (mode == 1) {
		if (argc > 3)
			val = simple_strtoul(argv[3], NULL, 10);
		else {
			printf("NO write value parameter\n");
			return pdFAIL;
		}
	}

	if (val)
		val = 1;
	else
		val = 0;

	/*
		Start to Control GPIO:
		1. OPEN GPIO
		2. config its info structure
		3. write opt for output mode
		4. read opt for input mode
		5. close GPIO
	*/
	snx_ms1_gpio_open();

	info.pinumber = num;
	info.mode = mode;
	info.value = val;

	if (mode == 1)
		snx_ms1_gpio_write(info);

	if(mode == 0)
	{
		snx_ms1_gpio_read(&info);
	}

#if CONFIG_APP_MPTOOL // customer mptool request
	printf("MS1%d %s\n", num, info.value ? "HIGH" : "LOW");
#else
	printf("MS1 Pin(%d), Mode(%d), val(%d)\n", num, mode, info.value);
#endif

	snx_ms1_gpio_close();

	return pdPASS;   
}



int cmd_aud_gpio_ctrl(int argc, char* argv[])
{
	int num = -1,mode = 0, val = 1;
	gpio_pin_info info;

	if (argc < 3) {
		printf(" Usage: aud_gpio_ctrl [num] [mode] [value]\n");
		printf(" num: 0, 1, 2, 3, 4\n");
		printf(" mode:\n");
		printf("      0: Input mode \n");
		printf("      1: Output mode\n");

		return pdFAIL;
	}

	num = simple_strtoul(argv[1], NULL, 10);
	mode = simple_strtoul(argv[2], NULL, 10);

	if (num < 0 || num >  4 || mode <0 || mode >1 )
	{
		printf("Wrong PIN num (%d) or mode (%d)\n", num, mode);
		return pdFAIL;
	}

	if (mode == 1) {
		if (argc > 3)
			val = simple_strtoul(argv[3], NULL, 10);
		else {
			printf("NO write value parameter\n");
			return pdFAIL;
		}
	}

	if (val)
		val = 1;
	else
		val = 0;

	/*
		Start to Control GPIO:
		1. OPEN GPIO
		2. config its info structure
		3. write opt for output mode
		4. read opt for input mode
		5. close GPIO
	*/
	snx_audio_gpio_open();

	info.pinumber = num;
	info.mode = mode;
	info.value = val;

	if (mode == 1)
		snx_audio_gpio_write(info);

	if(mode == 0)
	{
		snx_audio_gpio_read(&info);
	}

#if CONFIG_APP_MPTOOL // customer mptool request
	printf("AUD%d %s\n", num, info.value ? "HIGH" : "LOW");
#else
	printf("AUD Pin(%d), Mode(%d), val(%d)\n", num, mode, info.value);
#endif

	snx_audio_gpio_close();

	return pdPASS;   
}

int cmd_i2c_gpio_ctrl(int argc, char* argv[])
{
	int num = -1,mode = 0, val = 1;
	gpio_pin_info info;

	if (argc < 3) {
		printf(" Usage: i2c_gpio_ctrl [num] [mode] [value]\n");
		printf(" num: 0: I2C1_SCL, 1: I2C1_SDA, 2: I2C2_SCL, 3: I2C2_SDA\n");
		printf(" mode:\n");
		printf("      0: Input mode \n");
		printf("      1: Output mode\n");
		printf("      2: I2C mode\n");

		return pdFAIL;
	}

	num = simple_strtoul(argv[1], NULL, 10);
	mode = simple_strtoul(argv[2], NULL, 10);

	if (num < 0 || num >  3 || mode <0 || mode >2 )
	{
		printf("Wrong PIN num (%d) or mode (%d)\n", num, mode);
		return pdFAIL;
	}

	if (mode == 1) {
		if (argc > 3)
			val = simple_strtoul(argv[3], NULL, 10);
		else {
			printf("NO write value parameter\n");
			return pdFAIL;
		}
	}

	if (val)
		val = 1;
	else
		val = 0;

	/*
		Start to Control GPIO:
		1. OPEN GPIO
		2. config its info structure
		3. write opt for output mode
		4. read opt for input mode
		5. close GPIO
	*/
	snx_i2c_gpio_open();

	if (mode == 2)
		snx_i2c_gpio_disable(num/2);
	else
		snx_i2c_gpio_enable(num/2);

	info.pinumber = num;
	info.mode = mode;
	info.value = val;

	if (mode == 1)
		snx_i2c_gpio_write(info);

	if(mode == 0)
	{
		snx_i2c_gpio_read(&info);
	}

#if CONFIG_APP_MPTOOL // customer mptool request
	printf("I2C%d %s\n", num, info.value ? "HIGH" : "LOW");
#else
	printf("I2C Pin(%d), Mode(%d), val(%d)\n", num, mode, info.value);
#endif
	
	snx_i2c_gpio_close();

	return pdPASS;   
}


#if 1
int cmd_gpio_ctrl_interrupt(int argc, char* argv[])
{
	int num = -1,mode = 0, val = 1;
	int ret = 0;
	gpio_pin_info info;

	if (argc < 4) {
		printf(" Usage: gpio_ctrl_interrupt [num] [mode] [timeout]\n");
		printf(" num: 0, 1, 2, 3, 4, 6\n");
		printf(" mode:\n");
		printf("   Interrupt mode set 0: none 1: rasing 2: falling 3: both\n");
		printf(" timeout:\n");
		printf("   unit: ms\n");
		return pdFAIL;
	}

	num = simple_strtoul(argv[1], NULL, 10);
	mode = simple_strtoul(argv[2], NULL, 10);
	val =  simple_strtoul(argv[3], NULL, 10); 
	if (num < 0 || num >6  || (num == 5) || mode <0 || mode >3 )
	{
		printf("Wrong PIN num (%d) or mode (%d)\n", num, mode);
		return pdFAIL;
	}


	/*
	   Start to Control GPIO:
	   1. OPEN GPIO
	   2. config its info structure
	   3. write opt for output mode
	   4. read opt for input mode
	   5. close GPIO
	   */
	snx_gpio_open();

	info.pinumber = num;
	info.mode = 0;


	{
		uint32_t reg;
		uint32_t bypass = 0;
		reg = gpio_get_bypass();
		reg &= ~(1 << info.pinumber);
		reg |= (bypass << info.pinumber);
		gpio_set_bypass(reg);
	}


	snx_gpio_read(&info);
	snx_gpio_set_interrupt(info.pinumber, mode);
	ret = snx_gpio_poll (info.pinumber,val);
	if (ret == 0)
		print_msg_queue ("interrupt happen\n");
	else
		print_msg_queue ("interrupt no happen\n");

#if CONFIG_APP_MPTOOL // customer mptool request
	printf("GPIO%d %s\n", num, info.value ? "HIGH" : "LOW");
#else
	//print_msg_queue("GPIO Pin(%d), Mode(%d), val(%d)\n", num, mode, info.value);
#endif

	snx_gpio_close();

	return pdPASS;   
}

int cmd_gpio_ctrl_click(int argc, char* argv[])
{
	int num = -1,mode = 0, val = 1;
	int ret = 1 , press_cnt = 0;
	gpio_pin_info info;

	if (argc < 2) {
		print_msg_queue(" Usage: gpio_ctrl_click [num]  \n");
		print_msg_queue(" num: 0, 1, 2, 3, 4, 6\n");

		return pdFAIL;
	}

	num = simple_strtoul(argv[1], NULL, 10);

	if (num < 0 || num >6  || (num == 5) )
	{
		print_msg_queue("Wrong PIN num (%d) or mode (%d)\n", num, mode);
		return pdFAIL;
	}


	/*
	   Start to Control GPIO:
	   1. OPEN GPIO
	   2. config its info structure
	   3. write opt for output mode
	   4. read opt for input mode
	   5. close GPIO
	   */
	snx_gpio_open();

	info.pinumber = num;
	info.mode = 0;

#if 1
	{
		uint32_t reg;
		uint32_t bypass = 0;
		reg = gpio_get_bypass();
		reg &= ~(1 << info.pinumber);
		reg |= (bypass << info.pinumber);
		gpio_set_bypass(reg);
	}
#endif

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
	snx_gpio_set_interrupt(info.pinumber, mode);

	while (ret != 0)
	{
		ret = snx_gpio_poll (info.pinumber,1000);
		//print_msg_queue("ret = %d\n",ret);
	}
	//print_msg_queue("interrupt happen\n");
	if (ret == 0 && snx_gpio_poll (info.pinumber,500) == 0) //double click for 0.5s
	{
		print_msg_queue ("double click happen\n");

	}
	else 
	{
		snx_gpio_read(&info);

		while(info.value == !val) {  

			cpu_udelay(10000);
			snx_gpio_read(&info);
			press_cnt++;
			if(press_cnt > 100) {
				print_msg_queue ("long press\n"); // Long press
				break;
			}
		}
		if (press_cnt < 100)
			print_msg_queue ("short press\n"); // short press

	}


	snx_gpio_close();

	return pdPASS;
}

int cmd_uart2_gpio_ctrl(int argc, char* argv[])
{
	int num = -1,mode = 0, val = 1;
	gpio_pin_info info;

	if (argc < 3) {
		printf(" Usage: uart2_gpio_ctrl [num] [mode] [value]\n");
		printf(" num: 0, 1\n");
		printf(" mode:\n");
		printf("      0: Input mode \n");
		printf("      1: Output mode\n");

		return pdFAIL;
	}

	num = simple_strtoul(argv[1], NULL, 10);
	mode = simple_strtoul(argv[2], NULL, 10);

	if (num < 0 || num > 1 || mode <0 || mode >1 )
	{
		printf("Wrong PIN num (%d) or mode (%d)\n", num, mode);
		return pdFAIL; 
	}

	if (mode == 1) {
		if (argc > 3)
			val = simple_strtoul(argv[3], NULL, 10);
		else {
			printf("NO write value parameter\n");
			return pdFAIL;
		}
	}

	if (val)
		val = 1;
	else
		val = 0;

	/*
		Start to Control GPIO:
		1. OPEN GPIO
		2. config its info structure
		3. write opt for output mode
		4. read opt for input mode
		5. close GPIO
	*/
	snx_uart2_gpio_open();

	info.pinumber = num;
	info.mode = mode;
	info.value = val;

	if (mode == 1)
		snx_uart2_gpio_write(info);

	if(mode == 0)
	{
		snx_uart2_gpio_read(&info);
	}

#if CONFIG_APP_MPTOOL // customer mptool request
	printf("UART2%d %s\n", num, info.value ? "HIGH" : "LOW");
#else
	printf("UART2 Pin(%d), Mode(%d), val(%d)\n", num, mode, info.value);
#endif
	
	snx_uart2_gpio_close();

	return pdPASS;   
}

int cmd_jtag_gpio_ctrl(int argc, char* argv[])
{
	int num = -1,mode = 0, val = 1;
	gpio_pin_info info;

	if (argc < 3) {
		printf(" Usage: jtag_gpio_ctrl [num] [mode] [value]\n");
		printf(" num: 0, 1,2,3,4\n");
		printf(" mode:\n");
		printf("      0: Input mode \n");
		printf("      1: Output mode\n");

		return pdFAIL;
	}

	num = simple_strtoul(argv[1], NULL, 10);
	mode = simple_strtoul(argv[2], NULL, 10);

	if (num < 0 || num > 4 || mode <0 || mode >1 )
	{
		printf("Wrong PIN num (%d) or mode (%d)\n", num, mode);
		return pdFAIL; 
	}

	if (mode == 1) {
		if (argc > 3)
			val = simple_strtoul(argv[3], NULL, 10);
		else {
			printf("NO write value parameter\n");
			return pdFAIL;
		}
	}

	if (val)
		val = 1;
	else
		val = 0;

	/*
	   Start to Control GPIO:
	   1. OPEN GPIO
	   2. config its info structure
	   3. write opt for output mode
	   4. read opt for input mode
	   5. close GPIO
	   */
	snx_jtag_gpio_enable(1);
	snx_jtag_gpio_open();

	info.pinumber = num;
	info.mode = mode;
	info.value = val;

	if (mode == 1)
		snx_jtag_gpio_write(info);

	if(mode == 0)
	{
		snx_jtag_gpio_read(&info);
	}

#if CONFIG_APP_MPTOOL // customer mptool request
	printf("JTAG%d %s\n", num, info.value ? "HIGH" : "LOW");
#else
	printf("JTAG Pin(%d), Mode(%d), val(%d)\n", num, mode, info.value);
#endif

	snx_jtag_gpio_close();

	return pdPASS;   
}



int cmd_totaldev_gpio_ctrl(int argc, char* argv[])
{
	int dev = -1, num = -1,mode = 0, val = 1;
	gpio_pin_info info;

	if (argc < 4) {
		printf(" Usage: totaldev_gpio_ctrl [dev] [num] [mode] [value]\n");
		printf(" dev: 0:gpio, 1:pwm, 2:spi, 3: ms1, 4:audio, 5:i2c2, 6:uart2 7:jtag \n");
		printf(" num: \n");
		printf(" mode:\n");
		printf("      0: Input mode \n");
		printf("      1: Output mode\n");
		printf(" value:\n");
		printf("      0: low \n");
		printf("      1: high\n");

		return pdFAIL;
	}
	dev = simple_strtoul(argv[1], NULL, 10);
	num = simple_strtoul(argv[2], NULL, 10);
	mode = simple_strtoul(argv[3], NULL, 10);

	if (mode <0 || mode >1 )
	{
		printf("Wrong PIN num (%d) or mode (%d)\n", num, mode);
		return pdFAIL; 
	}

	if (mode == 1) {
		if (argc > 4)
			val = simple_strtoul(argv[4], NULL, 10);
		else {
			printf("NO write value parameter\n");
			return pdFAIL;
		}
	}

	if (val)
		val = 1;
	else
		val = 0;

	/*
	   Start to Control GPIO:

	   1. write opt for output mode
	   2. read opt for input mode

*/
	info.dev = dev;
	info.pinumber = num;
	info.mode = mode;
	info.value = val;

	if (mode == 1)
		snx_dev_gpio_write(info);

	if(mode == 0)
	{
		snx_dev_gpio_read(&info);
	}

	printf("dev(%d) , Pin(%d) , Mode(%d) , val(%d)\n",dev, num, mode, info.value);


	return pdPASS;   
}


int cmd_ms2_gpio_ctrl(int argc, char* argv[])
{
	int num = -1,mode = 0, val = 1;
	gpio_pin_info info;

	if (argc < 3) {
		printf(" Usage: ms2_gpio_ctrl [num] [mode] [value]\n");
		printf(" num: 0~7\n");
		printf(" mode:\n");
		printf("      0: Input mode \n");
		printf("      1: Output mode\n");
		printf(" value:\n");
		return pdFAIL;
	}

	num = simple_strtoul(argv[1], NULL, 10);
	mode = simple_strtoul(argv[2], NULL, 10);

	if (num < 0 || num >  7 || mode <0 || mode >1 )
	{
		printf("Wrong PIN num (%d) or mode (%d)\n", num, mode);
		return pdFAIL;
	}

	if (mode == 1) {
		if (argc > 3)
			val = simple_strtoul(argv[3], NULL, 10);
		else {
			printf("NO write value parameter\n");
			return pdFAIL;
		}
	}

	if (val)
		val = 1;
	else
		val = 0;

	/*
		Start to Control GPIO:
		1. enable & OPEN GPIO
		2. config its info structure
		3. write opt for output mode
		4. read opt for input mode
		5. close GPIO
	*/
	snx_ms2_gpio_enable(1);
	snx_ms2_gpio_open();

	info.pinumber = num;
	info.mode = mode;
	info.value = val;

	if (mode == 1)
		snx_ms2_gpio_write(info);

	if(mode == 0)
	{
		snx_ms2_gpio_read(&info);
	}

#if CONFIG_APP_MPTOOL // customer mptool request
	printf("MS2%d %s\n", num, info.value ? "HIGH" : "LOW");
#else
	printf("MS2 Pin(%d), Mode(%d), val(%d)\n", num, mode, info.value);
#endif

	snx_ms2_gpio_close();

	return pdPASS;   
}


#endif
