/*
 * spi.h
 *
 *  Created on: Apr 23, 2015
 *      Author: spark_lee
 */

#ifndef DRIVER_SN986XX_SPI_SRC_SPI_H_
#define DRIVER_SN986XX_SPI_SRC_SPI_H_

#include <FreeRTOS.h>
#include <task.h>
#include "semphr.h"
#include "queue.h"

#define BSP_SPI_BASE_ADDRESS		(0x98900000)

#define SPI_CTRL_REG		(0x00)
#define SPI_CLK_REG			(0x04)
#define SPI_TXRXCLR_REG		(0x08)
#define SPI_STAT_REG		(0x0c)
#define SPI_TXRXEN_REG		(0x10)
#define SPI_FLAG_REG		(0x14)
#define SPI_DATA_REG		(0x18)
#define SSP_DATA_REG		(0x18)
#define SPI_SLOT_REG		(0x20)

#define SPI_REV_REG			(0x40)
#define SPI_FIFOWD_REG		(0x44)

#define SSP_MSB_FIRST	(0)
#define SSP_LSB_FIRST	(1)
#define SPI_MSB_FIRST	(0)
#define SPI_LSB_FIRST	(1)

#define SPI_BUSY		(1)
#define SPI_IDLE		(0)

#define SPI_RXFIFO_FULL		(1)
#define SPI_RXFIFO_NOTFULL	(0)
#define SPI_TXFIFO_FULL		(0)
#define SPI_TXFIFO_NOTFULL	(1)


#define SPI_FORMAT			(1)

#define SPI_SLAVE_MODE		(1)
#define SPI_MASTER_MODE		(2)


typedef union
{
	uint32_t r;
	struct {
		uint32_t SSP_SCLKPH:1;
		uint32_t SSP_SCLKPO:1;
		uint32_t SSP_OPM:2;
		uint32_t SSP_FSJSTFY:1;
		uint32_t SSP_FSPO:1;
		uint32_t SSP_LSB:1;
		uint32_t SSP_LBM:1;
		uint32_t SSP_FSDIST:2;
		uint32_t Resv:2;
		uint32_t SSP_FFMT:3;
		uint32_t SSP_GPIO_MODE:1;

		uint32_t SSP_CLK_GPIO_OE:1;
		uint32_t SSP_FS_GPIO_OE:1;
		uint32_t SSP_TX_GPIO_OE:1;
		uint32_t SSP_RX_GPIO_OE:1;

		uint32_t SSP_CLK_GPIO_O:1;
		uint32_t SSP_FS_GPIO_O:1;
		uint32_t SSP_TX_GPIO_O:1;
		uint32_t SSP_RX_GPIO_O:1;

		uint32_t SSP_CLK_GPIO_I:1;
		uint32_t SSP_FS_GPIO_I:1;
		uint32_t SSP_TX_GPIO_I:1;
		uint32_t SSP_RX_GPIO_I:1;

		uint32_t Resv2:4;
	}b;

}SPI_CTRL_t;

typedef union
{
	uint32_t r;
	struct
	{
		uint32_t SSP_SCLKDIV:16;
		uint32_t SSP_SDL:5;
		uint32_t Rev:3;
		uint32_t SSP_PDL:8;
	}b;
}SPI_CLK_t;


typedef union
{
	uint32_t r;
	struct
	{
		uint32_t SSP_EN:1;
		uint32_t SSP_TXDOE:1;
		uint32_t SSP_RXF_CLR:1;
		uint32_t SSP_TXF_CLR:1;
		uint32_t SSP_ACWRST:1;
		uint32_t SSP_ACCRST:1;
		uint32_t SSP_RST:1;
		uint32_t Rev:25;
	}b;
}SPI_TXRXCLR_t;

typedef union
{
	uint32_t r;
	struct
	{
		uint32_t SSP_RXF_FULL:1;
		uint32_t SSP_TXF_NFULL:1;
		uint32_t SSP_BUSY:1;
		uint32_t Rev1:1;
		uint32_t SSP_RXF_VE:5;
		uint32_t Rev2:3;
		uint32_t SSP_TXF_VE:5;
		uint32_t Rev3:15;
	}b;
}SPI_STAT_t;

typedef union
{
	uint32_t r;
	struct
	{
		uint32_t SSP_RXF_OR_INTR_EN:1;
		uint32_t SSP_TXF_UR_INTR_EN:1;
		uint32_t SSP_RXF_TH_INTR_EN:1;
		uint32_t SSP_TXF_TH_INTR_EN:1;
		uint32_t SSP_RXF_DMA_EN:1;
		uint32_t SSP_TXF_DMA_EN:1;
		uint32_t AC97_FC_INTR_EN:1;
		uint32_t Rev1:1;
		uint32_t SSP_RXF_TH:4;
		uint32_t SSP_TXF_TH:4;
		uint32_t Rev2:16;

	}b;
}SPI_TXRXEN_t;

typedef union
{
	uint32_t r;
	struct
	{
		uint32_t SSP_RXF_OR_FLAG:1;
		uint32_t SSP_TXF_UR_FLAG:1;
		uint32_t SSP_RXF_TH_FLAG:1;
		uint32_t SSP_TXF_TH_FLAG:1;
		uint32_t AC97_FC_FLAG:1;
		uint32_t Rev:27;

	}b;
}SPI_FLAG_t;


typedef union
{
	uint32_t r;
	struct
	{
		uint32_t FIFO_WIDTH:8;
		uint32_t RXFIFO_DEPTH:8;
		uint32_t TXFIFO_DEPTH:8;
		uint32_t AC97_FCFG:1;
		uint32_t I2S_FCFG:1;
		uint32_t SPIMWR_FCFG:1;
		uint32_t SSP_FCFG:1;
		uint32_t Rev:4;
	}b;
}SPI_FIFOWD_t;



/******************/

typedef struct
{
	uint32_t CLKDIV;
	uint32_t PH;
	uint32_t PO;
	uint32_t DATA_LEN;	//Bit

}SPI_INIT_t;


/*** spi middleware ***/

#define SPI_ITEM_SIZE	sizeof(spi_data_t)
#define SPI_QUEUE_LEN	10

typedef struct
{
	unsigned char *data;
	unsigned int len;
}spi_data_t;

typedef struct
{
	xSemaphoreHandle spi_sem;
	xQueueHandle TxQ;
	xQueueHandle RxQ;
}spi_queue_t;

void spi_middleware_send(spi_data_t *pkg);
void spi_middleware_task(void* pvParameters);
void spi_app_task(void* pvParameters);
/********************/

uint32_t spi_isbusy();
uint32_t spi_isrxfull();
uint32_t spi_istxnfull();

void spi_enable(uint32_t en);
void spi_reset();

void spi_set_rxfifo_th(uint32_t th);
void spi_set_txfifo_th(uint32_t th);
void spi_set_rxfifo_th_intr_en(uint32_t en);
void spi_set_txfifo_th_intr_en(uint32_t en);
void spi_clr_txfifo();
void spi_clr_rxfifo();

uint32_t spi_getstat();

void spi_write(uint32_t data);
uint32_t spi_read();

void spi_init(SPI_INIT_t set);
void spi_test();

#endif /* DRIVER_SN986XX_SPI_SRC_SPI_H_ */
