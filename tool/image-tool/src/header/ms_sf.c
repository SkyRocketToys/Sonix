#include "common.h"
#include "ms_sf.h"

#define SF_CS_ENABLE	0
#define SF_CS_DISABLE	1

/**
 * 0x00
 * SF CTL APIs
 */
void sfSetMsMode (unsigned mode)
{
	u32 data;
	
	data  = readl(BASE_MS1 + SF_CTL);
	data &= ~(SF_CTL_MODE); 
	
	writel(data | mode, BASE_MS1 + SF_CTL);
}

/**
 * [value] 1: read mode 
 *         0: write mode
 */
void sfMsRegRWSwitch (u32 value)
{
	u32 data;
	
	data  = readl(BASE_MS1 + SF_CTL);
	data &= ~(SF_CTL_REGRW);
	data |= (value << 3);
	
	writel(data, BASE_MS1 + SF_CTL);	
}

/**
 * [isEnable] 1: enable DMA 
 *            0: disable DMA
 */
void sfMsDmaENSwitch (u32 isEnable)
{
	u32 data;
	
	data  = readl(BASE_MS1 + SF_CTL);
	data &= ~(SF_CTL_DMAEN);
	data |= (isEnable << 4);

	writel(data, BASE_MS1 + SF_CTL);	
}

/**
 *¡[value] 1: DMA read mode
 *         0: DMA write mode
 */
void sfMsDmaRWSwitch (u32 value)
{
	u32 data;
	
	data  = readl(BASE_MS1 + SF_CTL);
	data &= ~(SF_CTL_DMARW);
	data |= (value << 5);
	
	writel(data, BASE_MS1 + SF_CTL);	
}

/** 
 * [isEnable] 1: enable Extra ECC DATA enable when 
 *               DMA transfer for nandflash
 *            0: disable
 */
void sfExtraENSwitch (u32 isEnable)
{
	u32 data;
	
	data  = readl(BASE_MS1 + SF_CTL);
	data &= ~(SF_CTL_EXTRAEN);
	data |= (isEnable << 6);
	
	writel(data, BASE_MS1 + SF_CTL);	
}

/**
 * [isEnable] 1: enable ECC or CRC
 *            0: disable
 */
void sfEccENSwitch (u32 isEnable)
{
	u32 data;
	
	data  = readl(BASE_MS1 + SF_CTL);
	data &= ~(SF_CTL_ECCEN);
	data |= (isEnable << 7);

	writel(data, BASE_MS1 + SF_CTL);	
}

/**
 * [isEnable] 1: enable SF_CRC_CAL
 *            0: disable
 */
void sfCrcENSwitch(u32 isEnable)
{
	u32 data;
	
	data  = readl(BASE_MS1 + SF_CTL);
	data &= ~(SF_CTL_CRCEN);
	data |= (isEnable << 9);
	
	writel(data, BASE_MS1 + SF_CTL);	
}

/**
 * Return 1: mass-storage is ready for read/write
 */
int sfCheckMsRdy (void)
{
	int data;
	
	data  = readl(BASE_MS1 + SF_CTL);
	data &= SF_CTL_MSRDY;
	data  = (data >> 8);
	
	return data;
}

/**
 * [31:24], speed selection for SPI/SD(Range: 0 ~ 255)
 */
void sfSetMsSpeed (u32 msspeed)
{
	u32 data;
	
	data  = readl(BASE_MS1 + SF_CTL);
	data &= ~(SF_CTL_MSSPEED);
	data |= (msspeed << 24);
	
	writel(data, BASE_MS1 + SF_CTL);	
}

/**
 * 0x04
 * SF DMA_SIZE APIs
 */
void sfSetDmaSize (u32 size)
{
	u32 data;
	
	data  = readl(BASE_MS1 + SF_DMA_SIZE);
	data &= ~(SF_DMASIZE_MSDMASIZE);	
	data |= (size << 0);
	
	writel(data, BASE_MS1 + SF_DMA_SIZE);	
}

/**
 * [18:17]
 * 00: Normal Serial flash
 * 01: The AA_B(1bytes one time) DMA function for SPI flash
 * 10: The AA_W(2bytes one time) DMA function for SPI flash 
 * 11: Reserved
 */
