/**
 * @file
 * this is sdv2 driver header file
 * @author CJ
 */

#ifndef __SDV2_SD_H__
#define __SDV2_SD_H__

#include "sd_ctrl.h"

#define SD_CD_PIN					1
#define SD_WP_PIN					2
#define SD_D3_PIN					7

#define SD_CLK_IDENT				0xFA	//100.5/(250+2) = 399K
#define SD_CLK_HIGH					0x0		//100.5/(0+2) = 50.25M
#define SD_CLK_LOW					0x2		//100.5/(2+2) = 25.125M

#define SD_MS_RDY					0
#define SD_MS_NOT_RDY				1

#ifndef OS_ON
	#define OS_ON					0x01
#endif

#ifndef TRUE
	#define TRUE					0x01
#endif

#ifndef FALSE
	#define FALSE					0x00
#endif

#define SD_VER_11					0x01
#define SD_VER_20					0x02	
#define SD_STD_CARD					0x01	// standard capacity 
#define SD_HIGH_CARD				0x02	// high capacity 

/**
 * @brief sd command structure
 */
struct sd_m2_command
{
	uint32_t index;					/**< command index */
	uint32_t arg;					/**< command arg */
	uint32_t app_cmd;				/**< app cmd */
	uint32_t next_state;			/**< next state */
	int resp_type;					/**< response type */
	uint32_t resp[5];				/**< response (max: 160bits) */
};

//=========================================================================
// MS mode
//=========================================================================
#define MS_SPI_MODE				1
#define MS_SD_MODE				2

//=========================================================================
// OCR Register definition 
//=========================================================================
#define OCR_BUSY_BIT			31
#define OCR_CCS_BIT				30
#define OCR_32_33_BIT			20
					
#define OCR_BUSY_MASK			0x80000000
#define OCR_CCS_MASK			0x40000000
#define OCR_32_33_MASK			0x00100000

//=========================================================================
// SD_CURRENT_STATE 
//=========================================================================
#define SD_STATE_IDLE			0
#define SD_STATE_READY			1
#define SD_STATE_IDENT			2
#define SD_STATE_STBY			3
#define SD_STATE_TRAN			4
#define SD_STATE_DATA			5
#define SD_STATE_RCV			6
#define SD_STATE_PRG			7
#define SD_STATE_DIS			8

//=========================================================================
// SD_CMD SD_RESP Address offset 0x70
//=========================================================================
#define SD_RESP_NONE			0
#define SD_RESP_SD_R1 			1	// normal response
#define SD_RESP_SD_R2 			2	// CID, CSD
#define SD_RESP_SD_R3 			3	// OCR, no CRC
#define SD_RESP_SD_R4 			4	// SDIO support additional type
#define SD_RESP_SD_R5 			5 	// SDIO support additional type
#define SD_RESP_SD_R6 			6	// RCA
#define SD_RESP_SD_R7			7	// Card interface condition
#define SD_RESP_SD_R1b			SD_RESP_SD_R1 

#define SD_RESP_SPI_R1			1
#define SD_RESP_SPI_R2			2
#define SD_RESP_SPI_R3			3
#define SD_RESP_SPI_R7			7

#define SD_CMD0_INDEX			0
#define SD_CMD0_SD_ARG_INIT		0x0
#define SD_CMD0_SD_RESPTYPE		SD_RESP_NONE
#define SD_CMD0_SPI_ARG_INIT	0x0
#define SD_CMD0_SPI_RESPTYPE	SD_RESP_SPI_R1

#define SD_CMD1_INDEX			1
#define SD_CMD1_SPI_ARG_INIT	0x0
#define SD_CMD1_SPI_RESPTYPE	SD_RESP_SPI_R1

#define SD_CMD2_INDEX			2
#define SD_CMD2_SD_ARG_INIT		0x0
#define SD_CMD2_SD_RESPTYPE		SD_RESP_SD_R2

#define SD_CMD3_INDEX			3
#define SD_CMD3_SD_ARG_INIT		0x0
#define SD_CMD3_SD_RESPTYPE		SD_RESP_SD_R6

#define SD_CMD5_INDEX			5	
#define SD_CMD5_SD_ARG_INIT		0x0
#define SD_CMD5_SD_RESPTYPE		SD_RESP_SD_R4
#define SD_CMD5_SPI_ARG_INIT	0x0	
#define SD_CMD5_SPI_RESPTYPE	SD_RESP_SPI_R3

#define SD_CMD6_INDEX			6	
#define SD_CMD6_SD_RESPTYPE		SD_RESP_SD_R1
#define SD_CMD6_SPI_ARG_INIT		
#define SD_CMD6_SPI_RESPTYPE	SD_RESP_SPI_R1

