/*
 * gpio.h
 *
 *  Created on: Apr 20, 2015
 *      Author: spark_lee
 */

#ifndef DRIVER_SN986XX_GPIO_SRC_GPIO_H_
#define DRIVER_SN986XX_GPIO_SRC_GPIO_H_

#define BSP_GPIO_BASE_ADDRESS		(0x98100000)

#define GPIO_O_REG				(0x00)
#define GPIO_I_REG				(0x04)
#define GPIO_OE_REG				(0x08)
#define GPIO_BYP_REG			(0x0c)
#define GPIO_O_SET_REG			(0x10)
#define GPIO_O_CLR_REG			(0x14)
#define GPIO_PULL_EN_REG		(0x18)
#define GPIO_PULL_HIGH_REG		(0x1c)
#define GPIO_INTR_EN_REG		(0x20)
#define GPIO_RAW_FLAG_REG		(0x24)
#define GPIO_INTR_FLAG_REG		(0x28)
#define GPIO_INTR_MSK_REG		(0x2c)
#define GPIO_INTR_CLR_REG		(0x30)
#define GPIO_TRG_MODE_REG		(0x34)
#define GPIO_TRG_EDGE_REG		(0x38)
#define GPIO_TRG_LEVEL_REG		(0x3c)
#define GPIO_DEBOUNCE_EN_REG	(0x40)
#define GPIO_PRESCALE_REG		(0x44)

// UART2 gpio register address
#define UART2_GPIO_CTRL 0x0
#define UART2_GPIO_IO 0x14

#define UART2_GPIO_TX_MODE 1<<0
#define UART2_GPIO_RX_MODE 1<<4
#define UART2_GPIO_TX_I 1<<0
#define UART2_GPIO_TX_O 1<<1
#define UART2_GPIO_TX_OE 1<<2
#define UART2_GPIO_RX_I 1<<4 
#define UART2_GPIO_RX_O 1<<5
#define UART2_GPIO_RX_OE 1<<6  

// JTAG gpio register address
#define JTAG_CTRL  0x1c
typedef union
{
	uint32_t r;
	struct
	{
		uint32_t IMG_RATE:4;
		uint32_t H264_RATE:4;
    uint32_t RESERVED:8;
		uint32_t JTAG_GPIO_O:5;
		uint32_t JTAG_GPIO_OE:5;
		uint32_t JTAG_GPIO_I:5;
		uint32_t JTAG_EN:1;
	}b;
}JTAG_CTRL_t;




#define	GPIO_NO_BYPASS	(0)
#define	GPIO_BYPASS		(1)

#define GPIO_PULL_LOW	(0)
#define GPIO_PULL_HIGH	(1)

#define GPIO_EDGE_TRG	(0)
#define GPIO_LEVEL_TRG	(1)

#define GPIO_SINGLE_TRG	(0)
#define GPIO_BOTH_TRG	(1)

#define GPIO_ACT_HIGH	(0)
#define GPIO_ACT_RISING	(0)
#define GPIO_ACT_LOW	(1)
#define GPIO_ACT_FALL	(1)

#define GPIO_REGION_PASS	(1)
#define GPIO_REGION_FAIL	(0)


void gpio_set_out(uint32_t val);
uint32_t gpio_get_out();
uint32_t gpio_get_in();

void gpio_set_outputen(uint32_t val);
uint32_t gpio_get_outputen();

void gpio_set_bypass(uint32_t val);
uint32_t gpio_get_bypass();

void gpio_set_bit(uint32_t val);
void gpio_clr_bit(uint32_t val);

void gpio_set_pull_en(uint32_t val);
uint32_t gpio_get_pull_en();

void gpio_set_pull_high(uint32_t val);
uint32_t gpio_get_pull_high();

void gpio_set_intr_en(uint32_t val);
uint32_t gpio_get_intr_en();

uint32_t gpio_get_raw_flag();
uint32_t gpio_get_intr_flag();

void gpio_set_intr_mask(uint32_t val);
uint32_t gpio_get_intr_mask();
void gpio_clr_intr_flag(uint32_t val);