void sfSetWMode (u32 sfwmode)
{
	u32 data;
	
	data  = readl(BASE_MS1 + SF_DMA_SIZE);
	data &= ~(SF_CTL_W_MODE);
	data |= (sfwmode << 17);
	
	writel(data, BASE_MS1 + SF_DMA_SIZE);	
}

/**
 * [31:24]
 * AA_W command for serial flash DMA mode
 */
void sfSetWCmd (u32 sfCmd)
{
	u32 data;
	
	data  = readl(BASE_MS1 + SF_DMA_SIZE);
	data &= ~(SF_W_CMD_AAWDMA);	
	data |= (sfCmd << 24);
	
	writel(data, BASE_MS1 + SF_DMA_SIZE);	
}

/**
 * 0x08
 * SF CRC APIs
 */
u32 sfReadCrc16 (void)
{
	u32 data;
	
	data  = readl(BASE_MS1 + SF_CRC);
	data &= (SF_CRC_CRC16_L | SF_CRC_CRC16_H);
	data  = (data >> 0);
	
	return data;
}

/**
 * 0x48
 * DMA BLOCK APIs
 *
 * [15:0]
 * DMA Block number at every Multiple DMA, DMA_BLOCK + 1
 */
void sfDmaBlock (u32 dmaBlockNum)
{
	u32 data;
	
	data  = readl(BASE_MS1 + SF_DMA_BLKSU);
	data &= ~(SF_BLKSU_BLK);
	data |= (dmaBlockNum << 0);
	
	writel(data, BASE_MS1 + SF_DMA_BLKSU);	
}

/**
 * [31:16] 
 * Succeed DMA Block transfer number at every DMA
 */
u32 sfReadSUDmaBlock (void)
{
	u32 data;
	
	data  = readl(BASE_MS1 + SF_DMA_BLKSU);
	data &= SF_BLKSU_SUBLK;
	data  = (data >> 16);
	
	return data;
}

/**
 * 0x4c
 * TMCNT APIs
 *
 * [29:0] 
 * Time out counter up limit when M_DMA_EN = 1
 * Time is = clk_cycle * (TIME_CNT * 4)
 */
void sfSetTimeCount (u32 timeCnt)
{
	u32 data;
	
	data  = readl(BASE_MS1 + SF_TMCNT);		
	data &= ~(SF_TMCNT_TMCNT);								 
	data |= (timeCnt << 0);
	
	writel(data, BASE_MS1 + SF_TMCNT);	 
}

/**
 * 0x50
 * MDMAECC APIs
 *
 * [0]
 * 1: Multiple DMA enable
 */
void sfMsMDmaENSwitch (u32 isEnable)
{
	u32 data;
	
	data  = readl(BASE_MS1 + SF_MDMAECC);
	data &= ~(SF_MS_M_DMA_EN);
	data |= (isEnable >> 0);
	
	writel(data, BASE_MS1 + SF_MDMAECC);	
}

/**
 * [1]
 * 1: Multiple DMA finish and without error
 */
int sfCheckMsMDmaOk (void)
{
	int data;
	
	data  = readl(BASE_MS1 + SF_MDMAECC);
	data &= SF_MS_M_DMA_OK;
	data  = (data >> 1);
	
	return data;	
}

/**
 * [2]
 * 1: Multiple DMA exit with time out
 *    Writing "1" to clear this flag
 */
int sfCheckMsMDmaTimeOut (void)
{
	int data;
	
	data  = readl(BASE_MS1 + SF_MDMAECC);
	data &= SF_MS_M_DMA_TIME_OUT;
	data  =(data >> 2);
	
	return data;	
}

/**
 * [8] 
 * 1: Enable MS RDY interrupt
 */
void sfMsRdyIntrENSwitch (u32 isEnable)
{
	u32 data;
	
	data  = readl(BASE_MS1 + SF_MDMAECC);
	data &= ~(SF_MS_RDY_INTR_EN);
	data |= (isEnable << 8);
	
	writel(data, BASE_MS1 + SF_MDMAECC);	
}

/**
 * [9] 
 * 1: Enable MS error interrupt
 */
void sfMsErrIntrENSwitch (u32 isEnable)
{
	u32 data;
	
	data  = readl(BASE_MS1 + SF_MDMAECC);
	data &= ~(SF_MS_ERR_INTR_EN);
	data |= (isEnable << 9);
	
	writel(data, BASE_MS1 + SF_MDMAECC);	
}