#define SD_CMD7_INDEX			7
#define SD_CMD7_SD_RESPTYPE		SD_RESP_SD_R1b

#define SD_CMD8_INDEX			8
#define SD_CMD8_SD_ARG_INIT		0x1aa
#define SD_CMD8_SD_RESPTYPE		SD_RESP_SD_R7
#define SD_CMD8_SPI_ARG_INIT	0x1aa
#define SD_CMD8_SPI_RESPTYPE	SD_RESP_SPI_R7

#define SD_CMD9_INDEX			9
#define SD_CMD9_SD_ARG_INIT		0x0
#define SD_CMD9_SD_RESPTYPE		SD_RESP_SD_R2

#define SD_CMD10_INDEX			10
#define SD_CMD10_SD_ARG_INIT	0x0
#define SD_CMD10_SD_RESPTYPE	SD_RESP_SD_R2

#define SD_CMD13_INDEX			13
#define SD_CMD13_SD_ARG_INIT	0x0
#define SD_CMD13_SD_RESPTYPE	SD_RESP_SD_R1

#define SD_CMD16_INDEX			16
#define SD_CMD16_SD_RESPTYPE	SD_RESP_SD_R1
#define SD_CMD16_SPI_RESPTYPE	SD_RESP_SPI_R1

#define SD_CMD17_INDEX			17
#define SD_CMD17_SD_RESPTYPE	SD_RESP_SD_R1
#define SD_CMD17_SPI_RESPTYPE	SD_RESP_SPI_R1

#define SD_CMD18_INDEX			18
#define SD_CMD18_SD_RESPTYPE	SD_RESP_SD_R1
#define SD_CMD18_SPI_RESPTYPE	SD_RESP_SPI_R1

#define SD_CMD24_INDEX			24
#define SD_CMD24_SD_RESPTYPE	SD_RESP_SD_R1
#define SD_CMD24_SPI_RESPTYPE	SD_RESP_SPI_R1

#define SD_CMD25_INDEX			25
#define SD_CMD25_SD_RESPTYPE	SD_RESP_SD_R1
#define SD_CMD25_SPI_RESPTYPE	SD_RESP_SPI_R1

#define SD_CMD32_INDEX			32
#define SD_CMD32_SD_RESPTYPE	SD_RESP_SD_R1
#define SD_CMD32_SPI_RESPTYPE	SD_RESP_SPI_R1


#define SD_CMD33_INDEX			33
#define SD_CMD33_SD_RESPTYPE	SD_RESP_SD_R1
#define SD_CMD33_SPI_RESPTYPE	SD_RESP_SPI_R1


#define SD_CMD38_INDEX			38
#define SD_CMD38_SD_RESPTYPE	SD_RESP_SD_R1
#define SD_CMD38_SPI_RESPTYPE	SD_RESP_SPI_R1

#define SD_CMD52_INDEX			52
#define SD_CMD52_SD_ARG_INIT	0xff
#define SD_CMD52_SD_RESPTYPE	SD_RESP_SD_R5
#define SD_CMD52_SPI_ARG_INIT	0x0
#define SD_CMD52_SPI_RESPTYPE	SD_RESP_SPI_R1

#define SD_CMD53_INDEX			53
#define SD_CMD53_SD_ARG_INIT	0xff
#define SD_CMD53_SD_RESPTYPE	SD_RESP_SD_R5
#define SD_CMD53_SPI_ARG_INIT	0x0
#define SD_CMD53_SPI_RESPTYPE	SD_RESP_SPI_R1

#define SD_CMD55_INDEX			55
#define SD_CMD55_SD_ARG_INIT	0x0
#define SD_CMD55_SD_RESPTYPE	SD_RESP_SD_R1
#define SD_CMD55_SPI_ARG_INIT	0x0
#define SD_CMD55_SPI_RESPTYPE	SD_RESP_SPI_R1

#define SD_CMD58_INDEX			58
#define SD_CMD58_SPI_ARG_INIT	0x0
#define SD_CMD58_SPI_RESPTYPE	SD_RESP_SPI_R3

#define SD_ACMD6_INDEX			6
#define SD_ACMD6_SD_RESPTYPE	SD_RESP_SD_R1

#define SD_ACMD13_INDEX			13
#define SD_ACMD13_SD_RESPTYPE	SD_RESP_SD_R1
#define SD_ACMD13_SPI_RESPTYPE	SD_RESP_SPI_R2

