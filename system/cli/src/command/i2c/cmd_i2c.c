#include <FreeRTOS.h>
#include <stdio.h>
#include "cmd_i2c.h"
#include "cmd_debug.h"
#include "printlog.h"
#include <nonstdlib.h>
#include <i2c/i2c.h>
#include <stdlib.h>
#include <string.h>
#include <nonstdlib.h>

#define SNX_I2C_R8D8_MODE	(0)
#define SNX_I2C_R8D16_MODE	(1)
#define SNX_I2C_R16D8_MODE	(2)
#define SNX_I2C_R16D16_MODE	(3)

#define SNX_I2C_OP_WRITE	(0)
#define SNX_I2C_OP_READ		(1)

#define DEBUG 1
#define printf(fmt, args...) if(DEBUG) print_msg_queue((fmt), ##args)

//#define DEBUG

struct i2c_adapter * snx_i2c_open(int dev_num)
{
	struct i2c_adapter * adap;
	if(!(adap = i2c_get_adapter(dev_num))){
		print_msg("Cannot get I2C adapter #%d. No driver?\n", 0);
		return NULL;
	}
	return adap;
}

int snx_i2c_close(struct i2c_adapter * adap)
{
	adap = NULL;
	return 1;
}

int snx_i2c_burst_write(struct i2c_adapter * adap, int chip_addr, int start_addr, int len, void *data, int mode)
{
	struct i2c_msg msgs[1];
	int ret = 0;
	int i;
	unsigned char *val;
	
	if(len <= 0) {
		ret = -1;
		printf("[SNX_I2C] Wrong len (%d)\n", len);
		return ret;
	}

	val = (unsigned char *) malloc(sizeof(unsigned char) * (len + 1) * 2);

	memset(val, 0x0, (sizeof(unsigned char) * (len + 1) * 2));

	msgs[0].addr = chip_addr;
	msgs[0].flags = 0;

	switch (mode)
	{
		case SNX_I2C_R8D8_MODE:
			msgs[0].len = 1 + len;
			msgs[0].buf = val;
			val[0] = start_addr;
			for (i = 0; i < len; i++) {
				unsigned char *new_data = (unsigned char *) data;
				val[i+1] = new_data[i];
			}
			break;

		case SNX_I2C_R8D16_MODE:
			msgs[0].len = 1 + (len * 2);
			msgs[0].buf = val;
			val[0] = start_addr;
			for (i = 0; i < len; i++) {
				unsigned short *new_data = (unsigned short *) data;
				val[i+1] = (unsigned char)(new_data[i] >> 8);
				val[i+2] = (unsigned char)(new_data[i] & 0xff);
			}
			break;

		case SNX_I2C_R16D8_MODE:
			msgs[0].len = 2 + len;
			msgs[0].buf = val;
			val[0] = (unsigned char)(start_addr >> 8);
			val[1] = (unsigned char)(start_addr & 0xff);

			for (i = 0; i < len; i++) {
				unsigned char *new_data = (unsigned char *) data;
				val[i+2] = new_data[i];
			}
			break;

		case SNX_I2C_R16D16_MODE:
			msgs[0].len = 2 + (len * 2);
			msgs[0].buf = val;
			val[0] = (unsigned char)(start_addr >> 8);
			val[1] = (unsigned char)(start_addr & 0xff);
			for (i = 0; i < len; i++) {
				unsigned short *new_data = (unsigned short *) data;
				val[i+1] = (unsigned char)(new_data[i] >> 8);
				val[i+2] = (unsigned char)(new_data[i] & 0xff);
			}
			break;
		default:
			printf("[SNX_I2C] Wrong Mode (%d)\n", mode);
			break;

	}
	

#ifdef DEBUG

	printf("----- Writing Data -------\n\n");
	printf("-- \tchipaddr: 0x%x, reg: 0x%x\n", chip_addr, start_addr);
	for (i = 0; i < msgs[0].len ; i ++) {
		printf("-- \t msgs.data[%d] = 0x%x\n", i, val[i]);
	}
#endif

	if((ret = i2c_transfer(adap, &msgs[0], 1)) == pdFAIL)
	{
		printf("i2c transfer failed return: %d\n", ret);
	}
	

	return ret;
}

