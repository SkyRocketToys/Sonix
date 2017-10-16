/*
 * gpio.c
 *
 *  Created on: Apr 20, 2015
 *      Author: spark_lee
 */


#include "FreeRTOS.h"

#include <bsp.h>
#include <gpio.h>

#include <spi/spi.h>
#include <pwm/pwm.h>
#include <sf/sf.h>
#include <sd/sdv2.h> //add ms2_io gpio
#include "semphr.h"
#include "task.h"

void gpio_set_out(uint32_t val)
{
	outl((BSP_GPIO_BASE_ADDRESS+GPIO_O_REG), val);
}

uint32_t gpio_get_out()
{
	return inl((BSP_GPIO_BASE_ADDRESS+GPIO_O_REG));
}

uint32_t gpio_get_in()
{
	return inl((BSP_GPIO_BASE_ADDRESS+GPIO_I_REG));
}


void gpio_set_outputen(uint32_t val)
{
	outl((BSP_GPIO_BASE_ADDRESS+GPIO_OE_REG), val);
}
uint32_t gpio_get_outputen()
{
	return inl((BSP_GPIO_BASE_ADDRESS+GPIO_OE_REG));
}


void gpio_set_bypass(uint32_t val)
{
	outl((BSP_GPIO_BASE_ADDRESS+GPIO_BYP_REG), val);
}
uint32_t gpio_get_bypass()
{
	return inl((BSP_GPIO_BASE_ADDRESS+GPIO_BYP_REG));
}


void gpio_set_bit(uint32_t val)
{
	outl((BSP_GPIO_BASE_ADDRESS+GPIO_O_SET_REG), val);
}
void gpio_clr_bit(uint32_t val)
{
	outl((BSP_GPIO_BASE_ADDRESS+GPIO_O_CLR_REG), val);
}


void gpio_set_pull_en(uint32_t val)
{
	outl((BSP_GPIO_BASE_ADDRESS+GPIO_PULL_EN_REG), val);
}
uint32_t gpio_get_pull_en()
{
	return inl((BSP_GPIO_BASE_ADDRESS+GPIO_PULL_EN_REG));
}

void gpio_set_pull_high(uint32_t val)
{
	outl((BSP_GPIO_BASE_ADDRESS+GPIO_PULL_HIGH_REG), val);
}
uint32_t gpio_get_pull_high()
{
	return inl((BSP_GPIO_BASE_ADDRESS+GPIO_PULL_HIGH_REG));
}

void gpio_set_intr_en(uint32_t val)
{
	outl((BSP_GPIO_BASE_ADDRESS+GPIO_INTR_EN_REG), val);
}
uint32_t gpio_get_intr_en()
{
	return inl((BSP_GPIO_BASE_ADDRESS+GPIO_INTR_EN_REG));
}

uint32_t gpio_get_raw_flag()
{
	return inl((BSP_GPIO_BASE_ADDRESS+GPIO_RAW_FLAG_REG));
}

uint32_t gpio_get_intr_flag()
{
	return inl((BSP_GPIO_BASE_ADDRESS+GPIO_INTR_FLAG_REG));
}

void gpio_set_intr_mask(uint32_t val)
{
	outl((BSP_GPIO_BASE_ADDRESS+GPIO_INTR_MSK_REG), val);
}
uint32_t gpio_get_intr_mask()
{
	return inl((BSP_GPIO_BASE_ADDRESS+GPIO_INTR_MSK_REG));
}

void gpio_clr_intr_flag(uint32_t val)
{
	outl((BSP_GPIO_BASE_ADDRESS+GPIO_INTR_CLR_REG), val);
}

void gpio_set_trg_mode(uint32_t val)
{
	outl((BSP_GPIO_BASE_ADDRESS+GPIO_TRG_MODE_REG), val);
}
uint32_t gpio_get_trg_mode()
{
	return inl((BSP_GPIO_BASE_ADDRESS+GPIO_TRG_MODE_REG));
}


void gpio_set_trg_edge(uint32_t val)
{
	outl((BSP_GPIO_BASE_ADDRESS+GPIO_TRG_EDGE_REG), val);
}
uint32_t gpio_get_trg_edge()
{
	return inl((BSP_GPIO_BASE_ADDRESS+GPIO_TRG_EDGE_REG));
}