void gpio_set_trg_mode(uint32_t val);
uint32_t gpio_get_trg_mode();

void gpio_set_trg_edge(uint32_t val);
uint32_t gpio_get_trg_edge();

void gpio_set_trg_level(uint32_t val);
uint32_t gpio_get_trg_level();

void gpio_set_debounce_en(uint32_t val);
uint32_t gpio_get_debounce_en();

void gpio_set_prescale(uint32_t val);
uint32_t gpio_get_prescale();

/*===== middleware =====*/

typedef struct
{
	uint32_t pin;
	uint32_t outputen;
	uint32_t bypass;
}GPIO_IODIR_SET_t;

typedef struct
{
	uint32_t pin;
	uint32_t en;
	uint32_t pull_high;
}GPIO_PULL_SET_t;

typedef struct
{
	uint32_t pin;
	uint32_t en;
	uint32_t mask;
	uint32_t trg_mode;
	uint32_t trg_edge;
	uint32_t trg_level;
}GPIO_INTR_SET_t;

/***** for Linux API*****/
// function return vaule
#define GPIO_SUCCESS 0
#define GPIO_FAIL 1

typedef struct {
  unsigned int  dev; //0:gpio, 1:pwm, 2:spi, 3: ms1, 4:audio, 5:i2c2, 6:uart2
  unsigned int  pinumber; //pin number
  unsigned int  mode; // 0: input 1: output
  unsigned int  value; // 0:low 1:high
}gpio_pin_info;

//gpio pin
#define GPIO_PIN_0  0
#define GPIO_PIN_1  1
#define GPIO_PIN_2  2
#define GPIO_PIN_3  3
#define GPIO_PIN_4  4  //only sn98610
#define GPIO_PIN_5  5  //only sn98610
#define GPIO_PIN_6  6  //only sn98610

// gpio interrupt mode  only gpio support
#define INTURREPT_NONE      0
#define INTURREPT_RISING    1
#define INTURREPT_FALLING   2
#define INTURREPT_BOTH      3

// pwm gpio pin
#define PWM_GPIO_PIN0 0
#define PWM_GPIO_PIN1 1
#define PWM_GPIO_PIN2 2

//spi gpio pin
#define SPI_GPIO_CLK_PIN  	0
#define SPI_GPIO_FS_PIN		1
#define SPI_GPIO_TX_PIN		2
#define SPI_GPIO_RX_PIN		3


// ms1_gpio pin
#define MS1_GPIO_PIN1 1
#define MS1_GPIO_PIN2 2
#define MS1_GPIO_PIN3 3
#define MS1_GPIO_PIN4 4
#define MS1_GPIO_PIN5 5
#define MS1_GPIO_PIN6 6
#define MS1_GPIO_PIN7 7
#define MS1_GPIO_PIN11 11
#define MS1_GPIO_PIN12 12
#define MS1_GPIO_PIN13 13
#define MS1_GPIO_PIN14 14

// audio gpio pin
#define AUD_GPIO_PIN0 0
#define AUD_GPIO_PIN1 1
#define AUD_GPIO_PIN2 2
#define AUD_GPIO_PIN3 3
#define AUD_GPIO_PIN4 4

// i2c gpio pin
#define I2C1		  0
#define I2C2          1

#define I2C1_GPIO_SCL 0
#define I2C1_GPIO_SDA 1
#define I2C2_GPIO_SCL 2
#define I2C2_GPIO_SDA 3

// uart2 gpio pin
#define UART2_GPIO_PIN_TX 0
#define UART2_GPIO_PIN_RX 1

//jtag gpio pin
#define JTAG_GPIO_PIN_TRSTN 0
#define JTAG_GPIO_PIN_TCK 1
#define JTAG_GPIO_PIN_TMS 2
#define JTAG_GPIO_PIN_TDI 3
#define JTAG_GPIO_PIN_TDO 4