int snx_i2c_write(struct i2c_adapter * adap, int chip_addr, int addr, int data, int mode)
{

	int ret = 0;

	ret = snx_i2c_burst_write(adap, chip_addr, addr, 1, &data, mode);

	return ret;
}

int snx_i2c_burst_read(struct i2c_adapter * adap, int chip_addr, int start_addr, int len, void *data, int mode)
{
	struct i2c_msg msgs[2];
	int ret = 0;
	int i;
	unsigned char *val;
	
	if(len <= 0) {
		ret = -1;
		printf("[SNX_I2C] Wrong len (%d)\n", len);
		return ret;
	}

	val = (unsigned char *) malloc(sizeof(unsigned char) * len * 2);

	memset(val, 0x0, (sizeof(unsigned char) * len * 2));

	msgs[0].addr = chip_addr;
	msgs[0].flags = 0;

	switch (mode)
	{
		case SNX_I2C_R8D8_MODE:
		case SNX_I2C_R8D16_MODE:
			msgs[0].len = 1;
			msgs[0].buf = val;
			val[0] = start_addr;
			break;

		case SNX_I2C_R16D8_MODE:
		case SNX_I2C_R16D16_MODE:
			msgs[0].len = 2;
			msgs[0].buf = val;
			val[0] = (unsigned char)(start_addr >> 8);
			val[1] = (unsigned char)(start_addr & 0xff);
			break;
		default:
			printf("[SNX_I2C] Wrong Mode (%d)\n", mode);
			break;

	}
	
	msgs[1].addr = chip_addr;
	msgs[1].flags = I2C_M_RD;

	switch (mode)
	{
		case SNX_I2C_R8D8_MODE:
		case SNX_I2C_R16D8_MODE:
		
			msgs[1].len = len;
			msgs[1].buf = val;
			break;

		case SNX_I2C_R8D16_MODE:
		case SNX_I2C_R16D16_MODE:

			msgs[1].len = len * 2;
			msgs[1].buf = val;
			break;
		default:
			printf("[SNX_I2C] Wrong Mode (%d)\n", mode);
			break;

	}

	
	if((ret = i2c_transfer(adap, msgs, 2)) == pdFAIL)
		return ret;
/*
	if((ret = i2c_transfer(adap, &msgs[1], 1)) == pdFAIL)
		return ret;
*/
	for (i = 0; i < len; i++) {
		switch (mode)
		{
			case SNX_I2C_R8D8_MODE:
			case SNX_I2C_R16D8_MODE:
			
				{
					unsigned char *new_data = (unsigned char *) data;
				
					new_data[i] = (unsigned char)val[i];

    			}
				break;

			case SNX_I2C_R8D16_MODE:
			case SNX_I2C_R16D16_MODE:
				{
					unsigned short *new_data = (unsigned short *) data;
				
					new_data[i] = (unsigned short)val[i*2];
	    			new_data[i] = (new_data[i]  << 8 )| (unsigned short)val[ (i*2 + 1)];
    			}
				break;
			default:
				printf("[SNX_I2C] Wrong Mode (%d)\n", mode);
				break;

		}

	}
#ifdef DEBUG
	printf("----- reading Data -------\n\n");
	printf("-- \tchipaddr: 0x%x, reg: 0x%x\n", chip_addr, start_addr);
	for (i = 0; i < msgs[1].len ; i ++) {
		printf("-- \t msgs.data[%d] = 0x%x\n", i, val[i]);
	}
#endif
	return ret;
}

int snx_i2c_read(struct i2c_adapter * adap, int chip_addr, int addr, int mode)
{
	int value = 0;

	snx_i2c_burst_read(adap, chip_addr, addr, 1, &value, mode);

	return value;
}