void gpio_set_trg_level(uint32_t val)
{
	outl((BSP_GPIO_BASE_ADDRESS+GPIO_TRG_LEVEL_REG), val);
}
uint32_t gpio_get_trg_level()
{
	return inl((BSP_GPIO_BASE_ADDRESS+GPIO_TRG_LEVEL_REG));
}


void gpio_set_debounce_en(uint32_t val)
{
	outl((BSP_GPIO_BASE_ADDRESS+GPIO_DEBOUNCE_EN_REG), val);
}
uint32_t gpio_get_debounce_en()
{
	return inl((BSP_GPIO_BASE_ADDRESS+GPIO_DEBOUNCE_EN_REG));
}


void gpio_set_prescale(uint32_t val)
{
	outl((BSP_GPIO_BASE_ADDRESS+GPIO_PRESCALE_REG), val);
}
uint32_t gpio_get_prescale()
{
	return inl((BSP_GPIO_BASE_ADDRESS+GPIO_PRESCALE_REG));
}


/********************/
static uint32_t gpio_used = 0;



uint32_t gpio_region(uint32_t pin)
{
	uint32_t stat;

	taskENTER_CRITICAL();

	if ((gpio_used & pin) == 0) {
		gpio_used |= pin;
		stat = GPIO_REGION_PASS;
	} else {
		stat = GPIO_REGION_FAIL;
	}

	taskEXIT_CRITICAL();

	return stat;
}


void gpio_release_region(uint32_t pin)
{
	taskENTER_CRITICAL();

	gpio_used &= ~(pin);

	taskEXIT_CRITICAL();

}


void gpio_out(uint32_t out, uint32_t mask)
{
	uint32_t reg;

	taskENTER_CRITICAL();

	reg = gpio_get_out();
	reg &= ~(mask);
	reg |= out;
	gpio_set_out(reg);

	taskEXIT_CRITICAL();
}

uint32_t gpio_in()
{
	return gpio_get_in();
}

void gpio_set_io_pin(GPIO_IODIR_SET_t set)
{
	uint32_t reg;

	taskENTER_CRITICAL();

	reg = gpio_get_outputen();
	reg &= ~(set.pin);
	reg |= (set.outputen);
	gpio_set_outputen(reg);

	reg = gpio_get_bypass();
	reg &= ~(set.pin);
	reg |= (set.bypass);
	gpio_set_bypass(reg);


	taskEXIT_CRITICAL();
}


void gpio_set_pull_pin(GPIO_PULL_SET_t set)
{
	uint32_t reg_en, reg_high;

	taskENTER_CRITICAL();

	reg_high = gpio_get_pull_high();
	reg_high &= ~(set.pin);
	reg_high |= set.pull_high;

	reg_en = gpio_get_pull_en();
	reg_en &= ~(set.pin);
	reg_en |= set.en;

	gpio_set_pull_high(reg_high);
	gpio_set_pull_en(reg_en);

	taskEXIT_CRITICAL();
}




void gpio_set_intr_pin(GPIO_INTR_SET_t set)
{
	uint32_t reg;

	taskENTER_CRITICAL();

	reg = gpio_get_intr_mask();
	reg &= ~(set.pin);
	reg |= (set.mask);
	gpio_set_intr_mask(reg);

	reg = gpio_get_trg_mode();
	reg &= ~(set.pin);
	reg |= (set.trg_mode);
	gpio_set_trg_mode(reg);

	reg = gpio_get_trg_edge();
	reg &= ~(set.pin);
	reg |= (set.trg_edge);
	gpio_set_trg_edge(reg);

	reg = gpio_get_trg_level();
	reg &= ~(set.pin);
	reg |= (set.trg_level);
	gpio_set_trg_level(reg);

	reg = gpio_get_intr_en();
	reg &= ~(set.pin);
	reg |= (set.en);
	gpio_set_intr_en(reg);

	taskEXIT_CRITICAL();
}


/*************** for Linux API *****************/