/**
 * [16]
 * Indicates that MS DMA is complete
 */
int sfCheckMsRdyFlag (void)
{
	int data;
	
	data  = readl(BASE_MS1 + SF_MDMAECC);
	data &= SF_MS_RDY_FLAG;
	data  = (data >> 16);
	
	return data;	
}

/**
 * [17]
 * Indicates that error occurs on AHB bus
 */
int sfCheckMsErrFlag (void)
{
	int data;
	
	data = readl(BASE_MS1 + SF_MDMAECC);
	data &= SF_MS_ERR_FLAG;
	data = (data >> 17);
	
	return data;	
}

/**
 * [24]
 * Clear MS_RDY_FLAG
 * Write 1 to clear this flag
 */
void sfClearMsRdyFlag (u32 value)
{
	u32 data;
	
	data  = readl(BASE_MS1 + SF_MDMAECC);
	data &= ~(SF_CLR_MS_RDY);
	data |= (value << 24);
	
	writel(data, BASE_MS1 + SF_MDMAECC);
} 

/**
 * [25]
 * Clear MS_ERR_FLAG
 * Write 1 to clear this flag
 */
void sfClearMsErrFlag (u32 value)
{
	u32 data;
	
	data  = readl(BASE_MS1 + SF_MDMAECC);
	data &= ~(SF_CLR_MS_ERR);
	data |= (value << 25);
	
	writel(data, BASE_MS1 + SF_MDMAECC);		
} 

/**
 * 0x54
 * LBA APIs
 *
 * [22:9]
 * For NandFlash:
 *   Logic block address for DMA and extra enable
 */
void sfSetLba (u32 value)
{
	u32 data;
	
	data  = readl(BASE_MS1 + SF_LBA);
	data &= ~(SF_LBA_LBAW);
	data |= (value << 9);
	
	writel(data, BASE_MS1 + SF_LBA);	
}

/**
 * 0x5c
 * DMA ADDR APIs
 *
 * MS_DMA start address
 * Please set MS_DMA_ADDR[1:0] = 0 to meet 4 byte alignment
 */
void sfSetDmaAddr (u32 addr)
{
	writel(addr, BASE_MS1 + SF_DMA_ADDR);
}

/**
 * 0x74
 * MS_REG2
 *
 * write Data
 */
void sfWriteData (u32 value)
{
	writel(value, BASE_MS1 + SF_REG_DATA);
}

/**
 * read Data
 */
u32 sfReadData (void)
{
	return readl(BASE_MS1 + SF_REG_DATA);
}

/**
 * 0x38
 * 
 * MS_IO_EN [8] 
 * controlls sf's CS
 */
void sfMsIO_OE8Switch (u32 isEnable)
{
	u32 data; 
	
	data  = readl(BASE_MS1 + SF_MS_IO_OE);

	data &= ~(SF_MS_IO_OE_8);
	data |= (isEnable << 8);
	
	writel(data, BASE_MS1 + SF_MS_IO_OE);	
	 
}

/**
 * 0x34
 *
 * MS_IO_O [8] 
 * output data when MS_IO[8] is GPIO output mode
 */
void sfMsIO_O8Switch (u32 isEnable)
{
	u32 data;
	
	data  = readl(BASE_MS1 + SF_MS_IO_O);

	data &= ~(SF_MS_IO_O_8);
	data |= (isEnable << 8);

	writel(data, BASE_MS1 + SF_MS_IO_O);	
}

void sfMsIO_OE2Switch (u32 isEnable)
{
	u32 data; 
	
	data  = readl(BASE_MS1 + SF_MS_IO_OE);

	data &= ~(SF_MS_IO_OE_2);
	data |= (isEnable << 2);
	
	writel(data, BASE_MS1 + SF_MS_IO_OE);	
	 
}

void sfMsIO_O2Switch (u32 isEnable)
{
	u32 data;
	
	data  = readl(BASE_MS1 + SF_MS_IO_O);

	data &= ~(SF_MS_IO_O_2);
	data |= (isEnable << 2);

	writel(data, BASE_MS1 + SF_MS_IO_O);	
}