#define SD_ACMD41_INDEX			41
#define SD_ACMD41_SD_ARG_INIT	0x0
#define SD_ACMD41_SD_RESPTYPE	SD_RESP_SD_R3
#define SD_ACMD41_SPI_ARG_INIT	0x0
#define SD_ACMD41_SPI_RESPTYPE	SD_RESP_SPI_R1

#define SD_ACMD51_INDEX			51
#define SD_ACMD51_SD_RESPTYPE	SD_RESP_SD_R1
#define SD_ACMD51_SPI_RESPTYPE	SD_RESP_SPI_R1

//=========================================================================
// Return message
//=========================================================================
#define	SD_RTN_ERR_RSP_CMD7			(-7)
#define	SD_RTN_ERR_RSP_CMD13			(-13)
#define	SD_RTN_ERR_RSP_CMD16			(-16)
#define	SD_RTN_ERR_RSP_CMD16_ILL		(-161)
#define	SD_RTN_ERR_RSP_CMD17			(-17)
#define	SD_RTN_ERR_RSP_CMD24			(-24)
#define	SD_RTN_ERR_RSP_CMD58			(-58)
#define	SD_RTN_ERR_RSP_ACMD41			(-141)

#define	SD_RTN_ERR_MS_RDY			(-200)

#define	SD_RTN_ERR_MDMA_COMPLETE_FLAG		(-300)
#define	SD_RTN_ERR_MDMA_TIMEOUT			(-301)
#define	SD_RTN_ERR_MDMA_FAIL			(-302)
#define	SD_RTN_ERR_MDMA_SU_BLOCK		(-303)
#define SD_RTN_ERR_MDMA_SIZE			(-304)

#define	SD_RTN_ERR_SEMAPHORE_CREATE		(-400)
#define SD_RTN_ERR_SEMAPHORE_TAKE		(-401)

#define SD_RTN_ERR_VER_NO_SUPPORT		(-500)

#define SD_RTN_ERR_REGISTER_IRQ			(-600)

#define	SD_RTN_PASS				1

// =========================================================================
// SD Function
// =========================================================================
uint32_t sd_ready_detect(void);
void sd_m2setcmd(uint32_t index, uint32_t arg, int resp_type,struct sd_m2_command *cmd_info);
void sd_resetm2cmd(struct sd_m2_command *cmd_info);
void sd_sd_m2_WcmdRresp(struct sd_m2_command *cmd_info);
void sd_tx_cmd(uint32_t index, uint32_t arg, int resp_type,struct sd_m2_command *cmd_info);
int sd_sd_write_cmd(uint32_t start_addr, uint32_t dma_size, struct sd_m2_command *cmd_info);
void sd_sd_read_cmd(uint32_t start_addr, struct sd_m2_command *cmd_info);

void sd_sd_resp1(struct sd_m2_command *cmd_info);
void sd_sd_resp2(struct sd_m2_command *cmd_info);
//void sd_sd_resp3(struct sd_m2_command *cmd_info);
//void sd_sd_resp4(struct sd_m2_command *cmd_info);
//void sd_sd_resp5(struct sd_m2_command *cmd_info);
//void sd_sd_resp6(struct sd_m2_command *cmd_info);
//void sd_sd_resp7(struct sd_m2_command *cmd_info);

uint32_t sd_check_status_cmd13(uint32_t RCAValue, struct sd_m2_command *cmd_info);
uint32_t sd_sd_app_cmd_tran(uint32_t index, uint32_t arg, uint32_t sd_bus_width);
uint32_t sd_sd_app_cmd_idle(uint32_t index, uint32_t arg, uint32_t *value);

int sd_sd_identify(struct sd_info *info);
void sd_cal_capacity(struct sd_info *info);

// =========================================================================
// SD R1 Response check 
// =========================================================================
uint32_t sd_r1_check_all(struct sd_m2_command *cmd_info, uint32_t SDIO_SWITCH);

// =========================================================================
// SD SPI mode
// =========================================================================
void sd_spi_resp1(struct sd_m2_command *cmd_info);
void sd_spi_resp2(struct sd_m2_command *cmd_info);
void sd_spi_resp3(struct sd_m2_command *cmd_info);
void sd_spi_resp7(struct sd_m2_command *cmd_info);
void sd_spi_m2_cmd(uint32_t index, uint32_t arg, int resp_type,struct sd_m2_command *cmd_info);
int sd_spi_read_cmd(uint32_t start_addr, struct sd_m2_command *cmd_info);
int sd_spi_write_cmd(uint32_t start_addr, uint32_t dma_size, struct sd_m2_command *cmd_info);
void sd_spi_m2_WcmdRresp(struct sd_m2_command *cmd_info);
int sd_spi_identify(struct sd_info *info);

#endif /* __SDV@_SD_H__  */