// gpio function
uint32_t snx_gpio_open()
{
	return GPIO_SUCCESS;
}
uint32_t snx_gpio_close()
{
	return GPIO_SUCCESS;
}
uint32_t snx_gpio_write(gpio_pin_info info)
{
	uint32_t pinbit = (1<<info.pinumber);
	uint32_t reg;

	taskENTER_CRITICAL();

	/*** set in/out ***/
	reg = gpio_get_outputen();
	reg &= ~(pinbit);
	reg |= (pinbit);
	gpio_set_outputen(reg);

	reg = gpio_get_bypass();
	reg &= ~(pinbit);
	reg |= (0 << info.pinumber);
	gpio_set_bypass(reg);

	reg = gpio_get_out();
	reg &= ~(pinbit);
	reg |= (info.value<<info.pinumber);
	gpio_set_out(reg);

	taskEXIT_CRITICAL();

	return GPIO_SUCCESS;
}
uint32_t snx_gpio_read(gpio_pin_info* info)
{
	uint32_t pinbit = (1<<info->pinumber);
	uint32_t reg;
	uint32_t io;

	taskENTER_CRITICAL();

	/*** set in/out ***/
	reg = gpio_get_outputen();
	reg &= ~(pinbit);
	gpio_set_outputen(reg);
	
	reg = gpio_get_bypass();
	reg &= ~(pinbit);
	reg |= (0 << info->pinumber);
	gpio_set_bypass(reg);

	io = gpio_get_in();
	info->value = (io>>(info->pinumber))&0x1;

	taskEXIT_CRITICAL();

	return GPIO_SUCCESS;
}
uint32_t snx_gpio_set_interrupt(uint32_t pin, uint32_t type)
{
	uint32_t reg;
	uint32_t pinbit = (1<<pin);


	taskENTER_CRITICAL();
#if 0  
  
  gpio_set_debounce_en (pin);
  print_msg_queue ("set debounce_en pin\n");
#endif
	reg = gpio_get_trg_edge();

	if(type == INTURREPT_BOTH){
		reg |= (pinbit);//1:both
	}
	else{
		reg &= ~(pinbit);//0:single
	}

	gpio_set_trg_edge(reg);

	reg = gpio_get_trg_level();

	if(type == INTURREPT_FALLING){
		reg |= (pinbit);//1:falling
	}
	else{
		reg &= ~(pinbit);//0:rising
	}

	gpio_set_trg_level(reg);

	reg = gpio_get_intr_en();
	reg &= ~(pinbit);
	if(type != INTURREPT_NONE){
		reg |= (pinbit);
	}
	gpio_set_intr_en(reg);

	taskEXIT_CRITICAL();

	return GPIO_SUCCESS;
}


uint32_t snx_gpio_poll (uint32_t pin,uint32_t timeout)
{
	system_bus_freq_t *clk;
	uint32_t apbclk;
	volatile uint32_t c;
	uint32_t pinbit = (1<<pin);
  c = timeout;
//	clk = clk_get();
//	apbclk = clk->fapb;

//	c = (apbclk/1000000)*timeout;
	while(c>0){
		if((gpio_get_raw_flag () &pinbit)!= 0){
     // print_msg_queue ("GPIO_SUCCESS\n");
#if 1  //clear interrupt
      gpio_clr_intr_flag (1<<pin);
#endif     
			return GPIO_SUCCESS;
      
		}
    cpu_udelay (1000); // unit 1 ms
		c--;
	}
  //print_msg_queue ("GPIO_FAIL\n");
#if 1  //clear interrupt
  gpio_clr_intr_flag (1<<pin);
#endif  
	return GPIO_FAIL;
}

// pwm gpio function
uint32_t snx_pwm_gpio_open()
{
	return GPIO_SUCCESS;
}
uint32_t snx_pwm_gpio_close ()
{
	return GPIO_SUCCESS;
}
uint32_t snx_pwm_gpio_write (gpio_pin_info info)
{
	uint32_t base;

	switch(info.pinumber)
	{
	case PWM_GPIO_PIN0:
		base = PWM1;
		break;
	case PWM_GPIO_PIN1:
		base = PWM2;
		break;
	case PWM_GPIO_PIN2:
		base = PWM3;
		break;

	}

	taskENTER_CRITICAL();
	pwm_enable(base, 0);
	pwm_gpio_outputen(base, 1);
	pwm_gpio_out(base, info.value);
	taskEXIT_CRITICAL();

	return GPIO_SUCCESS;
}
uint32_t snx_pwm_gpio_read (gpio_pin_info* info)
{
	uint32_t base;

	switch(info->pinumber)
	{
	case PWM_GPIO_PIN0:
		base = PWM1;
		break;
	case PWM_GPIO_PIN1:
		base = PWM2;
		break;
	case PWM_GPIO_PIN2:
		base = PWM3;
		break;

	}

	pwm_enable(base, 0);
	pwm_gpio_outputen(base, 0);
	info->value = pwm_gpio_in(base);

	return GPIO_SUCCESS;
}

