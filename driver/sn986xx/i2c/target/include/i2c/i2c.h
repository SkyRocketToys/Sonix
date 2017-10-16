/** \file i2c.h
 * Functions in this file are show :
 * \n 1.define i2c client and adpater data struct
 * \n 2.define i2c data transfer apis
 * \n 
 * \author Qingbin Li
 * \date   2015-8-31
 */


#ifndef __DRIVER_I2C_H__
#define __DRIVER_I2C_H__

#define I2C_NAME_SIZE		20

#define I2C_STANDARD_CLK	100000
#define I2C_FAST_CLK		400000

#define I2C_CLIENT_TEN	0x10		/**< we have a ten bit chip address		*/


struct i2c_msg {
	unsigned short addr;		/**< slave address				*/
	unsigned short flags;
#define I2C_M_TEN		0x0010	/**< this is a ten bit chip address		*/
#define I2C_M_RD		0x0001	/**< read data, from slave to master		*/
#define I2C_M_NOSTART		0x4000	/**< if I2C_FUNC_PROTOCOL_MANGLING	*/
#define I2C_M_REV_DIR_ADDR	0x2000	/**< if I2C_FUNC_PROTOCOL_MANGLING	*/
#define I2C_M_IGNORE_NAK	0x1000	/**< if I2C_FUNC_PROTOCOL_MANGLING	*/
#define I2C_M_NO_RD_ACK		0x0800	/**< if I2C_FUNC_PROTOCOL_MANGLING	*/
#define I2C_M_RECV_LEN		0x0400	/**< length will be first received byte		*/
	unsigned short len;		/**< msg length				*/
	unsigned char *buf;		/**< pointer to msg data			*/
};

struct i2c_adapter {
	unsigned int id;

	int timeout;			/**< in jiffies					*/
	int retries;

	int nr;
	char name[48];

	int (*master_xfer)(struct i2c_adapter *adap, struct i2c_msg *msgs, int num);
	void *algo_data;

	//struct completion dev_released;
	//struct list_head userspace_clients;
};

struct i2c_client {
	unsigned short flags;		/**< div., see below				*/
	unsigned short addr;		/**< chip address - NOTE: 7bit		*/
					/**< addresses are stored in the		*/
					/**< _LOWER_ 7 bits				*/
	char name[I2C_NAME_SIZE];
	struct i2c_adapter *adapter;	/**< the adapter we sit on			*/
	int irq;			/**< irq issued by device			*/
	//struct list_head detected;
	void *subdev;
};

#define I2C_BOARD_INFO(dev_type, dev_addr) \
	.type = dev_type, .addr = (dev_addr)

struct i2c_board_info{
	char		type[I2C_NAME_SIZE];
	unsigned short	flags;
	unsigned short	addr;
	int (*__init)(struct i2c_client *client, unsigned int mclk);
	int (*__cleanup)(struct i2c_client *client);
};


struct i2c_adapter *i2c_get_adapter(int adpterid);
int i2c_transfer(struct i2c_adapter *adap, struct i2c_msg *msgs, int num);
struct i2c_client *i2c_new_device(struct i2c_adapter *adap, struct i2c_board_info const *info);
int i2c_release_device(struct i2c_client *client);



void i2c_init(void);
void i2c_exit(void);

#endif

