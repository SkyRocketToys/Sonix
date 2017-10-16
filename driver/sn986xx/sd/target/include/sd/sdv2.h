/**
 * @file
 * this is sd register control header file
 * @author CJ
 */

#ifndef __SDV2_H__
#define __SDV2_H__

#ifndef ENABLE
	#define ENABLE				1
#endif
#ifndef DISABLE
	#define DISABLE				0
#endif

#define BIT0							0x00000001
#define BIT1							0x00000002
#define BIT2							0x00000004
#define BIT3							0x00000008
#define BIT4							0x00000010
#define BIT5							0x00000020
#define BIT6							0x00000040
#define BIT7							0x00000080
#define BIT8							0x00000100
#define BIT9							0x00000200
#define BIT10							0x00000400
#define BIT11							0x00000800
#define BIT12							0x00001000
#define BIT13							0x00002000
#define BIT14							0x00004000
#define BIT15							0x00008000
#define BIT16							0x00010000
#define BIT17							0x00020000
#define BIT18							0x00040000
#define BIT19							0x00080000
#define BIT20							0x00100000
#define BIT21							0x00200000
#define BIT22							0x00400000
#define BIT23							0x00800000
#define BIT24							0x01000000
#define BIT25							0x02000000
#define BIT26							0x04000000
#define BIT27							0x08000000
#define BIT28							0x10000000
#define BIT29							0x20000000
#define BIT30							0x40000000
#define BIT31							0x80000000

//=========================================================================
// Register address offset
//=========================================================================
#define	MS2_BASE_ADDRESS		(0x91000000)

#define SD_CTL					(MS2_BASE_ADDRESS)
#define SD_DMA_SIZE				(MS2_BASE_ADDRESS + 0x04)
#define SD_CRC1					(MS2_BASE_ADDRESS + 0x08)
#define SD_CRC2					(MS2_BASE_ADDRESS + 0x0C)
#define SD_CRC3					(MS2_BASE_ADDRESS + 0x10)
#define SD_SDIO					(MS2_BASE_ADDRESS + 0x14)
#define SD_SDIO_ADDR			(MS2_BASE_ADDRESS + 0x18)

#define SD_MS_IO_I				(MS2_BASE_ADDRESS + 0x30)
#define SD_MS_IO_O				(MS2_BASE_ADDRESS + 0x34)
#define SD_MS_IO_OE				(MS2_BASE_ADDRESS + 0x38)

#define SD_SPI_CMD				(MS2_BASE_ADDRESS + 0x3c)
#define SD_SPI_INDEX			(MS2_BASE_ADDRESS + 0x40)

#define SD_DMA_BLKSU			(MS2_BASE_ADDRESS + 0x48)
#define SD_TMCNT				(MS2_BASE_ADDRESS + 0x4c)
#define SD_MDMAECC				(MS2_BASE_ADDRESS + 0x50)
#define SD_LBA					(MS2_BASE_ADDRESS + 0x54)

#define SD_DMA_ADDR				(MS2_BASE_ADDRESS + 0x5c)
#define SD_REG_CMD				(MS2_BASE_ADDRESS + 0x70)
#define SD_REG_DATA				(MS2_BASE_ADDRESS + 0x74)
#define SD_REG_DUMMYCLOCK		(MS2_BASE_ADDRESS + 0x78)
#define SD_AUTO_RESPONSE		(MS2_BASE_ADDRESS + 0x7c)

//=========================================================================
// SD_CTL Address offset 0x00
//=========================================================================
#define SD_CTL_MODE_BIT				0
#define SD_CTL_REGRW_BIT			3
#define SD_CTL_DMAEN_BIT			4
#define SD_CTL_DMARW_BIT			5
#define SD_CTL_EXTRAEN_BIT			6
#define SD_CTL_ECCEN_BIT			7
#define SD_CTL_MSRDY_BIT			8 
#define SD_CTL_SPIBUSYTRI_BIT		12
#define SD_CTL_SPICMDTRI_BIT		13
#define SD_CTL_RAEDDATACMD_BIT		22
#define SD_CTL_MSSPEED_BIT			24

#define SD_CTL_MODE_MASK			(BIT0 | BIT1 | BIT2)
#define SD_CTL_REGRW_MASK			(1 << SD_CTL_REGRW_BIT)
#define SD_CTL_DMAEN_MASK			(1 << SD_CTL_DMAEN_BIT)
#define SD_CTL_DMARW_MASK			(1 << SD_CTL_DMARW_BIT)
#define SD_CTL_EXTRAEN_MASK			(1 << SD_CTL_EXTRAEN_BIT)
#define SD_CTL_ECCEN_MASK			(1 << SD_CTL_ECCEN_BIT)
#define SD_CTL_MSRDY_MASK			(1 << SD_CTL_MSRDY_BIT) 
#define SD_CTL_SPIBUSYTRI_MASK		(1 << SD_CTL_SPIBUSYTRI_BIT) 
#define SD_CTL_SPICMDTRI_MASK		(1 << SD_CTL_SPICMDTRI_BIT) 
#define SD_CTL_RAEDDATACMD_MASK		(1 << SD_CTL_RAEDDATACMD_BIT) 
#define SD_CTL_MSSPEED_MASK			( BIT24 | BIT25 | BIT26 | BIT27 | BIT28 | BIT29 | BIT30 | BIT31 )