void sfMsIO_OE1Switch (u32 isEnable)
{
	u32 data; 
	
	data  = readl(BASE_MS1 + SF_MS_IO_OE);

	data &= ~(SF_MS_IO_OE_1);
	data |= (isEnable << 1);
	
	writel(data, BASE_MS1 + SF_MS_IO_OE);	
	 
}

void sfMsIO_O1Switch (u32 isEnable)
{
	u32 data;
	
	data  = readl(BASE_MS1 + SF_MS_IO_O);

	data &= ~(SF_MS_IO_O_1);
	data |= (isEnable << 1);

	writel(data, BASE_MS1 + SF_MS_IO_O);	
}



void gpIO_OE0Switch (u32 isEnable)
{
	u32 data; 
	
	data  = readl(0x98100008);

	data &= ~(0x1);
	data |= (isEnable << 0);
	
	writel(data, 0x98100008);	
	 
}

void gpIO_O0Switch (u32 isEnable)
{
	u32 data;
	
	data  = readl(0x98100000);

	data &= ~(0x1);
	data |= (isEnable << 0);

	writel(data, 0x98100000);
}



//#ifdef CONFIG_SYSTEM_PLATFORM_ST58660FPGA
#if defined(CONFIG_SYSTEM_PLATFORM_ST58660FPGA) || defined(CONFIG_SYSTEM_PLATFORM_SN98660) || defined(CONFIG_SYSTEM_PLATFORM_SN98661) || defined (CONFIG_SYSTEM_PLATFORM_SN98670) || defined (CONFIG_SYSTEM_PLATFORM_SN98671) || defined (CONFIG_SYSTEM_PLATFORM_SN98672) || defined (CONFIG_SYSTEM_PLATFORM_SN98293)
/**
 * 0x38
 * 
 * MS_IO_EN [3] 
 * controlls sf's CS
 */
void sfMsIO_OE3Switch (u32 isEnable)
{
	u32 data; 
	
	data  = readl(BASE_MS1 + SF_MS_IO_OE);

	data &= ~(SF_MS_IO_OE_3);
	data |= (isEnable << 3);

	writel(data, BASE_MS1 + SF_MS_IO_OE);	
	 
}

/**
 * 0x34
 *
 * MS_IO_O [3] 
 * output data when MS_IO[3] is GPIO output mode
 */
void sfMsIO_O3Switch (u32 isEnable)
{
	u32 data;
	
	data  = readl(BASE_MS1 + SF_MS_IO_O);

	data &= ~(SF_MS_IO_O_3);
	data |= (isEnable << 3);
	
	writel(data, BASE_MS1 + SF_MS_IO_O);	
 }

#endif /* #ifdef CONFIG_SYSTEM_PLATFORM_ST58660FPGA*/

void sfChipDisable(void)
{ 
//#ifdef CONFIG_SYSTEM_PLATFORM_ST58660FPGA
#if defined(CONFIG_SYSTEM_PLATFORM_ST58660FPGA) || defined(CONFIG_SYSTEM_PLATFORM_SN98660) || defined(CONFIG_SYSTEM_PLATFORM_SN98661) || defined (CONFIG_SYSTEM_PLATFORM_SN98670) || defined (CONFIG_SYSTEM_PLATFORM_SN98671) || defined (CONFIG_SYSTEM_PLATFORM_SN98672) || defined (CONFIG_SYSTEM_PLATFORM_SN98293)
	sfMsIO_OE3Switch(ENABLE);
	sfMsIO_O3Switch(SF_CS_DISABLE);
#else
	sfMsIO_OE8Switch(ENABLE);
	sfMsIO_O8Switch(SF_CS_DISABLE); 	
#endif
}
 
void sfChipEnable (void)
{ 
//#ifdef CONFIG_SYSTEM_PLATFORM_ST58660FPGA
#if defined(CONFIG_SYSTEM_PLATFORM_ST58660FPGA) || defined(CONFIG_SYSTEM_PLATFORM_SN98660) || defined(CONFIG_SYSTEM_PLATFORM_SN98661) || defined (CONFIG_SYSTEM_PLATFORM_SN98670) || defined (CONFIG_SYSTEM_PLATFORM_SN98671) || defined (CONFIG_SYSTEM_PLATFORM_SN98672) || defined (CONFIG_SYSTEM_PLATFORM_SN98293)
	sfMsIO_OE3Switch(ENABLE);
	sfMsIO_O3Switch(SF_CS_ENABLE);
#else
	sfMsIO_OE8Switch(ENABLE);
	sfMsIO_O8Switch(SF_CS_ENABLE); 	
#endif

}




