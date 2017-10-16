/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2014        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include <FreeRTOS.h>
#include <bsp.h>
#include <libmid_sd/mid_sd.h>
#include "diskio.h"		/* FatFs lower layer API */
//#include "usbdisk.h"	/* Example: Header file of existing USB MSD control module */
//#include "atadrive.h"	/* Example: Header file of existing ATA harddisk control module */
//#include "sdcard.h"		/* Example: Header file of existing MMC/SDC contorl module */

#include <usbh/USBH.h>
#include <usbh/MSC.h>
/* Definitions of physical drive number for each drive */
#define ATA		0	/* Example: Map ATA harddisk to physical drive 0 */
#define MMC		1	/* Example: Map MMC/SD card to physical drive 1 */
#define USB		2	/* Example: Map USB MSD to physical drive 2 */

uint8_t global_status = STA_NOINIT;

volatile uint32_t disable_diskio_t = 0;

/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	BYTE pdrv		/* Physical drive nmuber to identify the drive */
)
{
	DSTATUS stat = RES_ERROR;
	int result;

	USBH_Device_Structure *dev = NULL;
	MSC_DEVICE_STRUCT *msc_dev = NULL;
	msc_dev = (MSC_DEVICE_STRUCT*) &MSC_DEV;

	switch (pdrv) {
	case ATA :
		//result = ATA_disk_status();

		// translate the reslut code here
		return 0;//stat;

	case MMC :
		result = mid_sd_get_card_status(MID_SD_BLOCK, NULL);

		// translate the reslut code here
		if(result == MID_SD_QUEUE_CARD_REMOVED)
			global_status = STA_NODISK;
		if(result == MID_SD_QUEUE_CARD_WRITEPROTECT)
			global_status = STA_PROTECT;

		return global_status;

	case USB :
		msc_check_dev_sts(dev, msc_dev);
		if (msc_dev->status == MSC_DISCONNECT) {
			stat = RES_NOTRDY;
		}
		else if (msc_dev->status == MSC_ACTIVE) {
			stat = RES_OK;
		}

		return stat;
	}
	return STA_NOINIT;
}



/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
	BYTE pdrv				/* Physical drive nmuber to identify the drive */
)
{
	DSTATUS stat = RES_ERROR;
	int result;

	switch (pdrv) {
	case ATA :
		//result = ATA_disk_initialize();

		// translate the reslut code here

		return stat;

	case MMC :	
		result = mid_sd_identify(MID_SD_BLOCK, NULL);
		if(result==MID_SD_QUEUE_FINISH)
		{	
			global_status &= (~STA_NOINIT); 
			stat = RES_OK;
		}	
		else
		{
			stat = RES_NOTRDY;			
			MID_FF_PRINT_QUEUE("%s: fail\n",__func__);
		}	
		return stat;
		
	case USB :
		if(msc_ready() == SUCCESS)
		{
			stat = RES_OK;
		}
		else
		{
			stat = RES_NOTRDY;
		}

		// translate the reslut code here

		return stat;
	}

	return STA_NOINIT;
}

/*-----------------------------------------------------------------------*/
/* reset global status                                                   */
/*-----------------------------------------------------------------------*/

void disk_Reset (void)
{
	global_status = STA_NOINIT;
}