// spi gpio function
uint32_t snx_spi_gpio_open ()
{
	return GPIO_SUCCESS;
}
uint32_t snx_spi_gpio_close ()
{
	return GPIO_SUCCESS;
}
uint32_t snx_spi_gpio_write (gpio_pin_info info)
{
	SPI_CTRL_t ctrl;

	taskENTER_CRITICAL();
	ctrl.r = inl((BSP_SPI_BASE_ADDRESS+SPI_CTRL_REG));
  ctrl.b.SSP_GPIO_MODE = 1; //set spi to gpio mode
	switch(info.pinumber)
	{
	case SPI_GPIO_CLK_PIN:
		ctrl.b.SSP_CLK_GPIO_OE = 1;
		ctrl.b.SSP_CLK_GPIO_O = info.value;
		break;
	case SPI_GPIO_FS_PIN:
		ctrl.b.SSP_FS_GPIO_OE = 1;
		ctrl.b.SSP_FS_GPIO_O = info.value;
		break;
	case SPI_GPIO_TX_PIN:
		ctrl.b.SSP_TX_GPIO_OE = 1;
		ctrl.b.SSP_TX_GPIO_O = info.value;
		break;
	case SPI_GPIO_RX_PIN:
		ctrl.b.SSP_RX_GPIO_OE = 1;
		ctrl.b.SSP_RX_GPIO_O = info.value;
		break;

	}

	outl((BSP_SPI_BASE_ADDRESS+SPI_CTRL_REG), ctrl.r);
	taskEXIT_CRITICAL();

	return GPIO_SUCCESS;
}
uint32_t snx_spi_gpio_read (gpio_pin_info* info)
{
	SPI_CTRL_t ctrl;

	taskENTER_CRITICAL();
	ctrl.r = inl((BSP_SPI_BASE_ADDRESS+SPI_CTRL_REG)); 
  ctrl.b.SSP_GPIO_MODE = 1;
  ctrl.b.SSP_CLK_GPIO_OE = 0;
  outl((BSP_SPI_BASE_ADDRESS+SPI_CTRL_REG), ctrl.r);
  ctrl.r = inl((BSP_SPI_BASE_ADDRESS+SPI_CTRL_REG));
	switch (info->pinumber) {
	case SPI_GPIO_CLK_PIN:
		ctrl.b.SSP_CLK_GPIO_OE = 0;
		info->value = ctrl.b.SSP_CLK_GPIO_I;
		break;
	case SPI_GPIO_FS_PIN:
		ctrl.b.SSP_FS_GPIO_OE = 0;
		info->value = ctrl.b.SSP_FS_GPIO_I;
		break;
	case SPI_GPIO_TX_PIN:
		ctrl.b.SSP_TX_GPIO_OE = 0;
		info->value = ctrl.b.SSP_TX_GPIO_I;
		break;
	case SPI_GPIO_RX_PIN:
		ctrl.b.SSP_RX_GPIO_OE = 0;
		info->value = ctrl.b.SSP_RX_GPIO_I;
		break;

	}

	taskEXIT_CRITICAL();

	return GPIO_SUCCESS;
}

/*** MS1 GPIO ***/
uint32_t snx_ms1_gpio_open()
{
	uint32_t ctrl;
	
	taskENTER_CRITICAL();
	ctrl = inl(SF_CTL);
	ctrl &= ~(0x00000007);
	outl(SF_CTL, ctrl);
	taskEXIT_CRITICAL();

	return GPIO_SUCCESS;
}

uint32_t snx_ms1_gpio_close ()
{
	uint32_t ctrl;

	taskENTER_CRITICAL();
	ctrl = inl(SF_CTL);
	ctrl &= ~(0x00000007);
	ctrl |= (0x00000003);
	outl(SF_CTL, ctrl);
	taskEXIT_CRITICAL();

	return GPIO_SUCCESS;
}

uint32_t snx_ms1_gpio_write (gpio_pin_info info)
{
	uint32_t out;
	uint32_t out_en;

	taskENTER_CRITICAL();
	out = inl(SF_MS_IO_O);
	out_en = inl(SF_MS_IO_OE);
	
	out_en &= ~(1 << info.pinumber);
	out_en |= (info.mode << info.pinumber);
	
	out &= ~(1 << info.pinumber);
	out |= (info.value << info.pinumber);

	outl(SF_MS_IO_OE, out_en);
	outl(SF_MS_IO_O, out);
	taskEXIT_CRITICAL();

	return GPIO_SUCCESS;
}