//=========================================================================
// SD_DMA_SIZE Address offset 0x04
//=========================================================================
#define SD_DMASIZE_MSDMASIZE_BIT	0

#define SD_DMASIZE_MSDMASIZE_MASK	0x00000fff	

//=========================================================================
// SD_CRC1 Address offset 0x08
//=========================================================================
#define SD_CRC1_CRC16_0_L_BIT		0
#define SD_CRC1_CRC16_0_H_BIT		8
#define SD_CRC1_CRC7_BIT			16

#define SD_CRC1_CRC16_0_L_MASK		0x000000ff
#define SD_CRC1_CRC16_0_H_MASK		0x0000ff00
#define SD_CRC1_CRC7_MASK			0x007f0000

//=========================================================================
// SD_CRC2 Address offset 0x0c
//=========================================================================
#define SD_CRC2_CRC16_1_BIT			0

#define SD_CRC2_CRC16_1_MASK		0x0000ffff

//=========================================================================
// SD_CRC3 Address offset 0x10
//=========================================================================
#define SD_CRC2_CRC16_2_BIT			0
#define SD_CRC2_CRC16_3_BIT			16

#define SD_CRC2_CRC16_2_MASK		0x0000ffff
#define SD_CRC2_CRC16_3_MASK		0xffff0000

//=========================================================================
// SD_SDIO Address offset 0x14
//=========================================================================
#define SDIO_EN_BIT					0
#define SDIO_RD_WAIT_EN_BIT			1

#define SDIO_FUNC_BIT				8
#define SDIO_OP_CODE_BIT			12
#define SDIO_BLK_MODE_BIT			13

#define SDIO_EN_MASK				0x00000001
#define SDIO_RD_WAIT_EN_MASK		0x00000002
#define SDIO_FUNC_MASK				0x00000700
#define SDIO_OP_CODE_MASK			0x00001000
#define SDIO_BLK_MODE_MASK			0x00002000

//=========================================================================
// SD_SDIO_ADDR Address offset 0x18
//=========================================================================
#define SDIO_REG_ADDR_BIT			0

#define SDIO_REG_ADDR_MASK			0x0001ffff

//=========================================================================
// SD_MS_IO Address offset 0x30 & 0x34 & 0x38
//=========================================================================
#define SD_MS_IO_I_I_BIT			0
#define SD_MS_IO_O_O_BIT			0
#define SD_MS_IO_OE_OE_BIT			0

#define SD_MS_IO_I_I_MASK			0x000000ff	
#define SD_MS_IO_O_O_MASK			0x000000ff
#define SD_MS_IO_OE_OE_MASK			0x000000ff	

//=========================================================================
// SD_CMD SD_INDEX Address offset 0x3c & 0x40
//=========================================================================
#define SD_SPI_CMD_CMD_BIT 			0
#define SD_SPI_INDEX_INDEX_BIT		0

#define SD_SPI_CMD_CMD_MASK 		0xffffffff
#define SD_SPI_INDEX_INDEX_MASK		0x000000ff

//=========================================================================
// SD_DMA_BLKSU Address offset 0x48
//=========================================================================
#define SD_BLKSU_BLK_BIT			0
#define SD_BLKSU_SUBLK_BIT			16

#define SD_BLKSU_BLK_MASK			0x0000ffff
#define SD_BLKSU_SUBLK_MASK			0xffff0000

//=========================================================================
// SD_TMCNT Address offset 0x4c
//=========================================================================
#define SD_TMCNT_TMCNT_BIT			0

#define SD_TMCNT_TMCNT_MASK			0x3fffffff

//=========================================================================
// SD_MDMAECC Address offset 0x50
//=========================================================================
#define SD_MS_M_DMA_EN_BIT			0
#define SD_MS_M_DMA_OK_BIT			1
#define SD_MS_M_DMA_TIME_OUT_BIT	2
#define SD_MDMAECC_CRC_W_BIT		6
#define SD_MDMAECC_CRCERR_BIT		7
#define SD_MS_RDY_INTR_EN_BIT		8
#define SD_MS_ERR_INTR_EN_BIT		9
#define SD_DETECT_INTR_EN_BIT		11
#define SD_MS_RDY_FLAG_BIT			16
#define SD_MS_ERR_FLAG_BIT			17
#define SD_DETECT_FLAG_BIT			19
#define SD_CLR_MS_RDY_BIT			24
#define SD_CLR_MS_ERR_BIT			25
#define SD_CLR_DETECT_BIT			27
#define SD_CLR_MDMA_TIMEOUT_BIT		28