/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	BYTE pdrv,		/* Physical drive nmuber to identify the drive */
	BYTE *buff,		/* Data buffer to store read data */
	DWORD sector,	/* Sector address in LBA */
	UINT count		/* Number of sectors to read */
)
{
	DRESULT res=0;
	int result;
	USBH_Device_Structure *dev = NULL;

	unsigned int ddr_addr = (unsigned int) buff;
	ddr_addr = ((ddr_addr)&0x1FF);

	switch (pdrv) {
	case ATA :
		// translate the arguments here

		//result = ATA_disk_read(buff, sector, count);

		// translate the reslut code here

		return res;

	case MMC :
		result = mid_sd_read(buff,sector,count,MID_SD_BLOCK,NULL);
		if(result == MID_SD_QUEUE_FINISH)
			res = RES_OK;
		else
		{
			res = RES_NOTRDY;
			MID_FF_PRINT_QUEUE("%s: fail\n",__func__);
		}	
		return res;
		
	case USB :
		if (disable_diskio_t == 0) {
			dev = (USBH_Device_Structure*) msc_init(1);
			if (dev == NULL) {
				return RES_NOTRDY;
			}

			if (ddr_addr != 0) {
				print_msg ("xxxxxxxxxxxxxxxxxxxx Rad=%x\n",buff);
			}

			if (msc_read(dev, (uint8_t*) buff, count*512, sector) == SUCCESS) {
				res = RES_OK;
			}
			else {
				res = RES_ERROR;
			}
		}else {
			res = RES_ERROR;
			print_msg ("xxxxxxxxxxxxxxxxxxxx disable_diskio_t = 1");
		}

		return res;
	}

	return RES_PARERR;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if _USE_WRITE
DRESULT disk_write (
	BYTE pdrv,			/* Physical drive nmuber to identify the drive */
	const BYTE *buff,	/* Data to be written */
	DWORD sector,		/* Sector address in LBA */
	UINT count			/* Number of sectors to write */
)
{
	DRESULT res=0;
	int result;
	USBH_Device_Structure *dev = NULL;

	unsigned int ddr_addr = (unsigned int) buff;

	ddr_addr = ((ddr_addr)&0x1FF);

	switch (pdrv) {
	case ATA :
		// translate the arguments here

		//result = ATA_disk_write(buff, sector, count);

		// translate the reslut code here

		return res;

	case MMC :
			result = mid_sd_write(sector,(void*)buff, count,MID_SD_BLOCK, NULL);
			if(result == MID_SD_QUEUE_FINISH)
			{
				res = RES_OK;
			}
			else if(result == MID_SD_QUEUE_CARD_REMOVED)
			{
				res = RES_NOTRDY;
				MID_FF_PRINT_QUEUE("%s: No SD card\n", __func__);
			}
			else if(result == MID_SD_QUEUE_CARD_WRITEPROTECT)
			{
				res = RES_WRPRT;
				MID_FF_PRINT_QUEUE("%s: SD card is write protected\n", __func__);
			}
			else
			{
				res = RES_ERROR;
				MID_FF_PRINT_QUEUE("%s: R/W failed\n", __func__);
			}
		return res;

	case USB :
		if (disable_diskio_t == 0) {
			dev = (USBH_Device_Structure*) msc_init(1);

		
			if (dev == NULL) {
				return RES_NOTRDY;
			}

			if (ddr_addr != 0) {
				print_msg ("xxxxxxxxxxxxxxxxxxxx Wad=%x\n",buff);
			}


			if (msc_write(dev, (uint8_t*) buff, count*512, sector) == SUCCESS) {
				res = RES_OK;
			}
			else {
				res = RES_ERROR;
			}


		}else {
			res = RES_ERROR;
			print_msg ("xxxxxxxxxxxxxxxxxxxx disable_diskio_t = 1");
		}
		return res;
	}

	return RES_PARERR;
}
#endif


/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

#if _USE_IOCTL
DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE cmd,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
	DRESULT res=RES_ERROR;
	//int result;

	switch (pdrv) {
	case ATA :

		// Process of the command for the ATA drive

		return res;

	case MMC :

		// Process of the command for the MMC/SD card
		if(cmd==CTRL_SYNC)
		{
			if(MID_SD_QUEUE_FINISH == mid_sd_sync())
				res = RES_OK;
			else
			{
				res = RES_ERROR;
				MID_FF_PRINT_QUEUE("%s: fail\n",__func__);
			}
		}
		else if(cmd == GET_SECTOR_COUNT)
		{
			//if(MID_SD_QUEUE_FINISH == mid_sd_get_capacity( (uint32_t*)buff, 1, NULL))
			if(MID_SD_QUEUE_FINISH == mid_sd_get_capacity( (uint32_t*)buff, MID_SD_BLOCK, NULL))
				res = RES_OK;
			else
			{
				res = RES_ERROR;
				MID_FF_PRINT_QUEUE("%s: fail\n",__func__);
			}
			//MID_FF_PRINT_QUEUE("size = %x\n",  *((DWORD*)buff));
		}
		return res;

	case USB :
		
		// Process of the command the USB drive
		if(cmd==CTRL_SYNC)
		{
			res = RES_OK;
		}
		else if(cmd == GET_SECTOR_COUNT)
		{
			uint8_t status = FAIL;
			USBH_Device_Structure *dev = NULL;
			MSC_REQ_Struct msc_req;
			memset(&msc_req, 0, sizeof(msc_req));
			msc_req.lba = 0;

			dev = (USBH_Device_Structure*) msc_init(1);
			if (dev == NULL) {
				return RES_NOTRDY;
			}

			status = msc_get_capacity((USBH_Device_Structure*)dev, (MSC_REQ_Struct*)&msc_req);
			if(status == SUCCESS) {
				buff = msc_req.lba;
				if(buff > 0)
					res = RES_OK;
				else
					res = RES_ERROR;
			}
			else {
				res = RES_ERROR;
				MID_FF_PRINT_QUEUE("%s: fail\n",__func__);
			}
		}
		return res;
	}

	return RES_PARERR;
}
#endif



void disable_diskio(void) 
{
	 disable_diskio_t = 1;
}
