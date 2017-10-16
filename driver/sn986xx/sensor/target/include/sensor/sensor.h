#ifndef __DRIVER_SENSOR_H__
#define __DRIVER_SENSOR_H__

#include <i2c/i2c.h>

#define SENSOR_ID			0x1000		/*!< ctrl id for sensor id */
#define SENSOR_DMY_PX 			0x1001		/*!< ctrl id for sensor dummy pixel */
#define SENSOR_DMY_LN 			0x1002		/*!< ctrl id for sensor dummy line */
#define SENSOR_PX_HZ 			0x1003		/*!< ctrl id for sensor pixel hz */
#define SENSOR_PX_CLK			0x1004		/*!< ctrl id for sensor pixel clock */
#define SENSOR_PX_PER_LINE		0x1005		/*!< ctrl id for sensor pixel per line */
#define SENSOR_LINE			0x1006		/*!< ctrl id for sensor line */
#define SENSOR_GROUP_LATCH		0x1007		/*!< ctrl id for sensor group latch*/

#define SENSOR_AEC			0x1100		/*!< ctrl id for sensor AEC */
#define SENSOR_AEC_EXP			0x1101		/*!< ctrl id for sensor exposure line */
#define SENSOR_AEC_GAIN			0x1102		/*!< ctrl id for sensor gain */
#define SENSOR_AEC_LIGHT_HZ		0x1103		/*!< ctrl id for sensor light hz */
#define SENSOR_SET_RESERVE1		0x1104		/*!< ctrl id for sensor reserve */
#define SENSOR_AEC_MAX_EXPL		0x1105		/*!< ctrl id for sensor max exposure line */

#define SENSOR_AWB			0x1200		/*!< ctrl id for sensor AWB */
#define SENSOR_AWB_B_GAIN		0x1201		/*!< ctrl id for sensor B gain */
#define SENSOR_AWB_G_GAIN		0x1202		/*!< ctrl id for sensor G gain */
#define SENSOR_AWB_R_GAIN		0x1203		/*!< ctrl id for sensor R gain */

#define SENSOR_MIRROR			0x1300		/*!< ctrl id for sensor mirror */
#define SENSOR_FLIP			0x1301		/*!< ctrl id for sensor flip */

#define SENSOR_MCLK			0x1400		/*!< ctrl id for sensor mclk */

#define SENSOR_FPS			0x1500		/*!< ctrl id for sensor fps */

#define SENSOR_MAX_FPS			0x1600		/*!< ctrl id for sensor max fps */
#define SENSOR_ONOFF			0x1700
#define SENSOR_I2C			0x1800

#define MAX_SENSOR_FPS			30

#define SENSOR_SUBDEV_NAME_SIZE		32

struct sensor_subdev {
	char name[SENSOR_SUBDEV_NAME_SIZE];
	unsigned int flags;
	const struct sensor_subdev_ops *ops;
	/* pointer to private data */
	void *dev_priv;
	void *host_priv;
};

struct sensor_subdev_ops {
	int (*g_chip_ident)(struct sensor_subdev *sd, int *ident);
	int (*init)(struct sensor_subdev *sd, unsigned int val);
	int (*reset)(struct sensor_subdev *sd, unsigned int val);
	int (*g_ctrl)(struct sensor_subdev *sd, unsigned int id, int *val);
	int (*s_ctrl)(struct sensor_subdev *sd, unsigned int id, int val);

	int (*e_mode)(struct sensor_subdev *sd, int index, void *fival, void *fsize);
	int (*g_mode)(struct sensor_subdev *sd, int *width, int *height, int *rate);
	int (*s_mode)(struct sensor_subdev *sd, int width, int height, int rate);

	int (*g_register)(struct sensor_subdev *sd, unsigned int size, unsigned int reg, unsigned int *val);
	int (*s_register)(struct sensor_subdev *sd, unsigned int size, unsigned int reg, unsigned int val);
	int (*s_stream)(struct sensor_subdev *sd, int enable);
	int (*g_crop)(struct sensor_subdev *sd, int *left, int *top, int *width, int *height);
	int (*s_crop)(struct sensor_subdev *sd, int left, int top, int width, int height);
	int (*g_parm)(struct sensor_subdev *sd, int *rate);
	int (*s_parm)(struct sensor_subdev *sd, int rate);
};

struct _sensor{
	int id;
	int mipi;
	int yuv;
	unsigned int reorder;
	unsigned int delay_cell_sel;	//delay cell selection
	unsigned int pd_time_sel;	//power down timing selection
	int fps_ctrl;
	int ae_sequence_timing; // 0: ae do at hw_end isr; 1: ae do at hsync isr

	int vsync_ris;
	int hsync_ris;
	int vsync_high;
	
	unsigned int dummy_insert;
	unsigned int dummy_div;
	unsigned int dummy_line_num;
	unsigned int dummy_pixle_num;

	struct i2c_board_info *bdi;
	struct sensor_subdev *sd;	
};

#define sensor_subdev_call(sd, f, args...)				\
	(!(sd) ? pdFAIL : (((sd)->ops->f) ?	\
		(sd)->ops->f((sd) , ##args) : pdFAIL))


int snx_sensor_scan_callback(char *name, unsigned long mclk, struct _sensor *sensor);
void snx_sensor_free_callback(struct _sensor *sensor);


#endif