#define SD_MS_M_DMA_EN_MASK			BIT0
#define SD_MS_M_DMA_OK_MASK			BIT1
#define SD_MS_M_DMA_TIME_OUT_MASK	BIT2
#define SD_MDMAECC_CRC_W_MASK		BIT6
#define SD_MDMAECC_CRCERR_MASK		BIT7
#define SD_MS_RDY_INTR_EN_MASK		BIT8
#define SD_MS_ERR_INTR_EN_MASK		BIT9
#define SD_DETECT_INTR_EN_MASK		BIT11
#define SD_MS_RDY_FLAG_MASK			BIT16
#define SD_MS_ERR_FLAG_MASK			BIT17
#define SD_DETECT_FLAG_MASK			BIT19
#define SD_CLR_MS_RDY_MASK			BIT24
#define SD_CLR_MS_ERR_MASK			BIT25
#define SD_CLR_ECC_ERR_MASK			BIT26
#define SD_CLR_DETECT_MASK			BIT27
#define SD_CLR_MDMA_TIMEOUT_MASK	BIT28
#define SD_CLR_W_ERR_MASK			BIT29

//=========================================================================
// SD LBA Address offset 0x54
//=========================================================================
#define SD_LBA_LBAW_BIT				9

#define SD_LBA_LBAW_MASK			0x00003e00

//=========================================================================
// MS mode
//=========================================================================
#define SD_CTL_MODE_GPIO			0x0	
#define SD_CTL_MODE_SPI				0x1	
#define SD_CTL_MODE_SD				0x2	

//=========================================================================
// MS R/W mode
//=========================================================================
#define SD_READ_MODE 				0x1
#define SD_WRITE_MODE 				0x0

//=========================================================================
// MSDMA status
//=========================================================================
#define	MDMA_OK_ON_TIME				0
#define MDMA_OK_TIME_OUT			(-1)
#define MDMA_FAIL_ON_TIME			(-2)
#define MDMA_FAIL_TIME_OUT			(-3)

//=========================================================================
// function
//=========================================================================
void sdSetMsMode(uint32_t mode);
void sdMsRegRWSwitch(uint32_t value);
void sdMsDmaENSwitch (uint32_t isEnable);
void sdMsDmaRWSwitch(uint32_t value);
void sdExtraENSwitch(uint32_t isEnable);
void sdEccENSwitch(uint32_t isEnable);
uint32_t sdCheckMsRdy(void);
void sdSetSpiBusyTri(uint32_t isEnable);
void sdSetSpiCmdTri(uint32_t isEnable);
void sdReadDataCmd(uint32_t isEnable);
void sdSetMsSpeed(uint32_t msspeed);
void sdSetBlkSize(uint32_t size);
uint32_t sdReadCrc16_0(void);
uint32_t sdReadCrc7(void);
uint32_t sdReadCrc16_1(void);
uint32_t sdReadCrc16_2and16_3(void);
uint32_t sdMsIOI(void);
void sdMsIOO_8(uint32_t value);
void sdMsIOOE_8(uint32_t value);
void sdReadGpioIn (uint32_t num);
void sdWriteGpioOut (uint32_t gpio_num, uint32_t sig);
void sdWriteSpiCmd(uint32_t cmd);
uint32_t sdReadSpiCmd(void);
void sdWriteSpiIndex(uint32_t index);
uint32_t sdReadSpiIndex(void);
void sdDmaBlock(uint32_t dmaBlockNum);
uint32_t sdReadSUDmaBlock(void);
void sdSetTimeCount(uint32_t timeCnt);
void sdMsMDmaENSwitch(uint32_t isEnable);
uint32_t sdCheckMsMDmaOkOnTime(void);
uint32_t sdCheckCrcErr(void);
void sdMsRdyIntrENSwitch(uint32_t isEnable);
uint32_t sdCheckMsRdyFlag(void);
void sdClearMsRdyFlag(uint32_t value);
void sdMsErrIntrENSwitch(uint32_t isEnable);
uint32_t sdCheckMsErrFlag(void);
void sdClearMsErrFlag(uint32_t value);
void sdCdIntrENSwitch(uint32_t isEnable);
uint32_t sdCdIntrFlag(void);
void sdClearCdIntrFlag(uint32_t value);
void sdSetLba(uint32_t value);
void sdSetDmaAddr(uint32_t addr);
void sdWriteCommand(uint32_t cmd);
uint32_t sdReadCommand(void);
void sdWriteData(uint32_t value);
uint32_t sdReadData(void);
void sdWriteDummyClock(uint32_t value);
void sdWriteAutoResponse(uint32_t value);
void sdChipDisable(void);
void sdChipEnable(void);
void sdReset(void);
void sdioENSwitch(uint32_t isEnable);
void sdioFuncSet(uint32_t Value);
void sdioOPCodeSet(uint32_t Value);
void sdioBLKmodeSet(uint32_t Value);
void sdioRegAddrSet(uint32_t Value);

#endif /* __SDV2_H__  */