uint32_t snx_ms1_gpio_read (gpio_pin_info* info)
{
	uint32_t in_value;
	uint32_t out_en;

	taskENTER_CRITICAL();

	out_en = inl(SF_MS_IO_OE);
	out_en &= ~(1 << info->pinumber);
	out_en |= (info->mode << info->pinumber);
	outl(SF_MS_IO_OE, out_en);
	
	in_value = inl(SF_MS_IO_I);

	info->value = (in_value >> info->pinumber) & 0x01;
	
	taskEXIT_CRITICAL();

	return GPIO_SUCCESS;
}


/************* Audio GPIO ***************/
#define AUD_FORMAT                              (BSP_AUDIO_BASE_ADDRESS + 0x4)
#define AUD_I2S_GPIO                            (BSP_AUDIO_BASE_ADDRESS + 0xBC)
#define AUD_IO_I_BIT                            0
#define AUD_IO_O_BIT                            8
#define AUD_IO_OEN_BIT                          16
uint32_t g_aud_format = 0;

uint32_t snx_audio_gpio_open()
{
	uint32_t format;

	taskENTER_CRITICAL();
	format = inl(AUD_FORMAT);
	g_aud_format = format;
	format &= ~(0x0000000F);
	format |= (0x0000000A);
	outl(AUD_FORMAT, format);
	taskEXIT_CRITICAL();

	return GPIO_SUCCESS;
}

uint32_t snx_audio_gpio_close ()
{
	taskENTER_CRITICAL();
	outl(AUD_FORMAT, g_aud_format);
	taskEXIT_CRITICAL();

	return GPIO_SUCCESS;
}
uint32_t snx_audio_gpio_write (gpio_pin_info info)
{
	uint32_t i2s_gpio;

	taskENTER_CRITICAL();
	i2s_gpio = inl(AUD_I2S_GPIO);

	i2s_gpio |= 1 << (info.pinumber + AUD_IO_OEN_BIT);

	i2s_gpio &= ~(1 << (info.pinumber + AUD_IO_O_BIT));
	i2s_gpio |= info.value << (info.pinumber + AUD_IO_O_BIT);

	outl(AUD_I2S_GPIO, i2s_gpio);
	taskEXIT_CRITICAL();

	return GPIO_SUCCESS;
}
uint32_t snx_audio_gpio_read (gpio_pin_info* info)
{
	uint32_t i2s_gpio;

	taskENTER_CRITICAL();
	i2s_gpio = inl(AUD_I2S_GPIO);
	i2s_gpio &= ~(1 << (info->pinumber + AUD_IO_OEN_BIT));
	outl(AUD_I2S_GPIO, i2s_gpio);

	i2s_gpio = inl(AUD_I2S_GPIO);
	info->value = (i2s_gpio >> (info->pinumber + AUD_IO_I_BIT)) & 0x01;
	taskEXIT_CRITICAL();

	return GPIO_SUCCESS;
}
	
/********** I2C GPIO ***********/
#define I2C1_MODE                              (BSP_I2C_BASE_ADDRESS(0) + 0x1C)
#define I2C2_MODE                              (BSP_I2C_BASE_ADDRESS(1) + 0x1C)
#define I2C_IO_I_BIT                            6
#define I2C_IO_O_BIT                            4
#define I2C_IO_OEN_BIT                          2

uint32_t snx_i2c_gpio_open()
{
	return GPIO_SUCCESS;
}
uint32_t snx_i2c_gpio_close ()
{
	return GPIO_SUCCESS;
}

uint32_t snx_i2c_gpio_enable(uint32_t instance)
{

	uint32_t mode;
	uint32_t base;

	switch(instance)
	{
		case I2C1:
			base = I2C1_MODE;
			break;
		case I2C2:
			base = I2C2_MODE;
			break;

		default:
			return GPIO_FAIL;;
	}

	taskENTER_CRITICAL();
	mode = inl(base);
	mode &= ~(0x00000003);
	mode |= (0x00000002);
	outl(base, mode);
	taskEXIT_CRITICAL();

	return GPIO_SUCCESS;
}

uint32_t snx_i2c_gpio_disable (uint32_t instance)
{
	uint32_t mode;
	uint32_t base;

	switch(instance)
	{
		case I2C1:
			base = I2C1_MODE;
			break;
		case I2C2:
			base = I2C2_MODE;
			break;

		default:
			return GPIO_FAIL;;
	}

	taskENTER_CRITICAL();
	mode = inl(base);
	mode &= ~(0x00000003);
	outl(base, mode);
	taskEXIT_CRITICAL();

	return GPIO_SUCCESS;
}

