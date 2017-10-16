#include <FreeRTOS.h>
#include <bsp.h>
#include <task.h>
#include <i2c/i2c.h>
#include <sensor/sensor.h>
#include <cmd_verify.h>
#include <nonstdlib.h>

extern int ov9715_module_init(struct i2c_client *client, unsigned int mclk);
extern int ov9715_module_exit(struct i2c_client *client);

static struct i2c_board_info snx_i2c0_devs[] = {
	{
		I2C_BOARD_INFO("ov9715", 0x60),
		.__init = ov9715_module_init,
		.__cleanup = ov9715_module_exit,
	},
};

int cmd_verify_i2c_rdid(int argc, char* argv[])
{
	int ret;
	struct i2c_client *client;
	struct i2c_adapter *adap;
	struct i2c_board_info *bdi = &snx_i2c0_devs[0];
	unsigned int v, mclk, div, base;

	//setup clock divisor & enable mclk
	mclk = 24000000;
	base = 0x90600000;

	v = inl(base);

	if (v & (0x1<<31))
		div = 120000000 / mclk;
	else
		div = 96000000 / mclk;

	v &= ~(0xff<<8|0x1);
	v |= (div<<8|0x1);
	outl(base, v);

	vTaskDelay(50 / portTICK_PERIOD_MS); //50ms

	if(!(adap = i2c_get_adapter(0))){
		print_msg_queue("Cannot get I2C adapter #%d. No driver?\n", 0);
		return pdFAIL;
	}

	if(!(client = i2c_new_device(adap, bdi))){
		return pdFAIL;
	}

	if((ret = bdi->__init(client, 24000000)) == pdFAIL)
		goto out;


	bdi->__cleanup(client);

out:
	i2c_release_device(client);

	return 0;
}