//MSSF_SPI_CMD_OFFSET func === 0x3c
void sfSetMdmaStartAddr(unsigned int addr)
{
	writel(addr, BASE_MS1 + SF_SPI_CMD);
}

void sfSetRDDummyByte(unsigned int n)
{
	unsigned int data;

	data  = readl(BASE_MS1 + SF_MDMA_BYTE);
	data &= ~(SF_READ_DUMMY_BYTE);
	data |= (n << 0);
	writel(data, BASE_MS1 + SF_MDMA_BYTE);
}

void sfSetWRDummyByte(unsigned int n)
{
	unsigned int data;
	

	data  = readl(BASE_MS1 + SF_MDMA_BYTE);
	data &= ~(SF_WRITE_DUMMY_BYTE);
	 data |= (n << 4);
	writel(data, BASE_MS1 + SF_MDMA_BYTE);
}

void sfSetAddrCyc(unsigned int n)
{
	unsigned int data;

	data  = readl(BASE_MS1 + SF_MDMA_BYTE);

	data &= ~(SF_ADDR_CYC);
	data |= (n << 8);

	writel(data, BASE_MS1 + SF_MDMA_BYTE);
}

void sfSetDummyEN(unsigned int en)
{
	unsigned int data;

	data  = readl(BASE_MS1 + SF_4BIT);
	data &= ~(SF_DUMMY_EN);
	data |= (en<<12);

	writel(data, BASE_MS1 + SF_4BIT);
}

void sfSetDummyCyc(unsigned int cyc)
{
	unsigned int data;

	data  = readl(BASE_MS1 + SF_4BIT);
	data &= ~(SF_DUMMY_CYC);
	data |= (cyc<<16);

	writel(data, BASE_MS1 + SF_4BIT);
}

void sfSetCacheWcmd(unsigned int cmd)
{
	unsigned int data;
	
	data  = readl(BASE_MS1 + SF_DMA_SIZE);
	data &= ~(SF_CACHE_W_CMD);
	data |= (cmd << 24);

	writel(data, BASE_MS1 + SF_DMA_SIZE);
}

void sfSetCacheRcmd(unsigned int cmd)
{
	unsigned int data;

	data  = readl(BASE_MS1 + SF_CACHE_RW_CMD);
	data &= ~(SF_CACHE_R_CMD);
	data |= (cmd << 8);
	
	writel(data, BASE_MS1 + SF_CACHE_RW_CMD);
}

void sfSetErasecmd(unsigned int cmd)
{
	unsigned int data;
	
	data  = readl(BASE_MS1 + SF_CACHE_RW_CMD);
	data &= ~(SF_ERASE_CMD);
	data |= (cmd << 24);
	
	writel(data, BASE_MS1 + SF_CACHE_RW_CMD);
}

//MSSF_WEN_CMD_OFFSET func === 0x28
void sfSetWEnCmd(unsigned int cmd)
{
	unsigned int data;
	
	data  = readl(BASE_MS1 + SF_WEN_CMD);
	data &= ~(SF_W_EN_CMD);
	data |= (cmd << 16);

	writel(data, BASE_MS1 + SF_WEN_CMD);
}

void sfSetStatusCmd(unsigned int cmd)
{
	unsigned int data;

	data  = readl(BASE_MS1 + SF_WEN_CMD);
	data &= ~(SF_STATUS_CMD);
	data |= (cmd << 24);
	
	writel(data, BASE_MS1 + SF_WEN_CMD);
}


/**
 * [isEnable] 1: enable CMD_NIB_TRG
 *            0: disable CMD_NIB_TRG
 */
void sfMsCmdNibTrg (u32 isEnable)
{
	u32 data;
	
	data  = readl(BASE_MS1 + SF_CTL);
	data &= ~(SF_CTL_CMDNIBTRG);
	data |= (isEnable << 11);

	writel(data, BASE_MS1 + SF_CTL);	
}