uint32_t snx_i2c_gpio_write (gpio_pin_info info)
{

	uint32_t base;
	uint32_t i2c_gpio;
	uint32_t offset;

	switch(info.pinumber)
	{
		case I2C1_GPIO_SCL:
			base = I2C1_MODE;
			offset = 0;
			break;
		case I2C1_GPIO_SDA:
			base = I2C1_MODE;
			offset = 1;
			break;
		case I2C2_GPIO_SCL:
			base = I2C2_MODE;
			offset = 0;
			break;
		case I2C2_GPIO_SDA:
			base = I2C2_MODE;
			offset = 1;
			break;

		default:
			return GPIO_FAIL;;
	}

	taskENTER_CRITICAL();
	i2c_gpio = inl(base);
	i2c_gpio |= 1 << (offset + I2C_IO_OEN_BIT);

	i2c_gpio &= ~(1 << (offset + I2C_IO_O_BIT));
	i2c_gpio |= info.value << (offset + I2C_IO_O_BIT);
	
	outl(base, i2c_gpio);
	taskEXIT_CRITICAL();

	return GPIO_SUCCESS;;
}

uint32_t snx_i2c_gpio_read (gpio_pin_info* info)
{
		uint32_t base;
	uint32_t i2c_gpio;
	uint32_t offset;


	switch(info->pinumber)
	{
		case I2C1_GPIO_SCL:
			base = I2C1_MODE;
			offset = 0;
			break;
		case I2C1_GPIO_SDA:
			base = I2C1_MODE;
			offset = 1;
			break;
		case I2C2_GPIO_SCL:
			base = I2C2_MODE;
			offset = 0;
			break;
		case I2C2_GPIO_SDA:
			base = I2C2_MODE;
			offset = 1;
			break;

		default:
			return GPIO_FAIL;;
	}

	taskENTER_CRITICAL();
	i2c_gpio = inl(base);
	i2c_gpio &= ~(1 << (offset + I2C_IO_OEN_BIT));
	outl(base, i2c_gpio);

	i2c_gpio = inl(base);
	info->value = (i2c_gpio >> (offset + I2C_IO_I_BIT)) & 0x01;
	taskEXIT_CRITICAL();

	return GPIO_SUCCESS;
}

// UART2_GPIO_FUNCTION
uint32_t snx_uart2_gpio_open()
{
  uint32_t value;
	
	taskENTER_CRITICAL();
	value = inl(BSP_UART2_BASE_ADDRESS+UART2_GPIO_CTRL);
  //set uart2 tx & rx is gpio mode
  
  value &= ~(UART2_GPIO_TX_MODE|UART2_GPIO_RX_MODE);
  outl(BSP_UART2_BASE_ADDRESS+UART2_GPIO_CTRL, value);
    
	taskEXIT_CRITICAL();
}
uint32_t snx_uart2_gpio_close()
{
  return GPIO_SUCCESS;
}
uint32_t snx_uart2_gpio_write (gpio_pin_info info)
{
  uint32_t value;
	taskENTER_CRITICAL();
	value = inl(BSP_UART2_BASE_ADDRESS+UART2_GPIO_IO);
  if (info.pinumber == UART2_GPIO_PIN_TX)
  {
    value |= (UART2_GPIO_TX_OE);
    value &= ~(UART2_GPIO_TX_O);
    value |= info.value<<1;    
  }
  else if (info.pinumber == UART2_GPIO_PIN_RX) 
  {
    value |= (UART2_GPIO_RX_OE);
    value &= ~(UART2_GPIO_RX_O);
    value |= info.value<<5;  
  }

  outl(BSP_UART2_BASE_ADDRESS+UART2_GPIO_IO, value);
  taskEXIT_CRITICAL();
  return GPIO_SUCCESS;  
  
}
uint32_t snx_uart2_gpio_read (gpio_pin_info* info)
{
  uint32_t value;
	          
	taskENTER_CRITICAL();
  value = inl(BSP_UART2_BASE_ADDRESS+UART2_GPIO_IO);
  if (info->pinumber == UART2_GPIO_PIN_TX)
  {
    value &= ~(UART2_GPIO_TX_OE); 
    outl(BSP_UART2_BASE_ADDRESS+UART2_GPIO_IO, value);

	  value = inl(BSP_UART2_BASE_ADDRESS+UART2_GPIO_IO);
	  info->value = value & 0x01;
  }
  else  if (info->pinumber == UART2_GPIO_PIN_RX)
  {
    value &= ~(UART2_GPIO_RX_OE); 
    outl(BSP_UART2_BASE_ADDRESS+UART2_GPIO_IO, value);

	  value = inl(BSP_UART2_BASE_ADDRESS+UART2_GPIO_IO);
	  info->value = (value>>4) & 0x01;
  }

  taskEXIT_CRITICAL();
  return GPIO_SUCCESS;  
}
// JTAG_GPIO_FUNCTION 

