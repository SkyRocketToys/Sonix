#ifndef __SF_H__ 
#define __SF_H__ 
#include "generated/snx_sdk_conf.h"

#define BASE_MS1            (0x90900000)
#ifndef ENABLE
#define ENABLE	1
#endif

#ifndef DISABLE
#define	DISABLE	0
#endif

//--------------------------------
//Register definition!
//--------------------------------
#define	SF_CTL      		0x00	
#define	SF_DMA_SIZE			0x04 	
#define	SF_CRC				0x08
#define	SF_MS_IO_I			0x30
#define	SF_MS_IO_O			0x34
#define	SF_MS_IO_OE			0x38	
#define	SF_DMA_BLKSU		0x48
#define	SF_TMCNT			0x4c	
#define	SF_MDMAECC			0x50
#define	SF_LBA				0x54	
#define	SF_DMA_ADDR			0x5c
#define	SF_REG_DATA			0x74

//SN98660
#define SF_MDMA_BYTE        0x20
#define SF_CACHE_RW_CMD     0x24
#define SF_WEN_CMD          0x28
#define SF_4BIT             0x2c
#define SF_SPI_CMD          0x3c

//--------------------------------
//SF_CTL 0x00 				
//--------------------------------
#define SF_CTL_MODE			0x00000007
#define SF_CTL_REGRW		0x00000008
#define SF_CTL_DMAEN		0x00000010
#define SF_CTL_DMARW		0x00000020
#define SF_CTL_EXTRAEN		0x00000040
#define SF_CTL_ECCEN		0x00000080
#define SF_CTL_MSRDY		0x00000100
#define SF_CTL_CRCEN		0x00000200
#define SF_CTL_MSSPEED		0xff000000

#define SF_CTL_CMDNIBTRG    0x00000800

//--------------------------------
//SF_DMA_SIZE 0x04
//--------------------------------		
#define SF_DMASIZE_MSDMASIZE	0x00000fff
#define SF_CTL_W_MODE			0x00060000
#define SF_W_CMD_AAWDMA 		0xff000000	

//--------------------------------
//SF_CRC 0x08
//--------------------------------
#define SF_CRC_CRC16_L			0x000000ff
#define SF_CRC_CRC16_H			0x0000ff00

//--------------------------------
//SD_MS_IO 0x30 ~ 0x38   
//--------------------------------
#define SF_MS_IO_I_I			0x00007fff	
#define SF_MS_IO_O_O			0x00007fff	
#define SF_MS_IO_OE_OE			0x00007fff	
#define SF_MS_IO_O_3			0x00000008 // CS
#define SF_MS_IO_OE_3			0x00000008 // CS  
#define SF_MS_IO_O_8			0x00000100 // CS
#define SF_MS_IO_OE_8			0x00000100 // CS            	
#define SF_MS_IO_O_2            0x00000004 // HOLD
#define SF_MS_IO_OE_2           0x00000004 // HOLD
#define SF_MS_IO_O_1            0x00000002 // WP#
#define SF_MS_IO_OE_1           0x00000002 // WP#
//--------------------------------
//SF_DMA_BLKSU 0x48
//--------------------------------
#define SF_BLKSU_BLK			0x0000ffff
#define SF_BLKSU_SUBLK			0xffff0000

//--------------------------------
//SD_TMCNT 0x4c		  
//--------------------------------
#define SF_TMCNT_TMCNT			0x3fffffff	

//--------------------------------
//SF_MDMAECC 0x50
//--------------------------------
#define SF_MS_M_DMA_EN			0x00000001
#define SF_MS_M_DMA_OK			0x00000002
#define SF_MS_M_DMA_TIME_OUT	0x00000004

#define SF_MS_RDY_INTR_EN		0x00000100 
#define SF_MS_ERR_INTR_EN		0x00000200

#define SF_MS_RDY_FLAG			0x00010000
#define SF_MS_ERR_FLAG			0x00020000

#define SF_CLR_MS_RDY			0x01000000
#define SF_CLR_MS_ERR			0x02000000
#define SF_CLR_ECC_ERR			0x04000000

//-------------------------------
//SF LBA sd useful bits 0x54
//-------------------------------
#define SF_LBA_LBAW			0x007ffe00	



//SN98660
//--------------------------------
//SF_MDMA_BYTE 0x20
//--------------------------------
#define SF_READ_DUMMY_BYTE      0x0000000F
#define SF_WRITE_DUMMY_BYTE     0x000000F0
#define SF_ADDR_CYC             0x00000100
#define SF_CMD_NIBBLE           0x00007000