static void i2c_ctrl_usage() {   
	printf(" Usage: i2c_ctrl [Device] [OPT] [SlaveAddr] [Mode] [Reg] [Value]\n");
	printf(" Device: 0, 1\n");
	printf(" OPT:\n");
	printf("      0: Write opt \n");
	printf("      1: Read opt\n");
	printf(" SlaveAddr: Device slave address (8/12bits)\n");
	printf(" Mode:\n");
	printf("      0: SNX_I2C_R8D8_MODE \n");
	printf("      1: SNX_I2C_R8D16_MODE \n");
	printf("      2: SNX_I2C_R16D8_MODE \n");
	printf("      3: SNX_I2C_R16D16_MODE \n");
	printf(" Reg: register address \n");
	printf(" Value: register value\n");
	printf(" Example: \n");
	printf("      i2c_ctrl 0 1 0x60 0 0x01 \n");
	printf("      i2c_ctrl 0 0 0x60 0 0x01 0x23\n");
}  

int cmd_i2c_ctrl(int argc, char* argv[])
{                          

	int reg = -1; 
	int value = -1; 
	int addr = -1;
	int status = -1;
	struct i2c_adapter * adap = NULL;
	int device = 0;
	int mode = SNX_I2C_R8D8_MODE;
	int len = 1;
	int i;

	if (argc < 6) {
		i2c_ctrl_usage();
		return pdFAIL;
	}

	device = simple_strtoul(argv[1], NULL, 10);
	if ((device < 0) || (device > 1))
	{
		printf("Wrong device number (%d)\n", device);
		return pdFAIL;
	}

	status = simple_strtoul(argv[2], NULL, 10);
	if ((status < 0) || (status > 1))
	{
		printf("Wrong device opt (%d)\n", status);
		return pdFAIL;
	}

	addr = simple_strtoul(argv[3], NULL, 0);
	mode = simple_strtoul(argv[4], NULL, 10);
	if ((mode < SNX_I2C_R8D8_MODE) || (mode > SNX_I2C_R16D16_MODE) ) {
		printf("[SNX_I2C] Wrong MODE (%d)\n", mode);
		return pdFAIL;
	}

	reg = simple_strtoul(argv[5], NULL, 0);

	if (status == SNX_I2C_OP_WRITE) {
		if (argc > 6)
			value = simple_strtoul(argv[6], NULL, 0);
		else {
			printf("NO write value parameter\n");
			return pdFAIL;
		}
	}

	adap = snx_i2c_open(device);

  	printf("I2C device (%d) is open\n",device);
  
	if(adap) {
	
		if (status == SNX_I2C_OP_WRITE) {
			if (addr == -1 || reg == -1 || value == -1)
			{
				i2c_ctrl_usage();   
				goto EXIT;  
			}

			snx_i2c_write(adap, addr, reg, value, mode);
			printf("addr: %x set 0x%x = 0x%x\n", addr, reg, value);
			goto EXIT;

		} else if (status == SNX_I2C_OP_READ) {
			if (addr == -1 || reg == -1)
			{
				i2c_ctrl_usage();  
				goto EXIT;  
			}

			if (len == 1) 
				printf("addr: 0x%x get 0x%x = 0x%x\n", addr, reg, snx_i2c_read(adap, addr, reg, mode));
			else {
				
				{
					void *value;
					switch(mode) {
						case SNX_I2C_R8D8_MODE:
						case SNX_I2C_R16D8_MODE:
							value = (unsigned char*) malloc(sizeof(unsigned char)*len);
							break;
						case SNX_I2C_R16D16_MODE:
						case SNX_I2C_R8D16_MODE:
							value = (unsigned short*) malloc(sizeof(unsigned short)*len);
							break;
					}
					snx_i2c_burst_read(adap, addr, reg, len, value, mode);
					printf("addr: 0x%x start address: 0x%x len: %d\n", addr, reg, len);
					printf("data: \t");

					for(i = 0; i < len; i++) {

						switch(mode) {
						case SNX_I2C_R8D8_MODE:
						case SNX_I2C_R16D8_MODE:
							printf("0x%x\n", ((unsigned char*)(value))[i]);
							break;
						case SNX_I2C_R16D16_MODE:
						case SNX_I2C_R8D16_MODE:
							printf("0x%x\n", ((unsigned short*)(value))[i]);
							break;
						}
						
					}
					free(value);
				}
				
			}
		} else
			i2c_ctrl_usage();     
        
EXIT:
		snx_i2c_close(adap);
	}
	return pdPASS;   
}