//instance 1 :enable 0:disable jtag gpio  
uint32_t snx_jtag_gpio_enable(uint32_t instance) 
{
  JTAG_CTRL_t jtag;
  if(instance == 1)
  {
    jtag.r = inl(GLOBAL_SETTING_BASE_ARRD+JTAG_CTRL);
    jtag.b.JTAG_EN = 0;
    outl(GLOBAL_SETTING_BASE_ARRD+JTAG_CTRL, jtag.r);
  }
  else
  {
    jtag.r = inl(GLOBAL_SETTING_BASE_ARRD+JTAG_CTRL);
    jtag.b.JTAG_EN = 1;
    
    outl(GLOBAL_SETTING_BASE_ARRD+JTAG_CTRL, jtag.r);    
  }
}

uint32_t snx_jtag_gpio_open()
{
  return GPIO_SUCCESS;
}

uint32_t snx_jtag_gpio_close()
{   
  return GPIO_SUCCESS;
}

uint32_t snx_jtag_gpio_write(gpio_pin_info info)
{
  JTAG_CTRL_t jtag;
  taskENTER_CRITICAL();  
  jtag.r = inl(GLOBAL_SETTING_BASE_ARRD+JTAG_CTRL);
  jtag.b.JTAG_EN = 0; //diasble JTAG,enable JTAG_GPIO
  jtag.b.JTAG_GPIO_OE &= ~(1 << info.pinumber);
  jtag.b.JTAG_GPIO_OE |= (1<<info.pinumber);
  jtag.b.JTAG_GPIO_O &= ~(1<<info.pinumber);
  jtag.b.JTAG_GPIO_O |= (info.value<<info.pinumber); 
  outl(GLOBAL_SETTING_BASE_ARRD+JTAG_CTRL, jtag.r); 
  taskEXIT_CRITICAL();
  return GPIO_SUCCESS; 
}

uint32_t snx_jtag_gpio_read(gpio_pin_info* info)
{
  JTAG_CTRL_t jtag;
  taskENTER_CRITICAL();  
  jtag.r = inl(GLOBAL_SETTING_BASE_ARRD+JTAG_CTRL);
  jtag.b.JTAG_EN = 0; //diasble JTAG,enable JTAG_GPIO
  jtag.b.JTAG_GPIO_OE &= ~(1 << info->pinumber);
  outl(GLOBAL_SETTING_BASE_ARRD+JTAG_CTRL, jtag.r);
  jtag.r = inl(GLOBAL_SETTING_BASE_ARRD+JTAG_CTRL);
  info->value = (jtag.b.JTAG_GPIO_I>>info->pinumber) & 0x01;
  taskEXIT_CRITICAL();
  return GPIO_SUCCESS; 
}



// total device gpio function