//--------------------------------
//SF_CACHE_RW_CMD 0x24
//--------------------------------
#define SF_CACHE_R_CMD          0x0000FF00
#define SF_CACHE_W_CMD          0xFF000000
#define SF_ERASE_CMD            0xFF000000

//--------------------------------
//SF_W_EN_CMD 0x28
//--------------------------------
#define SF_W_EN_CMD             0x00FF0000
#define SF_STATUS_CMD           0xFF000000

//--------------------------------
//SF_4BIT/Double Rate 0x2c
//--------------------------------
#define SCK_PHS_SEL             0x7
#define SF_QPI_MODE             0x200
#define NF_SF_4BITS             0x400
#define SF_DT_EN                0x800
#define SF_DUMMY_EN             0x1000
#define SF_DUMMY_CYC            0x70000

//--------------------------------
//MSSF_MDMAECC 0x50
//--------------------------------
#define     MS_SD_ECC_FLAG_CLEAR            (0x1<<26)

//--------------------------------
//SF_DMA_SIZE 0x04
//--------------------------------      
#define SF_CTL_W_MODE           0x00060000
#define SF_W_CMD_AAWDMA         0xff000000  
#define SF_CACHE_W_CMD          0xFF000000

//--------------------------------
//Others
//-------------------------------- 
#define MSSF_ENABLE             1
#define MSSF_DISABLE            0
#define MSSF_CHIP_ENABLE        0
#define MSSF_CHIP_DISABLE       1
#define MSSF_WRITE_MODE         0x0
#define MSSF_READ_MODE          0x1
#define MSSF_SF_MODE        0x3




void sfSetMsMode (unsigned mode); 
void sfMsRegRWSwitch (u32 value); 
void sfMsDmaENSwitch (u32 isEnable);
void sfMsDmaRWSwitch (u32 value);
void sfExtraENSwitch (u32 isEnable);
void sfEccENSwitch (u32 isEnable);
int sfCheckMsRdy (void);
void sfCrcENSwitch(u32 isEnable);
void sfSetWMode (u32 sfwmode);
void sfSetMsSpeed (u32 msspeed);
void sfSetDmaSize (u32 size);
u32 sfReadCrc16 (void);
void sfSetLba (u32 value);
void sfSetWCmd (u32 sfCmd);
void sfDmaBlock (u32 dmaBlockNum);
u32 sfReadSUDmaBlock (void);
void sfSetTimeCount (u32 timeCnt);
void sfSetReadcmdCount (u32 readCmdCnt);
void sfMsMDmaENSwitch (u32 isEnable); 
int sfCheckMsMDmaOk (void);
int sfCheckMsMDmaTimeOut (void);
void sfMsRdyIntrENSwitch (u32 isEnable);
int sfCheckMsRdyFlag (void);
void sfClearMsRdyFlag (u32 value);
void sfMsErrIntrENSwitch (u32 isEnable);
int sfCheckMsErrFlag (void); 
void sfClearMsErrFlag (u32 value);
void sfSetDmaAddr (u32 addr);
void sfWriteData (u32 value);
u32 sfReadData (void);
void sfMsGpioOE0Switch (u32 isEnable);
void sfMsGpioO0Switch (u32 isEnable);
void sfChipDisable (void);
void sfChipEnable (void);

//SN98660
void sfSetMdmaStartAddr(unsigned int addr);
void sfSetRDDummyByte(unsigned int n);
void sfSetWRDummyByte(unsigned int n);
void sfSetAddrCyc(unsigned int n);
void sfSetDummyEN(unsigned int en);
void sfSetDummyCyc(unsigned int cyc);
void sfSetCacheWcmd(unsigned int cmd);
void sfSetCacheRcmd(unsigned int cmd);
void sfSetErasecmd(unsigned int cmd);
void sfSetWEnCmd(unsigned int cmd);
void sfSetStatusCmd(unsigned int cmd);
void sfMsCmdNibTrg (u32 isEnable);

void sfMsIO_OE8Switch (u32 isEnable);
void sfMsIO_O8Switch (u32 isEnable);
void sfMsIO_OE2Switch (u32 isEnable);
void sfMsIO_O2Switch (u32 isEnable);
void sfMsIO_OE1Switch (u32 isEnable);
void sfMsIO_O1Switch (u32 isEnable);



#endif