// gpio function
uint32_t snx_gpio_open();
uint32_t snx_gpio_close();
uint32_t snx_gpio_write(gpio_pin_info info);
uint32_t snx_gpio_read(gpio_pin_info* info);
uint32_t snx_gpio_set_interrupt(uint32_t pin, uint32_t type);
uint32_t snx_gpio_poll (uint32_t pin,uint32_t timeout);

// pwm gpio function
uint32_t snx_pwm_gpio_open();
uint32_t snx_pwm_gpio_close ();
uint32_t snx_pwm_gpio_write (gpio_pin_info info);
uint32_t snx_pwm_gpio_read (gpio_pin_info* info);

// spi gpio function
uint32_t snx_spi_gpio_open ();
uint32_t snx_spi_gpio_close ();
uint32_t snx_spi_gpio_write (gpio_pin_info info);
uint32_t snx_spi_gpio_read (gpio_pin_info* info);

// ms1_io gpio function
uint32_t snx_ms1_gpio_open();
uint32_t snx_ms1_gpio_close ();
uint32_t snx_ms1_gpio_write (gpio_pin_info info);
uint32_t snx_ms1_gpio_read (gpio_pin_info* info);
/**********************/

// audio gpio 
uint32_t snx_audio_gpio_open();
uint32_t snx_audio_gpio_close ();
uint32_t snx_audio_gpio_write (gpio_pin_info info);
uint32_t snx_audio_gpio_read (gpio_pin_info* info);

// I2C gpio
uint32_t snx_i2c_gpio_open();
uint32_t snx_i2c_gpio_close ();
uint32_t snx_i2c_gpio_enable(uint32_t instance);
uint32_t snx_i2c_gpio_disable (uint32_t instance);
uint32_t snx_i2c_gpio_write (gpio_pin_info info);
uint32_t snx_i2c_gpio_read (gpio_pin_info* info);


// uart2_gpio function
uint32_t snx_uart2_gpio_open();
uint32_t snx_uart2_gpio_close ();
uint32_t snx_uart2_gpio_write (gpio_pin_info info);
uint32_t snx_uart2_gpio_read (gpio_pin_info* info);

// jtag_gpio function
uint32_t snx_jtag_gpio_enable(uint32_t instance); // 1 :enable 0:disable jtag gpio  
uint32_t snx_jtag_gpio_open();
uint32_t snx_jtag_gpio_close ();
uint32_t snx_jtag_gpio_write (gpio_pin_info info);
uint32_t snx_jtag_gpio_read (gpio_pin_info* info);

// ms2_gpio function
uint32_t snx_ms2_gpio_enable(uint32_t instance); // 1 :enable 0:disable ms2 gpio  
uint32_t snx_ms2_gpio_open();
uint32_t snx_ms2_gpio_close();
uint32_t snx_ms2_gpio_write (gpio_pin_info info);
uint32_t snx_ms2_gpio_read (gpio_pin_info* info);

// total device gpio function
//info dev
#define GPIO_DEV  0
#define PWM_DEV   1
#define SPI_DEV   2
#define MS1_DEV   3
#define AUDIO_DEV 4
#define I2C2_DEV   5
#define UART2_DEV 6
#define JTAG_DEV  7
// info mode
#define GPIO_MODE_INPUT  0
#define GPIO_MODE_OUTPUT 1
//info value
#define GPIO_LOW    0
#define GPIO_HIGH   1
uint32_t snx_dev_gpio_write(gpio_pin_info info);
uint32_t snx_dev_gpio_read(gpio_pin_info* info);
/**********************/


uint32_t gpio_region(uint32_t pin) ;
void gpio_release_region(uint32_t pin) ;
void gpio_out(uint32_t out, uint32_t mask);
uint32_t gpio_in();

void gpio_set_io_pin(GPIO_IODIR_SET_t set);
void gpio_set_pull_pin(GPIO_PULL_SET_t set);
void gpio_set_intr_pin(GPIO_INTR_SET_t set);



#endif /* DRIVER_SN986XX_GPIO_SRC_GPIO_H_ */