uint32_t snx_dev_gpio_write(gpio_pin_info info)
{

  info.mode = 1;//set gpio output mode
  taskENTER_CRITICAL();
  if(info.dev == GPIO_DEV)
  {
    {
      uint32_t reg;
		  uint32_t bypass = 0;
		  reg = gpio_get_bypass();
		  reg &= ~(1 << info.pinumber);
		  reg |= (bypass << info.pinumber);
		  gpio_set_bypass(reg);
    }
    snx_gpio_write(info);   
  }
  else if(info.dev == PWM_DEV)
  {
    snx_pwm_gpio_write(info);    
  } 
  else if(info.dev == SPI_DEV) 
  {
    snx_spi_gpio_write(info);  
  }
  else if(info.dev == MS1_DEV)
  {
    snx_ms1_gpio_open();
    snx_ms1_gpio_write(info);  
    snx_ms1_gpio_close();
  }
  else if(info.dev == AUDIO_DEV)
  {
    snx_audio_gpio_open();
    snx_audio_gpio_write(info);
    snx_audio_gpio_close();
  }
  else if(info.dev == I2C2_DEV)
  {
    snx_i2c_gpio_enable(I2C2);  //only enable I2C2 GPIO
    snx_i2c_gpio_write(info);
  }
  else if(info.dev == UART2_DEV)
  {
    snx_uart2_gpio_open();
    snx_uart2_gpio_write(info);
    snx_uart2_gpio_close();
  }
  else if(info.dev == JTAG_DEV)
  {
    snx_jtag_gpio_enable(1);
    snx_jtag_gpio_write(info);  
  }
  else
  {
    taskEXIT_CRITICAL();
    return GPIO_FAIL;
  }
  taskEXIT_CRITICAL();
  return GPIO_SUCCESS;    
    
}
uint32_t snx_dev_gpio_read(gpio_pin_info* info)
{

  info->mode = 1;//set gpio input mode
  taskENTER_CRITICAL();
  if(info->dev == GPIO_DEV)
  {
    {
      uint32_t reg;
		  uint32_t bypass = 0;
		  reg = gpio_get_bypass();
		  reg &= ~(1 << info->pinumber);
		  reg |= (bypass << info->pinumber);
		  gpio_set_bypass(reg);
    }
    snx_gpio_read(info);   
  }
  else if(info->dev == PWM_DEV) 
  {
    snx_pwm_gpio_read(info);
  }
  else if(info->dev == SPI_DEV) 
  {
    snx_spi_gpio_read(info);
  }
  else if(info->dev == MS1_DEV)
  {
    snx_ms1_gpio_open();
    snx_ms1_gpio_read(info);  
    snx_ms1_gpio_close();
  }
  else if(info->dev == AUDIO_DEV)
  {
    snx_audio_gpio_open();
    snx_audio_gpio_read(info);
    snx_audio_gpio_close();
  }
  else if(info->dev == I2C2_DEV)
  {
    snx_i2c_gpio_enable(I2C2);  //only enable I2C2 GPIO
    snx_i2c_gpio_read(info);
  }
  else if(info->dev == UART2_DEV)
  {
    snx_uart2_gpio_open();
    snx_uart2_gpio_read(info);
    snx_uart2_gpio_close();
  }
  else if(info->dev == JTAG_DEV)
  {
    snx_jtag_gpio_enable(1);
    snx_jtag_gpio_read(info);
  }
  else
  {
    taskEXIT_CRITICAL();
    return GPIO_FAIL;
  }
  taskEXIT_CRITICAL();
  return GPIO_SUCCESS;  

}


/*** MS2 GPIO ***/
uint32_t snx_ms2_gpio_enable(uint32_t enable)
{     
	uint32_t ctrl;
	taskENTER_CRITICAL();
	
	ctrl = inl(SD_CTL);
	if(enable == 1)
		ctrl &= ~(0x00000007);  //gpio mode
	else
		ctrl |= (0x00000002);  //SD mode	
	outl(SD_CTL, ctrl);
	taskEXIT_CRITICAL();

	return GPIO_SUCCESS;
}
uint32_t snx_ms2_gpio_open()
{
	return GPIO_SUCCESS;
}

uint32_t snx_ms2_gpio_close ()
{
	return GPIO_SUCCESS;
}

uint32_t snx_ms2_gpio_write (gpio_pin_info info)
{
	uint32_t out;
	uint32_t out_en;

	taskENTER_CRITICAL();
	out = inl(SD_MS_IO_O);
	out_en = inl(SD_MS_IO_OE);
	
	out_en &= ~(1 << info.pinumber);
	out_en |= (info.mode << info.pinumber);
	
	out &= ~(1 << info.pinumber);
	out |= (info.value << info.pinumber);

	outl(SD_MS_IO_OE, out_en);
	outl(SD_MS_IO_O, out);
	taskEXIT_CRITICAL();

	return GPIO_SUCCESS;
}

uint32_t snx_ms2_gpio_read (gpio_pin_info* info)
{
	uint32_t in_value;
	uint32_t out_en;

	taskENTER_CRITICAL();

	out_en = inl(SD_MS_IO_OE);
	out_en &= ~(1 << info->pinumber);
	out_en |= (info->mode << info->pinumber);
	outl(SD_MS_IO_OE, out_en);
	
	in_value = inl(SD_MS_IO_I);

	info->value = (in_value >> info->pinumber) & 0x01;
	
	taskEXIT_CRITICAL();

	return GPIO_SUCCESS;
}

