#include "common.h"

#include "ms_sf.h"	
#include "sonix_sf.h"

#include "generated/snx_flashlayout_conf.h"

//--------------------------------
#define SF_CTL_MODE_SF		0x3
#define SF_READ_MODE		0x1
#define SF_WRITE_MODE		0x0
#define SF_NORMAL			0x0
#define SF_AAI				0x1
#define SF_AAW				0x2			
#define SF_SE				0x0
#define SF_BE				0x1
#define SF_RETRY_CNT        0x05
#define SF_TSS_DETECT_CNT   0x08
#define SF_ALL_DETECT_CNT   0x10

#define PATTERN_TYPE 1  // 1: 0x0 -> 0xff; 0: 0xff -> 0x0

#define MSSF_ADDR_1(offset) ((offset >> 16) & 0xff)
#define MSSF_ADDR_2(offset) ((offset >> 8)  & 0xff)
#define MSSF_ADDR_3(offset) (offset & 0xff)

#define IMAGE_TABLE_ENTRY_SIZE 		20

static struct serial_flash *serial_flash_current = NULL;

#define VERBOSE 0

#if 0
int CRC16_R[16];
u32 CRC_CORRECT;

static void sf_crc_init(void)
{
	int i;
	
	for (i = 0; i < 16; i++)
	{
		CRC16_R[i] = 0;
	}
}

static void sf_crc_cal(u32 si_data, int *CRC16_R)
{
	int i, j;
	u32 crc = 0x0;
	int  CRC16[16], si[8];

	for (i = 0; i < 16; i++)
	{
		CRC16[i] = *(CRC16_R + i);
	}

	for (i = 0; i < 8; i++){
		si[i] = (si_data & (0x1 << i)) >> i;	
	}

	for (i = 7; i >= 0; i--){

		for (j = 0; j < 16; j++){
			*(CRC16_R + j) = CRC16[j];
			CRC16[j] = 0;
		}	
			
		CRC16[0] = *(CRC16_R + 15) ^ si[i];

		CRC16[4] = *(CRC16_R + 3);
		CRC16[3] = *(CRC16_R + 2);
		CRC16[2] = *(CRC16_R + 1);
		CRC16[1] = *(CRC16_R + 0);		

		CRC16[5] = *(CRC16_R + 4) ^ *(CRC16_R + 15) ^ si[i];

		CRC16[11] = *(CRC16_R + 10);
		CRC16[10] = *(CRC16_R + 9);
		CRC16[9] = *(CRC16_R + 8);		
		CRC16[8] = *(CRC16_R + 7);		
		CRC16[7] = *(CRC16_R + 6);		
		CRC16[6] = *(CRC16_R + 5);		

		CRC16[12] = *(CRC16_R + 11) ^ *(CRC16_R + 15) ^ si[i];

		CRC16[15] = *(CRC16_R + 14);		
		CRC16[14] = *(CRC16_R + 13);		
		CRC16[13] = *(CRC16_R + 12);		
	}

	for (j = 0; j < 16; j++){		
		crc |= (CRC16[j] << j);
	}

	for (j = 0; j < 16; j++){
		*(CRC16_R + j) = CRC16[j];
		CRC16[j] = 0;
	}	

	CRC_CORRECT = crc;
}
#endif

static void sf_WREN(struct sf_instruction *sf_inst)
{
	sfChipEnable();
	sfMsRegRWSwitch(SF_WRITE_MODE);
	
	sfWriteData(sf_inst->WREN);
	
	while (1) {
		if(sfCheckMsRdy()) {
			break;
		}
	}
	
	sfChipDisable();
}

static u32 sf_RDSR(struct sf_instruction *sf_inst)
{
	u32 status = 0x0;
	
	sfChipEnable();
	
	sfMsRegRWSwitch(SF_WRITE_MODE);
	sfWriteData(sf_inst->RDSR);
	while (1) {
		if(sfCheckMsRdy()) {
			break;
		}
	}
	
	sfMsRegRWSwitch(SF_READ_MODE);
	sfWriteData(0x0); 
	while (1) {
		if(sfCheckMsRdy()) {
			break;
		}
	}
	status = sfReadData();
	
	sfChipDisable();
	
	return status;
}

#if 0
static void sf_WRSR(struct sf_instruction *sf_inst)
{
	sfChipEnable();
	sfMsRegRWSwitch(SF_WRITE_MODE);
	sfWriteData(sf_inst->WRSR);
	while (1) {
		if(sfCheckMsRdy()) {
			break;
		}
	}
	
	/**
	 * write data
	 */
	sfWriteData(0x0);
	while (1) {
		if (sfCheckMsRdy()) {
			break;
		}
	}
	sfChipDisable();
}

static void sf_WRSR_SST(struct sf_instruction *sf_inst)
{
	sfChipEnable ();
	sfMsRegRWSwitch(SF_WRITE_MODE);
	sfWriteData(sf_inst->EWSR);
	while (1) {
		if (sfCheckMsRdy()) {
			break;
		}
	}
	sfChipDisable();

	sfChipEnable();
	sfMsRegRWSwitch(SF_WRITE_MODE);
	sfWriteData(sf_inst->WRSR);
	while (1) {
		if (sfCheckMsRdy()) {
			break;
		}
	}
	
	/**
	 * write addr
	 */
	sfWriteData(0x0);
	while (1) {
		if (sfCheckMsRdy()) {
			break;
		}
	}
	sfChipDisable();
}
#endif

static void sf_initial(void)
{	
	sfSetMsMode(SF_CTL_MODE_SF);
	sfChipDisable();
	sfExtraENSwitch(DISABLE);
	sfEccENSwitch(DISABLE);
	sfSetMsSpeed(0x1);
}

/******************************************************************
 * read ID type
 ******************************************************************/
	
static int sf_readID1(struct sf_instruction *sf_inst, u32 id[])
{
	sfChipEnable();
	sfMsRegRWSwitch(SF_WRITE_MODE);
	sfWriteData(sf_inst->RDID); 
	while (1) {
		if (sfCheckMsRdy()) {
			break;
		}
	} 
	
	/**
	 * read Manufacturer ID
	 */
	sfMsRegRWSwitch(SF_READ_MODE);
	sfWriteData(0x0); 
	while (1) {
		if (sfCheckMsRdy()) {
			break;
		}
	}	
	id[0] = sfReadData();

	/**
	 * read Memory Type ID
	 */
	sfMsRegRWSwitch(SF_READ_MODE);
	sfWriteData(0x0); 
	while (1) {
		if (sfCheckMsRdy()) {
			break;
		}
	}	
	id[1] = sfReadData();

	/**
	 * read device ID
	 */
	sfMsRegRWSwitch(SF_READ_MODE);
	sfWriteData(0x0); 
	while (1) {
		if (sfCheckMsRdy()) {
			break;
		}
	}	
	id[2] = sfReadData();

	sfChipDisable();

	return 0;
}

static int sf_readID2(struct sf_instruction *sf_inst, u32 id[])
{
	sfChipEnable();
	sfMsRegRWSwitch(SF_WRITE_MODE);
	sfWriteData(sf_inst->RDID); 
	while (1) {
		if (sfCheckMsRdy()) {
			break;
		}
	} 
	
	/**
	 * dummy bytes 3 times
	 */
	sfWriteData(0x0); 
	while (1) {
		if (sfCheckMsRdy()) {
			break;
		}
	} 
	sfWriteData(0x0); 
	while (1) {
		if (sfCheckMsRdy()) {
			break;
		}
	} 
	sfWriteData(0x0); 
	while (1) {
		if (sfCheckMsRdy()) {
			break;
		}
	} 
	
	/**
	 * read ID
	 */
	sfMsRegRWSwitch(SF_READ_MODE);
	sfWriteData(0x0); 
	while (1) {
		if (sfCheckMsRdy()) {
			break;
		}
	}	
	id[0] = sfReadData();
	
	sfMsRegRWSwitch(SF_READ_MODE);
	sfWriteData(0x0); 
	while (1) {
		if (sfCheckMsRdy()) {
			break;
		}
	}	
	id[1] = sfReadData();
	
	sfMsRegRWSwitch(SF_READ_MODE);
	sfWriteData(0x0); 
	while (1) {
		if (sfCheckMsRdy()) {
			break;
		}
	}	
	id[2] = sfReadData();

	sfChipDisable();

	return 0;
}

static int sf_readID3(struct sf_instruction *sf_inst, u32 id[])
{
	sfChipEnable();
	sfMsRegRWSwitch(SF_WRITE_MODE);
	sfWriteData(sf_inst->RDID); 
	while (1) {
		if(sfCheckMsRdy()) {
			break;
		}
	} 
	
	/**
	 * write addr
	 */
	sfWriteData(0x0);
	while (1){
		if(sfCheckMsRdy()){
			break;
		}
	}
	sfWriteData(0x0);
	while (1){
		if(sfCheckMsRdy()){
			break;
		}
	}
	sfWriteData(0x0);
	while (1){
		if(sfCheckMsRdy()){
			break;
		}
	}	
	
	/**
	 * read Manufacture ID
	 */
	sfMsRegRWSwitch(SF_READ_MODE);
	sfWriteData(0x0); 
	while (1) {
		if(sfCheckMsRdy()) {
			break;
		}
	}	
	id[0] = sfReadData();

	/**
	 * read Device ID
	 */
	sfMsRegRWSwitch(SF_READ_MODE);
	sfWriteData(0x0); 
	while (1) {
		if(sfCheckMsRdy()) {
			break;
		}
	}	
	id[1] = sfReadData();
	
	sfChipDisable();

	return 0;
}

///////add on 2013-8-30
/******************************************************************
 * serial flash clear write protected
 ******************************************************************/
static int sf_clear_WRSR(struct sf_instruction *sf_inst, u32 id0, u32 id1, u32 id2)
{
	u32 status;

#if 0
	sfChipEnable();	
	sfMsRegRWSwitch(SF_WRITE_MODE);
	sfWriteData(0x06);
	while (1) {
		if(sfCheckMsRdy()) {
			break;
		}
	}
	sfChipDisable();


	sfChipEnable();	
	sfMsRegRWSwitch(SF_WRITE_MODE);
	sfWriteData(0x01);
	while (1) {
		if(sfCheckMsRdy()) {
			break;
		}
	}
	
	sfWriteData(0x80); 
	while (1) {
		if(sfCheckMsRdy()) {
			break;
		}
	}
	sfChipDisable();



	sfChipEnable();
	
	sfMsRegRWSwitch(SF_WRITE_MODE);
	sfWriteData(0x05);
	while (1) {
		if(sfCheckMsRdy()) {
			break;
		}
	}
	
	sfMsRegRWSwitch(SF_READ_MODE);
	sfWriteData(0x0); 
	while (1) {
		if(sfCheckMsRdy()) {
			break;
		}
	}
	status = sfReadData();
	serial_printf("status = %x\n",status);
	sfChipDisable();



	sfChipEnable();
	
	sfMsRegRWSwitch(SF_WRITE_MODE);
	sfWriteData(0x35);
	while (1) {
		if(sfCheckMsRdy()) {
			break;
		}
	}
	
	sfMsRegRWSwitch(SF_READ_MODE);
	sfWriteData(0x0); 
	while (1) {
		if(sfCheckMsRdy()) {
			break;
		}
	}
	status = sfReadData();
	serial_printf("status = %x\n",status);
	sfChipDisable();


	sfChipEnable();
	
	sfMsRegRWSwitch(SF_WRITE_MODE);
	sfWriteData(0x15);
	while (1) {
		if(sfCheckMsRdy()) {
			break;
		}
	}
	
	sfMsRegRWSwitch(SF_READ_MODE);
	sfWriteData(0x0); 
	while (1) {
		if(sfCheckMsRdy()) {
			break;
		}
	}
	status = sfReadData();
	serial_printf("status = %x\n",status);
	sfChipDisable();

#else

	if (sf_inst->MF == 0x03) //MX
	{
		sfChipEnable();	
		sfMsRegRWSwitch(SF_WRITE_MODE);
		sfWriteData(0x06);
		while (1) {
			if(sfCheckMsRdy()) {
				break;
			}
		}
		sfChipDisable();
	
	
		sfChipEnable();	
		sfMsRegRWSwitch(SF_WRITE_MODE);
		sfWriteData(0x01);
		while (1) {
			if(sfCheckMsRdy()) {
				break;
			}
		}
		
		sfWriteData(0x00); 
		while (1) {
			if(sfCheckMsRdy()) {
				break;
			}
		}
	
		sfWriteData(0x00); 
		while (1) {
			if(sfCheckMsRdy()) {
				break;
			}
		}
	
	
		sfChipDisable();

	
		sfChipEnable();
		
		sfMsRegRWSwitch(SF_WRITE_MODE);
		sfWriteData(0x05);
		while (1) {
			if(sfCheckMsRdy()) {
				break;
			}
		}
		
		sfMsRegRWSwitch(SF_READ_MODE);
		sfWriteData(0x0); 
		while (1) {
			if(sfCheckMsRdy()) {
				break;
			}
		}
		status = sfReadData();
		serial_printf("status = %x\n",status);
	
	
		sfWriteData(0x0); 
		while (1) {
			if(sfCheckMsRdy()) {
				break;
			}
		}
		status = sfReadData();
		serial_printf("configs = %x\n",status);
	
		sfChipDisable();
	}
	else if (sf_inst->MF == 0x04)	//GD
	{
	if ((id0 == 0xc8)&&(id1 == 0x40)&&(id2 == 0x15)) {
		sfChipEnable();	
		sfMsRegRWSwitch(SF_WRITE_MODE);
		sfWriteData(0x06);
		while (1) {
			if(sfCheckMsRdy()) {
				break;
			}
		}
		sfChipDisable();
		
		sfChipEnable();	
		sfMsRegRWSwitch(SF_WRITE_MODE);
		sfWriteData(0x01);
		while (1) {
			if(sfCheckMsRdy()) {
				break;
			}
		}
		
		//sfWriteData(0x80); 
		sfWriteData(0x03); 
		while (1) {
			if(sfCheckMsRdy()) {
				break;
			}
		}

		sfWriteData(0x00); 
		while (1) {
			if(sfCheckMsRdy()) {
				break;
			}
		}


		sfChipDisable();

		sfChipEnable();		
		sfMsRegRWSwitch(SF_WRITE_MODE);
		sfWriteData(0x05);
		while (1) {
			if(sfCheckMsRdy()) {
				break;
			}
		}
		
		sfMsRegRWSwitch(SF_READ_MODE);
		sfWriteData(0x0); 
		while (1) {
			if(sfCheckMsRdy()) {
				break;
			}
		}
		status = sfReadData();
		serial_printf("status-1 = %x\n",status);
		sfChipDisable();
	
		////////////////////////////////////////
	
		sfChipEnable();		
		sfMsRegRWSwitch(SF_WRITE_MODE);
		sfWriteData(0x35);
		while (1) {
			if(sfCheckMsRdy()) {
				break;
			}
		}
		
		sfMsRegRWSwitch(SF_READ_MODE);
		sfWriteData(0x0); 
		while (1) {
			if(sfCheckMsRdy()) {
				break;
			}
		}
		status = sfReadData();
		serial_printf("status-2 = %x\n",status);
		sfChipDisable();	


	}
	if ((id0 == 0xc8)&&(id1 == 0x40)&&(id2 == 0x18)) {

		sfChipEnable();	
		sfMsRegRWSwitch(SF_WRITE_MODE);
		sfWriteData(0x06);
		while (1) {
			if(sfCheckMsRdy()) {
				break;
			}
		}
		sfChipDisable();
	
	
		sfChipEnable();	
		sfMsRegRWSwitch(SF_WRITE_MODE);
		sfWriteData(0x01);
		while (1) {
			if(sfCheckMsRdy()) {
				break;
			}
		}
		
		sfWriteData(0x83); 
		while (1) {
			if(sfCheckMsRdy()) {
				break;
			}
		}
		sfChipDisable();

		////////////////////////////////
		sfChipEnable();	
		sfMsRegRWSwitch(SF_WRITE_MODE);
		sfWriteData(0x06);
		while (1) {
			if(sfCheckMsRdy()) {
				break;
			}
		}
		sfChipDisable();
	
	
		sfChipEnable();	
		sfMsRegRWSwitch(SF_WRITE_MODE);
		sfWriteData(0x31);
		while (1) {
			if(sfCheckMsRdy()) {
				break;
			}
		}
		
		sfWriteData(0x00); 
		while (1) {
			if(sfCheckMsRdy()) {
				break;
			}
		}
		sfChipDisable();

		////////////////////////////////
		sfChipEnable();	
		sfMsRegRWSwitch(SF_WRITE_MODE);
		sfWriteData(0x06);
		while (1) {
			if(sfCheckMsRdy()) {
				break;
			}
		}
		sfChipDisable();
	
	
		sfChipEnable();	
		sfMsRegRWSwitch(SF_WRITE_MODE);
		sfWriteData(0x11);
		while (1) {
			if(sfCheckMsRdy()) {
				break;
			}
		}
		
		sfWriteData(0x00); 
		while (1) {
			if(sfCheckMsRdy()) {
				break;
			}
		}
		sfChipDisable();

		////////////////////////////////////////
	
		sfChipEnable();		
		sfMsRegRWSwitch(SF_WRITE_MODE);
		sfWriteData(0x05);
		while (1) {
			if(sfCheckMsRdy()) {
				break;
			}
		}
		
		sfMsRegRWSwitch(SF_READ_MODE);
		sfWriteData(0x0); 
		while (1) {
			if(sfCheckMsRdy()) {
				break;
			}
		}
		status = sfReadData();
		serial_printf("status-1 = %x\n",status);
		sfChipDisable();
	
		////////////////////////////////////////
	
		sfChipEnable();		
		sfMsRegRWSwitch(SF_WRITE_MODE);
		sfWriteData(0x35);
		while (1) {
			if(sfCheckMsRdy()) {
				break;
			}
		}
		
		sfMsRegRWSwitch(SF_READ_MODE);
		sfWriteData(0x0); 
		while (1) {
			if(sfCheckMsRdy()) {
				break;
			}
		}
		status = sfReadData();
		serial_printf("status-2 = %x\n",status);
		sfChipDisable();	
		
		////////////////////////////////////////
	
		sfChipEnable();		
		sfMsRegRWSwitch(SF_WRITE_MODE);
		sfWriteData(0x15);
		while (1) {
			if(sfCheckMsRdy()) {
				break;
			}
		}
		
		sfMsRegRWSwitch(SF_READ_MODE);
		sfWriteData(0x0); 
		while (1) {
			if(sfCheckMsRdy()) {
				break;
			}
		}
		status = sfReadData();
		serial_printf("status-3 = %x\n",status);
		sfChipDisable();
	}
	else {
		sfChipEnable();
		sfMsRegRWSwitch(SF_WRITE_MODE);

		do {
			status = sf_RDSR(sf_inst);
		}while ((status & 0x1) != 0x0);

		sf_WREN (sf_inst);
		do {
			status = sf_RDSR (sf_inst);
		} while ((status & 0x3) != 0x2);

		sfChipEnable();
		sfMsRegRWSwitch(SF_WRITE_MODE);
		sfWriteData(sf_inst->WRSR); 
		while (1) {
			if(sfCheckMsRdy()) {
				break;
			}
		} 
		
		/**
		 * write addr
		 */
		sfWriteData(0x0);
		while (1){
			if(sfCheckMsRdy()){
				break;
			}
		}

		sfChipDisable ();

		status = sf_RDSR (sf_inst);
		serial_printf("status = %x\n",status);
	}

#if 0
		////////////////////////////////
		sfChipEnable();	
		sfMsRegRWSwitch(SF_WRITE_MODE);
		sfWriteData(0x06);
		while (1) {
			if(sfCheckMsRdy()) {
				break;
			}
		}
		sfChipDisable();
	
	
		sfChipEnable();	
		sfMsRegRWSwitch(SF_WRITE_MODE);
		sfWriteData(0x31);
		while (1) {
			if(sfCheckMsRdy()) {
				break;
			}
		}
		
		sfWriteData(0x00); 
		while (1) {
			if(sfCheckMsRdy()) {
				break;
			}
		}
		sfChipDisable();

		////////////////////////////////
		sfChipEnable();	
		sfMsRegRWSwitch(SF_WRITE_MODE);
		sfWriteData(0x06);
		while (1) {
			if(sfCheckMsRdy()) {
				break;
			}
		}
		sfChipDisable();
	
	
		sfChipEnable();	
		sfMsRegRWSwitch(SF_WRITE_MODE);
		sfWriteData(0x11);
		while (1) {
			if(sfCheckMsRdy()) {
				break;
			}
		}
		
		sfWriteData(0x00); 
		while (1) {
			if(sfCheckMsRdy()) {
				break;
			}
		}
		sfChipDisable();
#endif

		////////////////////////////////////////
	

		
		////////////////////////////////////////
#if 0	
		sfChipEnable();		
		sfMsRegRWSwitch(SF_WRITE_MODE);
		sfWriteData(0x15);
		while (1) {
			if(sfCheckMsRdy()) {
				break;
			}
		}
		
		sfMsRegRWSwitch(SF_READ_MODE);
		sfWriteData(0x0); 
		while (1) {
			if(sfCheckMsRdy()) {
				break;
			}
		}
		status = sfReadData();
		serial_printf("status-3 = %x\n",status);
		sfChipDisable();
#endif	
	} 
	else{		
		sfChipEnable(); 	   
		sfMsRegRWSwitch(SF_WRITE_MODE);
		sfWriteData(sf_inst->WREN);
		while (1) {
			if(sfCheckMsRdy()) {
				break;
			}
		}
		sfChipDisable();
		
		do{
			status = sf_RDSR (sf_inst);
		} while ((status & 0x3) != 0x2);

		sfChipEnable(); 	   
		sfMsRegRWSwitch(SF_WRITE_MODE);
		sfWriteData(sf_inst->WRSR);
		while (1) {
			if(sfCheckMsRdy()) {
				break;
			}
		}
		
		sfWriteData(0); 
		while (1) {
			if(sfCheckMsRdy()) {
				break;
			}
		}
		sfChipDisable();
		
		sfChipEnable();
		
		sfMsRegRWSwitch(SF_WRITE_MODE);
		sfWriteData(sf_inst->RDSR);
		while (1) {
			if(sfCheckMsRdy()) {
				break;
			}
		}
		
		sfMsRegRWSwitch(SF_READ_MODE);
		sfWriteData(0x0); 
		while (1) {
			if(sfCheckMsRdy()) {
				break;
			}
		}
		status = sfReadData();
		serial_printf("status = %x\n",status);

		sfChipDisable();	
	}	
#endif	

	return 0;
}
static int sf_ProtectSR(struct sf_instruction *sf_inst)
{
#if 0
	u32 status;
	//Set serial flash CE = 0
	sfChipEnable();
	sfMsRegRWSwitch(SF_WRITE_MODE);
	
	sf_WREN(sf_inst);
	
	do {
		status = sf_RDSR(sf_inst);
	}while ((status & 0x3) != 0x2);
	
	sfChipEnable();
	sfMsRegRWSwitch(SF_WRITE_MODE);
	
	sfWriteData(sf_inst->WRSR); 
	while (1) {
		if(sfCheckMsRdy()) {
			break;
		}
	} 
	
	/**
	 * write addr
	 */
	if (strncmp(serial_flash_current->name, "SPANSION", 8) == 0) {
		sfWriteData(0x1c);
	} else if (strncmp(serial_flash_current->name, "MXIC&AMIC", 9) == 0) {
		sfWriteData(0x3c);
	} else if (strncmp(serial_flash_current->name, "GD", 2) == 0) {
		sfWriteData(0x7c);
	}
	while (1){
		if(sfCheckMsRdy()){
			break;
		}
	}

	sfChipDisable ();
#endif
	return 0;
}
///////end 

/******************************************************************
 * serial flash erase
 ******************************************************************/
#if defined(CONFIG_SYSTEM_PLATFORM_ST58660FPGA) || defined(CONFIG_SYSTEM_PLATFORM_SN98660) || defined(CONFIG_SYSTEM_PLATFORM_SN98661) || defined (CONFIG_SYSTEM_PLATFORM_SN98670) || defined (CONFIG_SYSTEM_PLATFORM_SN98671) || defined (CONFIG_SYSTEM_PLATFORM_SN98672) || defined (CONFIG_SYSTEM_PLATFORM_SN98293)
static int sf_sector_erase(struct sf_instruction *sf_inst, u32 offset)
{
	//	u32 status = 0x0;

	// 	do {
	// 	status = sf_RDSR(sf_inst);
	// }while ((status & 0x1) == 0x1);

	sf_WREN(sf_inst);

	sfChipEnable ();

	sfSetWEnCmd(sf_inst->WREN);
	sfSetStatusCmd(sf_inst->RDSR);

	sfSetErasecmd(sf_inst->SE);
	sfSetMdmaStartAddr(offset);

	sfMsCmdNibTrg (MSSF_ENABLE);

		while (1) {
		if (sfCheckMsRdy ()) {
			break;
		}
	}

	sfMsCmdNibTrg (MSSF_DISABLE);

	sfChipDisable ();	

	return 0;
}
#else
static int sf_sector_erase(struct sf_instruction *sf_inst, u32 offset)
{
	u32 status = 0x0;


		do {
		status = sf_RDSR(sf_inst);
	}while ((status & 0x1) == 0x1);


	sf_WREN(sf_inst);

	sfChipEnable();
	sfMsRegRWSwitch(SF_WRITE_MODE);
	sfWriteData(sf_inst->SE);
	while (1) {
		if (sfCheckMsRdy()) {
			break;
		}
	}

	/**
	 * write addr
	 */
	sfWriteData(MSSF_ADDR_1(offset));
	while (1) {
		if (sfCheckMsRdy()) {
			break;
		}
	}
	sfWriteData(MSSF_ADDR_2(offset));
	while (1) {
		if (sfCheckMsRdy()) {
			break;
		}
	}
	sfWriteData(MSSF_ADDR_3(offset));
	while (1) {
		if (sfCheckMsRdy()) {
			break;
		}
	}
	sfChipDisable();

	return 0;
}
#endif

/******************************************************************
 * serial flash erase
 ******************************************************************/
#if defined(CONFIG_SYSTEM_PLATFORM_ST58660FPGA) || defined(CONFIG_SYSTEM_PLATFORM_SN98660) || defined(CONFIG_SYSTEM_PLATFORM_SN98661) || defined (CONFIG_SYSTEM_PLATFORM_SN98670) || defined (CONFIG_SYSTEM_PLATFORM_SN98671) || defined (CONFIG_SYSTEM_PLATFORM_SN98672) || defined (CONFIG_SYSTEM_PLATFORM_SN98293)
static int sf_block_erase(struct sf_instruction *sf_inst, u32 offset)
{
	//	u32 status = 0x0;


	// 	do {
	// 	status = sf_RDSR(sf_inst);
	// }while ((status & 0x1) == 0x1);


	sf_WREN(sf_inst);

	sfChipEnable ();

	sfSetWEnCmd(sf_inst->WREN);
	sfSetStatusCmd(sf_inst->RDSR);

	sfSetErasecmd(sf_inst->BE);
	sfSetMdmaStartAddr(offset);

	sfMsCmdNibTrg (MSSF_ENABLE);

		while (1) {
		if (sfCheckMsRdy ()) {
			break;
		}
	}


	sfMsCmdNibTrg (MSSF_DISABLE);

	sfChipDisable ();	

	return 0;
}
#else
static int sf_block_erase(struct sf_instruction *sf_inst, u32 offset)
{
	u32 status = 0x0;

			do {
		status = sf_RDSR(sf_inst);
	}while ((status & 0x1) == 0x1);


	sf_WREN(sf_inst);

	sfChipEnable();
	sfMsRegRWSwitch(SF_WRITE_MODE);
	sfWriteData(sf_inst->BE);
	while (1) {
		if (sfCheckMsRdy()) {
			break;
		}
	}

	/**
	 * write addr
	 */
	sfWriteData(MSSF_ADDR_1(offset));
	while (1) {
		if (sfCheckMsRdy()) {
			break;
		}
	}
	sfWriteData(MSSF_ADDR_2(offset));
	while (1) {
		if (sfCheckMsRdy()) {
			break;
		}
	}
	sfWriteData(MSSF_ADDR_3(offset));
	while (1) {
		if (sfCheckMsRdy()) {
			break;
		}
	}
	sfChipDisable();

	return 0;
}
#endif

/******************************************************************
 * CPU write
 ******************************************************************/
 #if 0
static int sf_cpu_write(struct sf_instruction *sf_inst, 
	u32 offset, uchar *addr, u32 len)
{
	int i;
	int retval;
	u32 status, write_data;

	retval = 0;

	do {
		status = sf_RDSR(sf_inst);
	}while ((status & 0x1) != 0x0);
	
	sf_WREN(sf_inst);

	do {
		status = sf_RDSR(sf_inst);
	}while ((status & 0x3) != 0x2);

	sfChipEnable();
	sfMsRegRWSwitch(SF_WRITE_MODE);
	sfWriteData(sf_inst->PP);
	while (1) {
		if (sfCheckMsRdy()) {
			break;
		}
	}

	/**
	 * write addr
	 */
	sfWriteData(MSSF_ADDR_1(offset));
	while (1) {
		if (sfCheckMsRdy()) {
			break;
		}
	}
	sfWriteData(MSSF_ADDR_2(offset));
	while (1){
		if (sfCheckMsRdy()) {
			break;
		}
	}
	sfWriteData(MSSF_ADDR_3(offset));
	while (1) {
		if (sfCheckMsRdy()) {
			break;
		}
	}

#ifdef CRCEK
	/**
	 * CRC calulate enable
	 */
	sfEccENSwitch(DISABLE);
	sfEccENSwitch(ENABLE);
	sfCrcENSwitch(DISABLE);
	sfCrcENSwitch(ENABLE);

	sf_crc_init();

	/** 
	 * write data
	 */
	for (i = 0; i < len; i++) {
#if PATTERN_TYPE
		write_data = addr[i];
#else
		write_data = 0xff - addr[i];
#endif
		sfWriteData(write_data);
		sf_crc_cal(write_data, CRC16_R);
		
		while (1) {
			if (sfCheckMsRdy()) {
				break;
			}
		}
	}

	data = sfReadCrc16();
	if (data == CRC_CORRECT) {
		;
	}
	else {	
		serial_puts("sf_cpu_write_NORMAL: CRC ERROR\n");
		retval = -1;
	}
	sfEccENSwitch(DISABLE);
	sfCrcENSwitch(DISABLE);
	sfChipDisable();
#else
	/** 
	 * write data
	 */
	for (i = 0; i < len; i++) {
#if PATTERN_TYPE
		write_data = addr[i];
#else
		write_data = 0xff - addr[i];
#endif
		sfWriteData(write_data);
		
		while (1) {
			if (sfCheckMsRdy()) {
				break;
			}
		}
	}


	sfChipDisable();
#endif


	do {
		status = sf_RDSR(sf_inst);
	}while ((status & 0x3) != 0x0);

	return retval;
}
#else
static int sf_cpu_write(struct sf_instruction *sf_inst, 
	u32 offset, uchar *addr, u32 len)
{
	int i;
	int retval;
	u32 status, write_data;

	retval = 0;

	do {
		status = sf_RDSR(sf_inst);
	}while ((status & 0x1) != 0x0);
	
	sf_WREN(sf_inst);

	do {
		status = sf_RDSR(sf_inst);
	}while ((status & 0x3) != 0x2);

	sfChipEnable();
	sfMsRegRWSwitch(SF_WRITE_MODE);
	sfWriteData(sf_inst->PP);
	while (1) {
		if (sfCheckMsRdy()) {
			break;
		}
	}

	/**
	 * write addr
	 */
	sfWriteData(MSSF_ADDR_1(offset));
	while (1) {
		if (sfCheckMsRdy()) {
			break;
		}
	}
	sfWriteData(MSSF_ADDR_2(offset));
	while (1){
		if (sfCheckMsRdy()) {
			break;
		}
	}
	sfWriteData(MSSF_ADDR_3(offset));
	while (1) {
		if (sfCheckMsRdy()) {
			break;
		}
	}

#if 0
	/**
	 * CRC calulate enable
	 */
	sfEccENSwitch(DISABLE);
	sfEccENSwitch(ENABLE);
	sfCrcENSwitch(DISABLE);
	sfCrcENSwitch(ENABLE);

	sf_crc_init();

	/** 
	 * write data
	 */
	for (i = 0; i < len; i++) {
#if PATTERN_TYPE
		write_data = addr[i];
#else
		write_data = 0xff - addr[i];
#endif
		sfWriteData(write_data);
		sf_crc_cal(write_data, CRC16_R);
		
		while (1) {
			if (sfCheckMsRdy()) {
				break;
			}
		}
	}

	data = sfReadCrc16();
	if (data == CRC_CORRECT) {
		;
	}
	else {	
		serial_puts("sf_cpu_write_NORMAL: CRC ERROR\n");
		retval = -1;
	}
	sfEccENSwitch(DISABLE);
	sfCrcENSwitch(DISABLE);
	sfChipDisable();
#else
	/** 
	 * write data
	 */
	for (i = 0; i < len; i++) {
		write_data = addr[i];
		sfWriteData(write_data);
		
		while (1) {
			if (sfCheckMsRdy()) {
				break;
			}
		}
	}

	sfChipDisable();
#endif


	// do {
	// 	status = sf_RDSR(sf_inst);
	// }while ((status & 0x3) != 0x0);

	return retval;
}

#endif

static int sf_cpu_read(struct sf_instruction *sf_inst, 
	u32 offset, u32 len)
{
	int i;
	int retval;
	u32 status, write_data;

	retval = 0;

	do {
		status = sf_RDSR(sf_inst);
	}while ((status & 0x1) != 0x0);
	
	sf_WREN(sf_inst);

	do {
		status = sf_RDSR(sf_inst);
	}while ((status & 0x3) != 0x2);

	sfChipEnable();
	sfMsRegRWSwitch(SF_WRITE_MODE);
	sfWriteData(0x03);
	while (1) {
		if (sfCheckMsRdy()) {
			break;
		}
	}

	/**
	 * write addr
	 */
	sfWriteData(MSSF_ADDR_1(offset));
	while (1) {
		if (sfCheckMsRdy()) {
			break;
		}
	}
	sfWriteData(MSSF_ADDR_2(offset));
	while (1){
		if (sfCheckMsRdy()) {
			break;
		}
	}
	sfWriteData(MSSF_ADDR_3(offset));
	while (1) {
		if (sfCheckMsRdy()) {
			break;
		}
	}

#if 0
	/**
	 * CRC calulate enable
	 */
	sfEccENSwitch(DISABLE);
	sfEccENSwitch(ENABLE);
	sfCrcENSwitch(DISABLE);
	sfCrcENSwitch(ENABLE);

	sf_crc_init();

	/** 
	 * write data
	 */
	for (i = 0; i < len; i++) {
#if PATTERN_TYPE
		write_data = addr[i];
#else
		write_data = 0xff - addr[i];
#endif
		sfWriteData(write_data);
		sf_crc_cal(write_data, CRC16_R);
		
		while (1) {
			if (sfCheckMsRdy()) {
				break;
			}
		}
	}

	data = sfReadCrc16();
	if (data == CRC_CORRECT) {
		;
	}
	else {	
		serial_puts("sf_cpu_write_NORMAL: CRC ERROR\n");
		retval = -1;
	}
	sfEccENSwitch(DISABLE);
	sfCrcENSwitch(DISABLE);
	sfChipDisable();
#else
	/** 
	 * read data
	 */
	sfMsRegRWSwitch(SF_READ_MODE);

	for (i = 0; i < len; i++) {
		
		sfWriteData(0x0); 
		while (1) {
			if(sfCheckMsRdy()) {
				break;
			}
		}
	
	status = sfReadData();
	serial_printf("%x,",status);

	if ((i != 1)&&((i%16)==1))
		serial_printf("\n");

	}

	sfChipDisable();
#endif



#if 0

		sfChipEnable();
	
	sfMsRegRWSwitch(SF_WRITE_MODE);
	sfWriteData(0x05);
	while (1) {
		if(sfCheckMsRdy()) {
			break;
		}
	}
	
	sfMsRegRWSwitch(SF_READ_MODE);
	sfWriteData(0x0); 
	while (1) {
		if(sfCheckMsRdy()) {
			break;
		}
	}
	status = sfReadData();
	serial_printf("status = %x\n",status);
	sfChipDisable();



	sfChipEnable();
	
	sfMsRegRWSwitch(SF_WRITE_MODE);
	sfWriteData(0x35);
	while (1) {
		if(sfCheckMsRdy()) {
			break;
		}
	}
	
	sfMsRegRWSwitch(SF_READ_MODE);
	sfWriteData(0x0); 
	while (1) {
		if(sfCheckMsRdy()) {
			break;
		}
	}
	status = sfReadData();
	serial_printf("status = %x\n",status);
	sfChipDisable();


	sfChipEnable();
	
	sfMsRegRWSwitch(SF_WRITE_MODE);
	sfWriteData(0x15);
	while (1) {
		if(sfCheckMsRdy()) {
			break;
		}
	}
	
	sfMsRegRWSwitch(SF_READ_MODE);
	sfWriteData(0x0); 
	while (1) {
		if(sfCheckMsRdy()) {
			break;
		}
	}
	status = sfReadData();
	serial_printf("status = %x\n",status);
	sfChipDisable();
#endif

	// do {
	// 	status = sf_RDSR(sf_inst);
	// }while ((status & 0x3) != 0x0);

	return retval;
}


static int sf_cpu_mread(struct sf_instruction *sf_inst, 
	u32 offset, uchar *addr, u32 len)
{
	int i;
	int retval;
	u32 status, write_data;

	retval = 0;

	// do {
	// 	status = sf_RDSR(sf_inst);
	// }while ((status & 0x1) != 0x0);
	
	// sf_WREN(sf_inst);

	// do {
	// 	status = sf_RDSR(sf_inst);
	// }while ((status & 0x3) != 0x2);

	sfChipEnable();
	sfMsRegRWSwitch(SF_WRITE_MODE);
	sfWriteData(0x03);
	while (1) {
		if (sfCheckMsRdy()) {
			break;
		}
	}

	/**
	 * write addr
	 */
	sfWriteData(MSSF_ADDR_1(offset));
	while (1) {
		if (sfCheckMsRdy()) {
			break;
		}
	}
	sfWriteData(MSSF_ADDR_2(offset));
	while (1){
		if (sfCheckMsRdy()) {
			break;
		}
	}
	sfWriteData(MSSF_ADDR_3(offset));
	while (1) {
		if (sfCheckMsRdy()) {
			break;
		}
	}


	/** 
	 * read data
	 */
	sfMsRegRWSwitch(SF_READ_MODE);

	for (i = 0; i < len; i++) {
		
		sfWriteData(0x0); 
		while (1) {
			if(sfCheckMsRdy()) {
				break;
			}
		}
	

	addr[i] = sfReadData();
	// serial_printf("%x,",addr[i]);

	// if ((i != 1)&&((i%16)==1))
	// 	serial_printf("\n");
	
	}

	sfChipDisable();

	return retval;
}



#define MAX_SPI_DMA_SIZE	4096

/******************************************************************
 * DMA write
 ******************************************************************/
#define MSSF_ENABLE	1
#define MSSF_DISABLE	0

#if 0
static int sf_dma_write(struct sf_instruction *sf_inst, 
	u32 offset, uchar *addr, u32 len)
{
	unsigned int dma_size;
	u32 status;
	
	dma_size = len -1;
	sfMsDmaRWSwitch (SF_WRITE_MODE);
	sfSetDmaAddr ((u32)(addr));	
	sfSetDmaSize (dma_size);

	do {
		status = sf_RDSR(sf_inst);
	}while ((status & 0x1) != 0x0);

	sf_WREN (sf_inst);
	do {
		status = sf_RDSR (sf_inst);
	} while ((status & 0x3) != 0x2);

	//chip enable
	sfChipEnable ();

	//write instruction
 	sfMsRegRWSwitch (SF_WRITE_MODE);
	sfWriteData (sf_inst->PP);
	while (1) {
		if (sfCheckMsRdy ()) {
			break;
		}
	} 

	/**
	 * write addr
	 */
	sfWriteData (MSSF_ADDR_1 (offset));
	while (1) {
		if (sfCheckMsRdy ()) {
			break;
		}
	}
	sfWriteData (MSSF_ADDR_2 (offset));
	while (1) {
		if (sfCheckMsRdy ()) {
			break;
		}
	}
	sfWriteData (MSSF_ADDR_3 (offset));
	while (1) {
		if (sfCheckMsRdy ()) {
			break;
		}
	}

	sfSetWMode (SF_NORMAL);
	//Read data
	//MS_DMA_EN = 1 dma enable      
	
	sfMsDmaENSwitch (MSSF_ENABLE);
	while (1) {
		if (sfCheckMsRdy ()) {
			break;
		}
	}   
	//MS_DMA_EN=0 dma disable
	sfMsDmaENSwitch (MSSF_DISABLE);

	//chip disable
	sfChipDisable ();		

	return 0;
}
#else
static int sf_dma_write(struct sf_instruction *sf_inst, 
	u32 offset, uchar *addr, u32 len)
{
	unsigned int dma_size;
	u32 status;
	
	dma_size = len -1;
	sfMsDmaRWSwitch (SF_WRITE_MODE);
	sfSetDmaAddr ((u32)(addr));	
	sfSetDmaSize (dma_size);

	do {
		status = sf_RDSR(sf_inst);
	}while ((status & 0x1) != 0x0);

	sf_WREN (sf_inst);
	// do {
	// 	status = sf_RDSR (sf_inst);
	// } while ((status & 0x3) != 0x2);

	//chip enable
	sfChipEnable ();

	//write instruction
 	sfMsRegRWSwitch (SF_WRITE_MODE);
	sfWriteData (sf_inst->PP);
	while (1) {
		if (sfCheckMsRdy ()) {
			break;
		}
	} 

	/**
	 * write addr
	 */
	sfWriteData (MSSF_ADDR_1 (offset));
	while (1) {
		if (sfCheckMsRdy ()) {
			break;
		}
	}
	sfWriteData (MSSF_ADDR_2 (offset));
	while (1) {
		if (sfCheckMsRdy ()) {
			break;
		}
	}
	sfWriteData (MSSF_ADDR_3 (offset));
	while (1) {
		if (sfCheckMsRdy ()) {
			break;
		}
	}

	sfSetWMode (SF_NORMAL);
	//Read data
	//MS_DMA_EN = 1 dma enable      
	
	sfMsDmaENSwitch (MSSF_ENABLE);
	while (1) {
		if (sfCheckMsRdy ()) {
			break;
		}
	}   
	//MS_DMA_EN=0 dma disable
	sfMsDmaENSwitch (MSSF_DISABLE);

	//chip disable
	sfChipDisable ();		

	return 0;
}
#endif

 #define	MAX_MDMA_WAIT_COUNT		100000 //100000
static int wait_till_msdma_ok(void)
{
	unsigned int count;

	for (count = 0; count < MAX_MDMA_WAIT_COUNT; count++) {
		if(sfCheckMsMDmaOk())
			return 0;
	}

        return 1;
}



static int sf_mdma_write(struct sf_instruction *sf_inst, 
	u32 offset, uchar *addr, u32 len, u32 mblks)
{
	unsigned int dma_size;
	u32 status;

	u32 transblk = 0;

	do {
		status = sf_RDSR(sf_inst);
	}while ((status & 0x1) != 0x0);

	sfChipEnable ();


	sfSetWMode (SF_NORMAL);

	sfSetAddrCyc(0);

	sfSetRDDummyByte(0);
	sfSetWRDummyByte(0);
	sfSetDummyCyc(0);
	sfSetDummyEN(0);
	sfSetCacheWcmd(sf_inst->PP);
	sfSetCacheRcmd(sf_inst->READ);

	sfSetWEnCmd(sf_inst->WREN);
	sfSetStatusCmd(sf_inst->RDSR);


	dma_size = len -1;
	sfMsDmaRWSwitch (SF_WRITE_MODE);
	sfSetDmaAddr ((u32)(addr));	
	sfSetDmaSize (dma_size);
	sfDmaBlock (mblks-1);
	sfSetMdmaStartAddr(offset);


	

	sfMsMDmaENSwitch (MSSF_ENABLE);
	sfMsDmaENSwitch (MSSF_ENABLE);

	while (1) {
		if (sfCheckMsRdy ()) {
			break;
		}
	}

	if(wait_till_msdma_ok()) {
		return 1;
	}

	transblk = sfReadSUDmaBlock ();
	serial_printf("offset=%x,addr=%x,dma_size=%x,mblks=%x,transblk=%x\n", offset, addr, dma_size, mblks,transblk);

	sfMsDmaENSwitch (MSSF_DISABLE);
	sfMsMDmaENSwitch (MSSF_DISABLE);

	sfChipDisable ();	

	return 0;
}


static int sf_cpu_write_AAI(struct sf_instruction *sf_inst, 
	u32 offset, uchar *addr, u32 len)
{
	int i;
	int retval;
	u32 status, write_data;
	
	retval = 0;
	
	sfSetWMode(SF_NORMAL);

	sf_WREN (sf_inst);

	do {
		status = sf_RDSR(sf_inst);
	}while (status != 0x2);
	
	sfChipEnable();
	sfMsRegRWSwitch(SF_WRITE_MODE);
	sfWriteData(sf_inst->AAI);
	while (1) {
		if (sfCheckMsRdy()) {
			break;
		}
	}
	
	/**
	 * write addr
	 */
	sfWriteData((offset & 0xFF0000) >> 16);
	while (1) {
		if (sfCheckMsRdy()) {
			break;
		}
	}
	sfWriteData((offset & 0xFF00) >> 8);
	while (1) {
		if (sfCheckMsRdy()) {
			break;
		}
	}
	sfWriteData(offset & 0xFF);
	while (1) {
		if (sfCheckMsRdy()) {
			break;
		}
	}	

	/**
	 * CRC calulate enable
	 */
#ifdef CRCEK	 
	sfEccENSwitch(ENABLE);
	sfEccENSwitch(ENABLE);
	sfCrcENSwitch(DISABLE);
	sfCrcENSwitch(ENABLE);

	sf_crc_init();
	
	/**
	 * write data
	 */
	i = 0;
#if PATTERN_TYPE
	write_data = addr[i++];
#else
	write_data = 0xff - addr[i++];	
#endif
	sfWriteData(write_data);
	sf_crc_cal(write_data, CRC16_R);

	while (1) {
		if (sfCheckMsRdy()) {
			break;
		}
	}

	data = sfReadCrc16();
	if (data == CRC_CORRECT) {
		;
	}
	else {
		serial_puts("sf_cpu_write_AAI: CRC ERROR\n");
		retval = -1;
	}		
	sfEccENSwitch(DISABLE);
	sfChipDisable();
#else	
	/**
	 * write data
	 */
	i = 0;
#if PATTERN_TYPE
	write_data = addr[i++];
#else
	write_data = 0xff - addr[i++];	
#endif
	sfWriteData(write_data);

	while (1) {
		if (sfCheckMsRdy()) {
			break;
		}
	}
	sfChipDisable();	
#endif	

	do {
		status = sf_RDSR(sf_inst);
	}while ((status & 0x1) == 0x1);

	/**
	 * after second byte
	 */
	for (; i < len; i++){ 
		sfChipEnable();
		sfMsRegRWSwitch(SF_WRITE_MODE);
		sfWriteData(sf_inst->AAI);
		while (1) {
			if (sfCheckMsRdy()) {
				break;
			}
		}

		/**
		 * CRC calulate enable
		 */
#ifdef CRCEK		 
		sfEccENSwitch(ENABLE);
		sfCrcENSwitch(DISABLE);

		sf_crc_init();
		 
		/**
		 * write data
		 */
#if PATTERN_TYPE
		write_data = addr[i];
#else
		write_data = 0xff - addr[i];	
#endif
		sfWriteData(write_data);
		sf_crc_cal(write_data, CRC16_R);
	
		while (1) {
			if (sfCheckMsRdy()) {
				break;
			}
		}	

		data = sfReadCrc16();
		if (data == CRC_CORRECT) {
			;
		}
		else {
			serial_puts("sf_cpu_write_AAI: CRC ERROR\n");
			retval = -1;
		}
		sfEccENSwitch(DISABLE);
		sfChipDisable();
#else	 
		/**
		 * write data
		 */
#if PATTERN_TYPE
		write_data = addr[i];
#else
		write_data = 0xff - addr[i];	
#endif
		sfWriteData(write_data);
	
		while (1) {
			if (sfCheckMsRdy()) {
				break;
			}
		}	
		sfChipDisable();		
#endif		

		do {
			status = sf_RDSR(sf_inst);
		}while ((status & 0x1) == 0x1);	
	}

	sfChipEnable();
	sfMsRegRWSwitch(SF_WRITE_MODE);
	sfWriteData(sf_inst->WRDI);
	while (1) {
		if (sfCheckMsRdy()) {
			break;
		}
	}
	sfChipDisable ();
	
	return retval;
}

static int sf_cpu_write_AAW (struct sf_instruction *sf_inst,
	u32 offset, uchar *addr, u32 len)
{
	int i,j;	
	int retval;
	u32 status, write_data;

	retval = 0;
	
	sfSetWMode(SF_NORMAL);
	
	sf_WREN(sf_inst);

	do {
		status = sf_RDSR(sf_inst);
	}while (status != 0x2);

	sfChipEnable();
	sfMsRegRWSwitch(SF_WRITE_MODE);
	sfWriteData(sf_inst->AAW);
	while (1) {
		if (sfCheckMsRdy()) {
			break;
		}
	}
	
	/**
	 * write addr
	 */
	sfWriteData((offset & 0xFF0000) >> 16);
	while (1) {
		if (sfCheckMsRdy()) {
			break;
		}
	}
	sfWriteData((offset &0xFF00) >> 8);
	while (1) {
		if (sfCheckMsRdy()) {
			break;
		}
	}
	sfWriteData(offset & 0xFF);
	while (1) {
		if (sfCheckMsRdy()) {
			break;
		}
	}	

#ifdef CRCEK
	/**
	 * CRC calulate enable
	 */
	sfEccENSwitch(DISABLE);
	sfEccENSwitch(ENABLE);
	sfCrcENSwitch(DISABLE);
	sfCrcENSwitch(ENABLE);

	sf_crc_init();
	
	/**
	 * write data
	 *
	 * 1st & 2nd byte
	 */
	i = 0;
	
#if PATTERN_TYPE
	write_data = addr[i++];
#else
	write_data = 0xff - addr[i++];	
#endif
	sfWriteData(write_data);
	sf_crc_cal(write_data, CRC16_R);
		
	while (1) {
		if (sfCheckMsRdy()) {
			break;
		}
	}
	
#if PATTERN_TYPE
	write_data = addr[i++];
#else
	write_data = 0xff - addr[i++];	
#endif
	sfWriteData(write_data);
	sf_crc_cal(write_data, CRC16_R);
	
	while (1) {
		if (sfCheckMsRdy()) {
			break;
		}
	}

	data = sfReadCrc16();
	if (data == CRC_CORRECT) {
		;
	}
	else {
		serial_puts("sf_cpu_write_AAW: CRC ERROR\n");
		retval = -1;		
	}
	sfEccENSwitch(DISABLE);
	sfCrcENSwitch(DISABLE);
	sfChipDisable ();
#else	
	/**
	 * write data
	 *
	 * 1st & 2nd byte
	 */
	i = 0;
	
#if PATTERN_TYPE
	write_data = addr[i++];
#else
	write_data = 0xff - addr[i++];	
#endif
	sfWriteData(write_data);
		
	while (1) {
		if (sfCheckMsRdy()) {
			break;
		}
	}
	
#if PATTERN_TYPE
	write_data = addr[i++];
#else
	write_data = 0xff - addr[i++];	
#endif
	sfWriteData(write_data);
	
	while (1) {
		if (sfCheckMsRdy()) {
			break;
		}
	}
	sfChipDisable ();
#endif
	
	do {
		status = sf_RDSR(sf_inst);
	}while ((status & 0x1) == 0x1);

	/**
	 * after sencond byte
	 */
	for (; i < len; i = i + 2){ 
		sfChipEnable();
		sfMsRegRWSwitch(SF_WRITE_MODE);
		sfWriteData(sf_inst->AAW);
		while (1) {
			if (sfCheckMsRdy()) {
				break;
			}
		}

#ifdef CRCEK
		/**
		 * CRC calulate enable
		 */
		sfEccENSwitch(DISABLE);
		sfEccENSwitch(ENABLE);
		sfCrcENSwitch(DISABLE);
		sfCrcENSwitch(ENABLE);

		sf_crc_init();
		
		for (j = i; ((j < i + 2) && (j < len)); j++) {
#if PATTERN_TYPE
			write_data = addr[j];
#else
			write_data = 0xff - addr[j];	
#endif
			sfWriteData(write_data);
			sf_crc_cal(write_data, CRC16_R);
	
			while (1) {
				if (sfCheckMsRdy()) {
					break;
				}
			}	
		}
		
		data = sfReadCrc16();
		if (data == CRC_CORRECT) {
			;
		}
		else {
			serial_puts("sf_cpu_write_AAW: CRC ERROR\n");
			retval = -1;		
		}
		sfEccENSwitch(DISABLE);
		sfCrcENSwitch(DISABLE);
		sfChipDisable();
#else	
		for (j = i; ((j < i + 2) && (j < len)); j++) {
#if PATTERN_TYPE
			write_data = addr[j];
#else
			write_data = 0xff - addr[j];	
#endif
			sfWriteData(write_data);
	
			while (1) {
				if (sfCheckMsRdy()) {
					break;
				}
			}	
		}
		sfChipDisable();		
#endif	

		do{
			status = sf_RDSR(sf_inst);
		}while ((status & 0x1) == 0x1);	
	}

	sfChipEnable();
	sfMsRegRWSwitch(SF_WRITE_MODE);
	sfWriteData(sf_inst->WRDI);
	while (1) {
		if (sfCheckMsRdy()) {
			break;
		}
	}
	sfChipDisable();

	return retval;
}

/********************************************************************
 ********************************************************************
 ****** Serial flash common init, erase, read, write function *******
 ********************************************************************
 ********************************************************************/
 
static int serial_flash_init(struct serial_flash *sf)
{
	int retval;
	u32 id[3] = {0, 0, 0};

#if defined(CONFIG_SYSTEM_PLATFORM_ST58660FPGA) || defined(CONFIG_SYSTEM_PLATFORM_SN98660) || defined (CONFIG_SYSTEM_PLATFORM_SN98670) || defined (CONFIG_SYSTEM_PLATFORM_SN98671) || defined (CONFIG_SYSTEM_PLATFORM_SN98672) || defined (CONFIG_SYSTEM_PLATFORM_SN98293)
	sfMsIO_OE2Switch (1);
	sfMsIO_O2Switch (1);

	sfMsIO_OE1Switch (1);
	sfMsIO_O1Switch (1);


	gpIO_OE0Switch (1);
	gpIO_O0Switch (1);
#else
	gpIO_OE0Switch (1);
	gpIO_O0Switch (1);	
#endif	

	sf_initial();
	

//#if defined(CONFIG_SYSTEM_PLATFORM_ST58660FPGA) || defined(CONFIG_SYSTEM_PLATFORM_SN98660)
//	sf->clearSR(sf->sf_inst);
	//sf_clear_WRSR(sf->sf_inst);
//#endif

	if(NULL == sf->readID) {
		serial_printf("serial flash %s: serial_flash_readID is NULL\n", 
			sf->name);
		return -EFAULT;
	}
	
	retval = sf->readID(sf->sf_inst, id);
	if(retval) {
		serial_printf("serial flash %s: serial_flash_readID failed\n",
			sf->name);
		return -EFAULT;
	}

  //////add on 2013-8-30
  serial_printf("ID = %x,%x,%x : %x,%x,%x\n",id[0],id[1],id[2],sf->id0,sf->id1,sf->id2);
  //////end
  
	if((sf->id0 != id[0]) || (sf->id1 != id[1]) || (sf->id2 != id[2]))
		return -EFAULT;

//#if defined(CONFIG_SYSTEM_PLATFORM_ST58660FPGA) || defined(CONFIG_SYSTEM_PLATFORM_SN98660)	
  //////add on 2013-8-30
  sf->clearSR(sf->sf_inst,sf->id0,sf->id1,sf->id2);
  //////end
//#endif 

	return 0;
}





static int serial_flash_sector_erase(struct serial_flash *sf, u32 offset, size_t len)
{
	int retval;
	u32 top_addr;

	if(0 == sf->sector_size) {
		serial_printf("serial flash %s: sector size error\n", 
			sf->name);
		return -EFAULT;
	}

	if(NULL == sf->sector_erase) {
		serial_printf("serial flash %s: serial_flash_sector_erase is NULL\n", 
			sf->name);
		return -EFAULT;
	}

	if((offset % sf->sector_size) || (len % sf->sector_size)) {
		serial_printf("serial flash %s: offset/len must multiple of sector size\n", 
			sf->name);
		return -EINVAL;
	}

	top_addr = offset + len;
	
	if(top_addr > sf->chip_size) {
		serial_printf("serial flash %s: end address out of chip size\n", 
			sf->name);
		return -EINVAL;
	}

#if VERBOSE
	serial_printf("Sector Erase:");
#endif

	for(; offset < top_addr; offset += sf->sector_size) {
		retval = sf->sector_erase(sf->sf_inst, offset);
		if(retval) {
			serial_printf("serial flash %s: serial_flash_sector_erase failed\n",
				sf->name);
			return -EFAULT;
		}
#if VERBOSE
		serial_printf(".");
#endif
	}

#if VERBOSE
	serial_printf("\n");
#endif

	return 0;
}

static int serial_flash_block_erase(struct serial_flash *sf, u32 offset, size_t len)
{
	int retval;
	u32 top_addr;

	if(0 == sf->block_size) {
		serial_printf("serial flash %s: block_size size error\n", 
			sf->name);
		return -EFAULT;
	}

	if(NULL == sf->block_size) {
		serial_printf("serial flash %s: serial_flash_block_erase is NULL\n", 
			sf->name);
		return -EFAULT;
	}

	if((offset % sf->block_size) || (len % sf->block_size)) {
		serial_printf("serial flash %s: offset/len must multiple of block size\n", 
			sf->name);
		return -EINVAL;
	}

	top_addr = offset + len;
	
	if(top_addr > sf->chip_size) {
		serial_printf("serial flash %s: end address out of chip size\n", 
			sf->name);
		return -EINVAL;
	}

	serial_printf("block Erase:");

	for(; offset < top_addr; offset += sf->block_size) {
		retval = sf->block_erase(sf->sf_inst, offset);
		if(retval) {
			serial_printf("serial flash %s: serial_flash_block_erase failed\n",
				sf->name);
			return -EFAULT;
		}
#if VERBOSE
		serial_printf(".");
#endif
	}

	serial_printf("\n");

	return 0;
}


/******************************************************************
 * serial flash write
 ******************************************************************/
#if defined(CONFIG_SYSTEM_PLATFORM_ST58660FPGA) || defined(CONFIG_SYSTEM_PLATFORM_SN98660) || defined(CONFIG_SYSTEM_PLATFORM_SN98661) || defined (CONFIG_SYSTEM_PLATFORM_SN98670) || defined (CONFIG_SYSTEM_PLATFORM_SN98671) || defined (CONFIG_SYSTEM_PLATFORM_SN98672) || defined (CONFIG_SYSTEM_PLATFORM_SN98293)
#if 1
static int serial_flash_write(struct serial_flash *sf, u32 offset,
		uchar *addr, size_t len)
{
	int retval;
	u32 page_offset=0,page_len=0;
	u32 top_addr=0;
	u32 mblks = 0;


	if(0 == sf->page_size) {
		serial_printf("serial flash %s: page size error\n", 
			sf->name);
		return -EFAULT;
	}

	if((NULL == sf->cpu_write) || (NULL == sf->dma_write)) {
		serial_printf("serial flash %s: serial_flash_write is NULL\n", 
			sf->name);
		return -EFAULT;
	}

	top_addr = offset + len;

	if(top_addr > sf->chip_size) {
		serial_printf("serial flash %s: end address out of chip size\n", 
			sf->name);
		return -EINVAL;
	}

#if VERBOSE
	serial_printf ("write:");
#endif
	/* align to a page */
	page_offset = offset & (sf->page_size - 1);
	if (page_offset) {
		page_len = min(sf->page_size - page_offset, len);

		retval = sf->cpu_write(sf->sf_inst, offset, addr, page_len);
		if(retval) {
			serial_printf("serial flash %s: serial_flash_cpu_write failed\n", 
				sf->name);
			return -EFAULT;
		} 
		offset += page_len;
		addr += page_len;
		len -= page_len;
		//serial_printf ("write cpu in the begin len = 0x%x, offset = 0x%x, addr = 0x%x, size=0x%x\n", len, offset, (int)addr, len);
#if VERBOSE
		serial_printf (".");
#endif
	}

	
	/* write page */
	mblks = len/sf->page_size;
#if VERBOSE
	serial_printf ("@@ the mblks = %d\n", mblks);
#endif
	if (mblks > 0) {
		retval = sf->mdma_write(sf->sf_inst, offset, addr, sf->page_size,mblks);
		if(retval) {
			serial_printf("serial flash %s: serial_flash_mdma_write failed\n", 
					sf->name);
			return -EFAULT;
		}
	}

	offset += (sf->page_size*mblks);
	addr += (sf->page_size*mblks);
	len -= (sf->page_size*mblks);
#if VERBOSE
	serial_printf ("*");
#endif
	if (len) {
	//	serial_printf ("write cpu in the end len = 0x%x, offset = 0x%x, addr = 0x%x, size=0x%x\n", len, offset, (int)addr, len);
		retval = sf->cpu_write(sf->sf_inst, offset, addr, len);
		if(retval) {
			serial_printf("serial flash %s: serial_flash_cpu_write failed\n", 
				sf->name);
			return -EFAULT;
		}
#if VERBOSE
		serial_printf (".");
#endif
	}
	
#if VERBOSE
	serial_printf("\nSerial Flash Write: done\n");
#endif

	return 0;
}

#else
static int serial_flash_write(struct serial_flash *sf, u32 offset,
		uchar *addr, size_t len)
{
	int retval;
	u32 page_offset=0,page_len=0;
	u32 top_addr=0;

	if(0 == sf->page_size) {
		serial_printf("serial flash %s: page size error\n", 
			sf->name);
		return -EFAULT;
	}

	if((NULL == sf->cpu_write) || (NULL == sf->dma_write)) {
		serial_printf("serial flash %s: serial_flash_write is NULL\n", 
			sf->name);
		return -EFAULT;
	}

	top_addr = offset + len;

	if(top_addr > sf->chip_size) {
		serial_printf("serial flash %s: end address out of chip size\n", 
			sf->name);
		return -EINVAL;
	}

#if VERBOSE
	serial_printf ("write:");
#endif
	/* align to a page */
	page_offset = offset & (sf->page_size - 1);
	if (page_offset) {
		page_len = min(sf->page_size - page_offset, len);

		retval = sf->cpu_write(sf->sf_inst, offset, addr, page_len);
		if(retval) {
			serial_printf("serial flash %s: serial_flash_cpu_write failed\n", 
				sf->name);
			return -EFAULT;
		} 
		offset += page_len;
		addr += page_len;
		len -= page_len;
		//serial_printf ("write cpu in the begin len = 0x%x, offset = 0x%x, addr = 0x%x, size=0x%x\n", len, offset, (int)addr, len);
#if VERBOSE
		serial_printf (".");
#endif
	}

	
	/* write page */
	while (len && (len >= sf->page_size)) {
		retval = sf->dma_write(sf->sf_inst, offset, addr, sf->page_size);
		if(retval) {
			serial_printf("serial flash %s: serial_flash_dma_write failed\n", 
				sf->name);
			return -EFAULT;
		}
		offset += sf->page_size;
		addr += sf->page_size;
		len -= sf->page_size;
#if VERBOSE
		serial_printf (".");
#endif
	}

	if (len) {
	//	serial_printf ("write cpu in the end len = 0x%x, offset = 0x%x, addr = 0x%x, size=0x%x\n", len, offset, (int)addr, len);
		retval = sf->cpu_write(sf->sf_inst, offset, addr, len);
		if(retval) {
			serial_printf("serial flash %s: serial_flash_cpu_write failed\n", 
				sf->name);
			return -EFAULT;
		}
#if VERBOSE
		serial_printf (".");
#endif
	}
	
	serial_printf("\nSerial Flash Write: done\n");


	sf_cpu_read(sf->sf_inst, 0, 64);

	return 0;
}

#endif


#else
static int serial_flash_write(struct serial_flash *sf, u32 offset,
		uchar *addr, size_t len)
{
	int retval;
	u32 page_offset=0,page_len=0;
	u32 top_addr=0;

	if(0 == sf->page_size) {
		serial_printf("serial flash %s: page size error\n", 
			sf->name);
		return -EFAULT;
	}

	if((NULL == sf->cpu_write) || (NULL == sf->dma_write)) {
		serial_printf("serial flash %s: serial_flash_write is NULL\n", 
			sf->name);
		return -EFAULT;
	}

	top_addr = offset + len;

	if(top_addr > sf->chip_size) {
		serial_printf("serial flash %s: end address out of chip size\n", 
			sf->name);
		return -EINVAL;
	}

	serial_printf ("write:");
	/* align to a page */
	page_offset = offset & (sf->page_size - 1);
	if (page_offset) {
		page_len = min(sf->page_size - page_offset, len);

		retval = sf->cpu_write(sf->sf_inst, offset, addr, page_len);
		if(retval) {
			serial_printf("serial flash %s: serial_flash_cpu_write failed\n", 
				sf->name);
			return -EFAULT;
		} 
		offset += page_len;
		addr += page_len;
		len -= page_len;
		//serial_printf ("write cpu in the begin len = 0x%x, offset = 0x%x, addr = 0x%x, size=0x%x\n", len, offset, (int)addr, len);
		serial_printf (".");
	}

	
	/* write page */
	while (len && (len >= sf->page_size)) {
		retval = sf->dma_write(sf->sf_inst, offset, addr, sf->page_size);
		if(retval) {
			serial_printf("serial flash %s: serial_flash_dma_write failed\n", 
				sf->name);
			return -EFAULT;
		}
		offset += sf->page_size;
		addr += sf->page_size;
		len -= sf->page_size;
		serial_printf (".");
	}

	if (len) {
	//	serial_printf ("write cpu in the end len = 0x%x, offset = 0x%x, addr = 0x%x, size=0x%x\n", len, offset, (int)addr, len);
		retval = sf->cpu_write(sf->sf_inst, offset, addr, len);
		if(retval) {
			serial_printf("serial flash %s: serial_flash_cpu_write failed\n", 
				sf->name);
			return -EFAULT;
		}
		serial_printf (".");
	}
	
	serial_printf("\nSerial Flash Write: done\n");

	return 0;
}
#endif



#if 0
static int serial_flash_write(struct serial_flash *sf, u32 offset,
	uchar *addr, size_t len)
{	
	int retval;
	u32 bytes;
	u32 top_addr;
	u32 actual_len;

	if(0 == sf->page_size) {
		serial_printf("serial flash %s: page size error\n", 
			sf->name);
		return -EFAULT;
	}

	if(NULL == sf->write) {
		serial_printf("serial flash %s: serial_flash_write is NULL\n", 
			sf->name);
		return -EFAULT;
	}

	top_addr = offset + len;

	if(top_addr > sf->chip_size) {
		serial_printf("serial flash %s: end address out of chip size\n", 
			sf->name);
		return -EINVAL;
	}

	bytes = offset % sf->page_size;

	serial_printf("Write:");
	for(; offset < top_addr; offset += actual_len) {
		actual_len = min(top_addr - offset,
			sf->page_size - bytes);
		
	 	retval = sf->write(sf->sf_inst, offset, addr, actual_len);
		if(retval) {
			serial_printf("serial flash %s: serial_flash_write failed\n", 
				sf->name);
			return -EFAULT;
		}
		
		serial_printf(".");
		
		bytes = 0;
		addr += actual_len;
	}
	serial_printf("\n");

	return 0;
}
#endif

//////add on 2013-09-2
static int serial_flash_protect(struct serial_flash *sf)
{	
	int retval;

	retval = sf->ProtectSR(sf->sf_inst);
	return 0;
}
//////end

/********************************************************************
 ********************************************************************
 ******************* Serial flash sub-system API ********************
 ********************************************************************
 ********************************************************************/

static struct sf_instruction insts_array[] = {

	[0] = { //SST_AAI
			.WREN	=	0x06,
	  		.WRDI	=	0x04,
	  		.RDID	=	0x90,
			.RDSR	=	0x05,
			.WRSR	=	0x01,
			.READ	=	0x03,
			.PP	=	NONE,
			.SE	=	0x20,
			.BE	=	0x52,
			.DP	=	NONE,
			.RES	=	NONE,
			.AAI	=	0xaf,
			.AAW	=	NONE,
			.BP	=	0x02,
			.EWSR	=	0x50,
			.TBP	=	0xfa,
			.MF 	= 0x00,
		},

	[1] = { //SST_AAW
			.WREN	=	0x06,
			.WRDI	=	0x04,
			.RDID	=	0x90,
			.RDSR	=	0x05,
			.WRSR	=	0x01,
			.READ	=	0x03,
			.PP	=	NONE,
			.SE	=	0x20,
			.BE	=	0x52,
			.DP	=	NONE,
			.RES	=	NONE,
			.AAI	=	NONE,
			.AAW	=	0xad,
			.BP	=	0x02,
			.EWSR	=	0x50,
			.TBP	=	0x7d,
			.MF 	= 0x01,
		},

	[2] = { //SST_PP
			.WREN	=	0x06,
			.WRDI	=	0x04,
			.RDID	=	0x90,
			.RDSR	=	0x05,
			.WRSR	=	0x01,
			.READ	=	0x03,
			.PP	=	0x02,
			.SE	=	0x20,
			.BE	=	0x52, //32KB:0x52; 64KB:0xd8
			.DP	=	NONE,
			.RES	=	NONE,
			.AAI	=	NONE,
			.AAW	=	NONE,
			.BP	=	NONE,
			.EWSR	=	0x50,
			.TBP	=	NONE,
			.MF 	= 0x02,
		},

	[3] = {	//MXIC&AMIC
			.WREN	=	0x06,
			.WRDI	=	0x04,
			.RDID	=	0x9f,
			.RDSR	=	0x05,
			.WRSR	=	0x01,
			.READ	=	0x03,
			.PP	=	0x02,
			.SE	=	0x20,
			.BE	=	0xd8, 	//64KB
			.CE     =	0xc7,
			.DP	=	0xb9,
			.RES	=	0xab,	
			.AAI	=	NONE,  
			.AAW	=	NONE,  
			.BP	=	NONE,
			.EWSR	=	NONE,
			.TBP	=	NONE,
			.MF 	= 0x03,
		},

	[4] = { //GD25Q128C & GD25Q16B & GD25Q32C & GD25Q64C
			.WREN	=	0x06,
			.WRDI	=	0x04,
			.RDID	=	0x9f,
			.RDSR	=	0x05,
			.WRSR	=	0x01,
			.READ	=	0x03,
			.PP	=	0x02,
			.SE	=	0x20,
			.BE	=	0xd8,
			.CE     =	0xc7,
			.DP	=	0xb9,
			.RES	=	0xab,
			.AAI	=	NONE,
			.AAW	=	NONE,
			.BP	=	NONE,
			.EWSR	=	NONE,
			.TBP	=	NONE,
			.MF 	= 0x04,
		},
	[5] = { //PMC
			.WREN	=	0x06,
			.WRDI	=	0x04,
			.RDID	=	0xab,
			.RDSR	=	0x05,
			.WRSR	=	0x01,
			.READ	=	0x03,
			.PP	=	0x02,
			.SE	=	0xd7,
			.BE	=	0x20, //sector:0x20 , block:0xd8
			.DP	=	NONE,
			.RES	=	NONE,
			.AAI	=	NONE,
			.AAW	=	NONE,
			.BP	=	NONE,
			.EWSR	=	NONE,
			.TBP	=	NONE,
			.MF 	= 0x05,
		},

	[6] = { //EN
			.WREN	=	0x06,
			.WRDI	=	0x04,
			.RDID	=	0x9f,
			.RDSR	=	0x05,
			.WRSR	=	0x01,
			.READ	=	0x03,
			.PP	=	0x02,
			.SE	=	0x20, //sector:0x20 , block:0xd8
			.BE	=	0xd8, //use sector erase ,actual 0xc7
			.DP	=	0xb9,
			.RES	=	NONE,
			.AAI	=	NONE,
			.AAW	=	NONE,
			.BP	=	NONE,
			.EWSR	=	NONE,
			.TBP	=	NONE,
			.MF 	= 0x06,
		},

	[7] = { //SPANSION
			.WREN	=	0x06,
			.WRDI	=	0x04,
			.RDID	=	0x9f,
			.RDSR	=	0x05,
			.WRSR	=	0x01,
			.READ	=	0x03,
			.PP	=	0x02,
			.SE	=	0x20,  
			.BE	=	0xd8,
			.DP	=	0xb9,
			.RES	=	0xab,
			.AAI	=	NONE,
			.AAW	=	NONE,
			.BP	=	NONE,
			.EWSR	=	NONE,
			.TBP	=	NONE,
			.MF 	= 0x07,
		},
	[8] = { //WINBOND
			.WREN	=	0x06,
			.WRDI	=	0x04,
			.RDID	=	0x9f,
			.RDSR	=	0x05,
			.WRSR	=	0x01,
			.READ	=	0x03,
			.PP	=	0x02,
			.SE	=	0x20,
			.BE	=	0xd8,
			.DP	=	0xb9,
			.RES	=	0xab,
			.AAI	=	NONE,
			.AAW	=	NONE,
			.BP	=	NONE,
			.EWSR	=	NONE,
			.TBP	=	NONE,
			.MF 	= 0x08,
		},
	[9] = { //KH25L80(16)06E
			.WREN	=	0x06,
			.WRDI	=	0x04,
			.RDID	=	0x9f,
			.RDSR	=	0x05,
			.WRSR	=	0x01,
			.READ	=	0x03,
			.PP	=	0x02,
			.SE	=	0x20,
			.BE	=	0xd8,
			.DP	=	0xb9,
			.RES	=	0xab,
			.AAI	=	NONE,
			.AAW	=	NONE,
			.BP	=	NONE,
			.EWSR	=	NONE,
			.TBP	=	NONE,
			.MF 	= 0x09,
		},
};

static struct serial_flash serial_flash_array[] = {
	{
		.name	= "SST_AAI",	//SST25VF512
		.id0	= 0xBF,
		.id1	= 0x48,
		.id2	= 0,
		.page_size	= 256,		// 256 byte
		.sector_size	= 256 * 16,	// 4K byte
		.block_size	= 256 * 16 * 8,	// 32K byte
		.chip_size	= 256 * 16 * 8 * 2,// 64K byte
		.sf_inst	= &insts_array[0],
		.readID		= sf_readID1,
		.sector_erase	= sf_sector_erase,
		.block_erase	= sf_block_erase,
		.cpu_write		= sf_cpu_write_AAI,
		//////add on 2013-8-30
		.clearSR	= sf_clear_WRSR,
		.ProtectSR	= sf_ProtectSR,
		//////end
	},
	{
		.name	= "SST_AAW",
		.sf_inst	= &insts_array[1],
		.readID		= sf_readID3,
		.sector_erase	= sf_sector_erase,
		.block_erase	= sf_block_erase,
		.cpu_write		= sf_cpu_write_AAW,
		//////add on 2013-8-30
		.clearSR	= sf_clear_WRSR,
		.ProtectSR	= sf_ProtectSR,
		//////end
	},
	{
		.name	= "SST_PP",
		.sf_inst	= &insts_array[2], 
		.readID		= sf_readID3,
		.sector_erase	= sf_sector_erase,
		.block_erase	= sf_block_erase,
		.cpu_write		= sf_cpu_write,
		//////add on 2013-8-30
		.clearSR	= sf_clear_WRSR,
		.ProtectSR	= sf_ProtectSR,
		//////end
	},
	{
		.name	= "MXIC&AMIC", 
		.id0	= 0xc2,
		.id1	= 0x20,
		.id2	= 0x18,
		.page_size	= 256,		//256 byte
		.sector_size	= 256 * 16,	//4K byte
		.block_size	= 256 * 16 * 16,//64K byte
		.chip_size	= 256 * 16 * 16 * 256,//16M byte
		.max_hz		= 33 * 1024 * 1024,
		.sf_inst	= &insts_array[3], 
		.readID		= sf_readID1,
		.sector_erase	= sf_sector_erase,
		.block_erase	= sf_block_erase,
		.cpu_write		= sf_cpu_write,
		.dma_write		= sf_dma_write,
		.mdma_write		= sf_mdma_write,
		//////add on 2013-8-30
		.clearSR	= sf_clear_WRSR,
		.ProtectSR	= sf_ProtectSR,
		//////end
	},
	//////add on 2013-8-30
	{
		.name = "MXIC&AMIC", //MX25L12835E
		.id0	= 0xc2,
		.id1	= 0x20,
		.id2	= 0x19,
		.page_size	= 256*16,		//256 byte
		.sector_size	= 256 * 16,	//4K byte
		.block_size	= 256 * 16 * 16,//64K byte
		.chip_size	= 256 * 16 * 16 * 256,//16M byte
		.max_hz		= 33 * 1024 * 1024,
		.sf_inst	= &insts_array[3],
		.readID		= sf_readID1,
		.sector_erase	= sf_sector_erase,
		.block_erase	= sf_block_erase,
		.cpu_write	= sf_cpu_write,
		.dma_write	= sf_dma_write,
		.mdma_write		= sf_mdma_write,
		.clearSR	= sf_clear_WRSR,
		.ProtectSR	= sf_ProtectSR,
	},
	//////end
	{
		.name = "GD", //GD25Q128C
		.id0	= 0xc8,
		.id1	= 0x40,
		.id2	= 0x18,
		.page_size	= 256,		//256 byte
		.sector_size	= 256 * 16,	//4K byte
		.block_size	= 256 * 16 * 16,//64K byte
		.chip_size	= 256 * 16 * 16 * 256,//16 Mbyte
		.max_hz		= 33 * 1024 * 1024,
		.sf_inst	= &insts_array[4],
		.readID		= sf_readID1,
		.sector_erase	= sf_sector_erase,
		.block_erase	= sf_block_erase,
		.cpu_write	= sf_cpu_write,
		.dma_write	= sf_dma_write,
		.mdma_write		= sf_mdma_write,
		.clearSR	= sf_clear_WRSR,
		.ProtectSR	= sf_ProtectSR,
	},
	{
		.name = "GD",					//GD25Q64C
		.id0	= 0xc8,
		.id1	= 0x40,
		.id2	= 0x17,
		.page_size	= 256,				//256 byte
		.sector_size	= 256 * 16, 	//4K byte
		.block_size = 256 * 16 * 16,	//64K byte
		.chip_size	= 8*1024*1024,		// 8 Mbyte
		.max_hz 	= 33 * 1024 * 1024,
		.sf_inst	= &insts_array[4],
		.readID 	= sf_readID1,
		.sector_erase	= sf_sector_erase,
		.block_erase	= sf_block_erase,
		.cpu_write	= sf_cpu_write,
		.dma_write	= sf_dma_write,
		.mdma_write 	= sf_mdma_write,
		.clearSR	= sf_clear_WRSR,
		.ProtectSR	= sf_ProtectSR,
	},
	{
		.name = "GD", 					//GD25Q32C
		.id0	= 0xc8,
		.id1	= 0x40,
		.id2	= 0x16,
		.page_size	= 256,				//256 byte
		.sector_size	= 256 * 16, 	//4K byte
		.block_size = 256 * 16 * 16,	//64K byte
		.chip_size	= 4*1024*1024,		// 4 Mbyte
		.max_hz 	= 33 * 1024 * 1024,
		.sf_inst	= &insts_array[4],
		.readID 	= sf_readID1,
		.sector_erase	= sf_sector_erase,
		.block_erase	= sf_block_erase,
		.cpu_write	= sf_cpu_write,
		.dma_write	= sf_dma_write,
		.mdma_write 	= sf_mdma_write,
		.clearSR	= sf_clear_WRSR,
		.ProtectSR	= sf_ProtectSR,
	},
	{
		.name = "GD", //GD25Q16B
		.id0	= 0xc8,
		.id1	= 0x40,
		.id2	= 0x15,
		.page_size	= 256,		//256 byte
		.sector_size	= 256 * 16,	//4K byte
		.block_size	= 256 * 16 * 16,//64K byte
		.chip_size	= 256 * 16 * 2 * 256,// 2 Mbyte
		.max_hz		= 33 * 1024 * 1024,
		.sf_inst	= &insts_array[4],
		.readID		= sf_readID1,
		.sector_erase	= sf_sector_erase,
		.block_erase	= sf_block_erase,
		.cpu_write	= sf_cpu_write,
		.dma_write	= sf_dma_write,
		.mdma_write		= sf_mdma_write,
		.clearSR	= sf_clear_WRSR,
		.ProtectSR	= sf_ProtectSR,
	},
	{
		.name	= "PMC", //Pm25LV512
		.id0	= 0x9d,
		.id1	= 0x7b,
		.id2	= 0x7f,
		.page_size	= 256*16,		// 256 byte
		.sector_size	= 256 * 16,	// 4K byte
		.block_size	= 256 * 16 * 8,	// 32K byte
		.chip_size	= 256 * 16 * 8 * 2,// 64K byte
		.sf_inst	= &insts_array[5], 
		.readID		= sf_readID2,
		.sector_erase	= sf_sector_erase,
		.block_erase	= sf_block_erase,
		.cpu_write	= sf_cpu_write,
		//////add on 2013-8-30
		.clearSR	= sf_clear_WRSR,
		.ProtectSR	= sf_ProtectSR,
		//////end
	},
	{
		.name = "EN", //EN25P64
		.id0 = 0xef,
		.id1 = 0x30,
		.id2 = 0x17,
		.page_size   = 256*16,		// 256 byte
		.sector_size = 256 * 16,	// 64K byte
		.block_size  = 256 * 16 * 16,	// 64K byte
		.chip_size   = 256 * 16 * 16 * 128,// 8M byte
		.sf_inst     = &insts_array[6],
		.readID      = sf_readID1,
		.sector_erase = sf_sector_erase,
		.block_erase	= sf_block_erase,
		.cpu_write       = sf_cpu_write,
		//////add on 2013-8-30
		.clearSR       = sf_clear_WRSR,
		.ProtectSR     = sf_ProtectSR,
		//////end
	},
	{
		.name = "SPANSION", //S25FL127S
		.id0         = 0x01,
		.id1         = 0x20,
		.id2         = 0x18,
		.page_size	= 256*16,		//256 byte
		.sector_size	= 256 * 16,	//4K byte
		.block_size	= 256 * 16 * 16,//64K byte
		.chip_size	= 256 * 16 * 16 * 256,//16 Mbyte
		.max_hz		= 50 * 1024 * 1024,
		.sf_inst	= &insts_array[7],
		.readID		= sf_readID1,
		.sector_erase	= sf_sector_erase,
		.block_erase	= sf_block_erase,
		.cpu_write	= sf_cpu_write,
		.dma_write	= sf_dma_write,
		.mdma_write		= sf_mdma_write,
		.clearSR	= sf_clear_WRSR,
		.ProtectSR	= sf_ProtectSR,
	},
	{
		.name = "WINBOND", //W25Q128FVEF
		.id0		= 0xef,
		.id1		= 0x40,
		.id2		= 0x18,
		.page_size	= 256,
		.sector_size	= 256 * 16,
		.block_size	= 256 * 16 * 16,
		.chip_size	= 256 * 16 * 16 * 256,
		.max_hz = 33 * 1024 * 1024,     //33M byte
		.sf_inst	= &insts_array[8],
		.readID		= sf_readID1,
		.sector_erase	= sf_sector_erase,
		.block_erase	= sf_block_erase,
		.cpu_write		= sf_cpu_write,
		.dma_write	= sf_dma_write,
		.mdma_write		= sf_mdma_write,
		.clearSR	= sf_clear_WRSR,
		.ProtectSR	= sf_ProtectSR,
	},
	{
		.name = "WINBOND", //W25Q16JV
		.id0		= 0xef,
		.id1		= 0x40,
		.id2		= 0x15,
		.page_size	= 256,
		.sector_size	= 256 * 16,
		.block_size 	= 256 * 16 * 16,
		.chip_size		= 256 * 16 * 16 * 32,
		.max_hz = 33 * 1024 * 1024, 	//33M byte
		.sf_inst	= &insts_array[8],
		.readID 	= sf_readID1,
		.sector_erase	= sf_sector_erase,
		.block_erase	= sf_block_erase,
		.cpu_write		= sf_cpu_write,
		.dma_write	= sf_dma_write,
		.mdma_write 	= sf_mdma_write,
		.clearSR	= sf_clear_WRSR,
		.ProtectSR	= sf_ProtectSR,
	},
	{
		.name = "KH25L80006E", //KH25L80006E
		.id0		= 0xc2,
		.id1		= 0x20,
		.id2		= 0x14,
		.page_size	= 256,
		.sector_size	= 256 * 16,
		.block_size	= 256 * 16 * 16,
		.chip_size	= 2*1024*1024,
		.max_hz = 33 * 1024 * 1024,     //33M byte
		.sf_inst	= &insts_array[9],
		.readID		= sf_readID1,
		.sector_erase	= sf_sector_erase,
		.block_erase	= sf_block_erase,
		.cpu_write		= sf_cpu_write,
		.dma_write	= sf_dma_write,
		.mdma_write		= sf_mdma_write,
		.clearSR	= sf_clear_WRSR,
		.ProtectSR	= sf_ProtectSR,
	},
	{
		.name = "KH25L81606E", //KH25L81606E
		.id0		= 0xc2,
		.id1		= 0x20,
		.id2		= 0x15,
		.page_size	= 256,
		.sector_size	= 256 * 16,
		.block_size	= 256 * 16 * 16,
		.chip_size	= 2*1024*1024,
		.max_hz = 33 * 1024 * 1024,     //33M byte
		.sf_inst	= &insts_array[9],
		.readID		= sf_readID1,
		.sector_erase	= sf_sector_erase,
		.block_erase	= sf_block_erase,
		.cpu_write		= sf_cpu_write,
		.dma_write	= sf_dma_write,
		.mdma_write		= sf_mdma_write,
		.clearSR	= sf_clear_WRSR,
		.ProtectSR	= sf_ProtectSR,
	},
};


int  ms_serial_flash_init(void)
{
	int i, cnt;
	u32 speed;
	
	cnt = sizeof(serial_flash_array) / sizeof(struct serial_flash);
	
	for(i = 0; i < cnt; i++) {
		serial_printf ("item = %d,%d\n", i,cnt);
		if(serial_flash_init(&serial_flash_array[i])) {
			continue;
		}
		break;
	}

	if(i == cnt) {
		serial_printf("serial_flash initial failed\n");
		return -1;
	}

	serial_flash_current = &serial_flash_array[i];


	speed = AHB2_CLOCK / serial_flash_current->max_hz;
	if (speed > 1) {
		if (AHB2_CLOCK % serial_flash_current->max_hz)
			speed -= 1;
		else
			speed -= 2;
	}
	else
		speed = 0;
	

	serial_printf ("speed set = %x\n", speed);
	sfSetMsSpeed (speed);

	sfSetTimeCount (0x3fffffff); 	//set max timeout
	

	serial_printf(serial_flash_current->name);
	serial_printf("\n");

	return 0;
}


static int ms_serial_flash_read(u32 offset, u32 len)
{
	sf_cpu_read(serial_flash_current->sf_inst, offset, len);
}


static int ms_serial_flash_erase(u32 offset, u32 len)
{
	
	u32 top_addr, bytes, erase_len;

#if VERBOSE > 0
	serial_printf ("serial flash erase offset = 0x%x len = 0x%x\n", offset, len);
#endif
	if(0 == serial_flash_current->block_size || 0 == serial_flash_current->sector_size) {
		serial_printf("serial flash %s: block/sector size error\n", 
			serial_flash_current->name);
		return -EFAULT;
	}
	
	if((offset % serial_flash_current->sector_size) || (len % serial_flash_current->sector_size)) {
		serial_printf("serial flash %s: erase offset/len must multiple of sector size\n", 
			serial_flash_current->name);
		return -EINVAL;
	}
	
	bytes = offset % serial_flash_current->block_size;
	top_addr = offset + len;

	if (bytes) {
		erase_len = min(len,
			serial_flash_current->block_size - bytes);
		if (serial_flash_sector_erase (serial_flash_current, offset, erase_len))
			return -EFAULT;
		offset += erase_len;
	}
	
	if (offset < top_addr) {
		u32 block_len, block_offset;

		block_offset = top_addr - top_addr % serial_flash_current->block_size;
		block_len = block_offset - offset;
		if (block_len && serial_flash_block_erase (serial_flash_current, offset, block_len))
			return -EFAULT; 
	
		if ((block_offset != top_addr) &&  serial_flash_sector_erase (serial_flash_current, block_offset, top_addr - block_offset))
			return -EFAULT; 
	}

#if VERBOSE > 0
	serial_printf ("Serial Flash Erase: done\n");
#endif
	return 0;
}


static int ms_serial_flash_write(u32 offset, uchar *addr, u32 len)
{	
#if VERBOSE > 0
	serial_printf("Write: offset 0x%08x, buf = 0x%08x , len 0x%08x\n",
		offset, addr, len);
#endif

	return serial_flash_write(serial_flash_current,
		offset, addr, len);
}

void sfSet_mdma_init(struct sf_instruction *sf_instr)
{
	sfSetWMode(SF_NORMAL);
	sfSetAddrCyc(0);
	
	sfSetRDDummyByte(0);
	sfSetWRDummyByte(0);
	/*
	sfSetQpiMode(DISABLE);
	sfSetWipAddr(0);
	sfSetWipInv(1);
	*/
	sfSetCacheWcmd(sf_instr->PP);
	sfSetCacheRcmd(sf_instr->READ);
	//sfSetWDisCmd(sf_instr->WRDI);
	sfSetWEnCmd(sf_instr->WREN);
	sfSetStatusCmd(sf_instr->RDSR);
}

static u8 snx_sf_read_status(struct sf_instruction *sf_inst)
{			
	u8 status;
	
	sfChipEnable();
	
	sfMsRegRWSwitch(SF_WRITE_MODE);
	sfWriteData(sf_inst->RDSR);
	while (1) {
		if(sfCheckMsRdy()) {
			break;
		}
	}
	
	//read Status
	sfMsRegRWSwitch(SF_READ_MODE);	
	sfWriteData(0);	//Dummy Write
	while (1) {
		if(sfCheckMsRdy()) {
			break;
		}
	}
	status = (u8)sfReadData();
	
	sfChipDisable();
	
	return status;
}

static int sf_mdma_read(struct serial_flash *sf, u32 src_addr, uchar *dst_addr, u32 size)
{
	u8 status;
	u32 dma_block = 0;
	u32 dma_size = 0;
	u32 len = size;
	u32 transblk;
	u32 dst_dram_addr = (u32)dst_addr;
	u32 src_sf_addr = (u32)src_addr;

	sfSet_mdma_init(sf->sf_inst);
	
	while(len)
	{
		if(len > sf->page_size)
		{
			if((src_sf_addr & 0xFF) != 0){
				dma_size = sf->page_size - (src_sf_addr & 0xFF);
				dma_block = 1;
				len = len - dma_size;
			}else{
				dma_size = sf->page_size;
				dma_block = len / sf->page_size;
				if (dma_block > SF_BLKSU_BLK)
					dma_block = SF_BLKSU_BLK;
				len = len - (dma_size*dma_block);
			}
		}else if((len + (src_sf_addr & 0xFF)) > sf->page_size){
			dma_size = sf->page_size - (src_sf_addr & 0xFF);
			dma_block = 1;
			len = len - dma_size;
		}else{
			dma_size = len;
			dma_block = 1;
			len = 0;
		}

		//serial_printf("dma_size = %d, dma_block = %d, len = %d\n",dma_size, dma_block, len);
		if((dma_block == 1)&&(dma_size < sf->page_size)){ 
			//serial_printf("oBLOCK\n");
			//Parameter Setting
			sfSetDmaSize(dma_size-1);
			sfMsDmaRWSwitch(SF_READ_MODE);
			sfSetDmaAddr(dst_dram_addr);

			//Pre-Process
			sfChipEnable();

			sfMsRegRWSwitch(SF_WRITE_MODE);
			sfWriteData(sf->sf_inst->READ);
			while (1) {
				if(sfCheckMsRdy()) {
					break;
				}
			}

			//Address
			sfWriteData (MSSF_ADDR_1 (src_sf_addr));
			while (1) {
				if(sfCheckMsRdy()) {
					break;
				}
			}
			sfWriteData (MSSF_ADDR_2 (src_sf_addr));
			while (1) {
				if(sfCheckMsRdy()) {
					break;
				}
			}
			sfWriteData (MSSF_ADDR_3 (src_sf_addr));
			while (1) {
				if(sfCheckMsRdy()) {
					break;
				}
			}

			sfSetWMode(SF_NORMAL);
			sfMsDmaENSwitch(ENABLE);
			while (1) {
				if(sfCheckMsRdy()) {
					break;
				}
			}
			sfMsDmaENSwitch(DISABLE);
			sfChipDisable();
			
			do{
				status=snx_sf_read_status(sf->sf_inst);
			}while ((status & 0x01));
		}else{
			//serial_printf("mBLOCK\n");
			sfChipEnable();

			sfSetDmaSize(dma_size-1);
			sfMsDmaRWSwitch(SF_READ_MODE);
			sfSetDmaAddr(dst_dram_addr);	
			sfDmaBlock(dma_block-1);
			sfSetMdmaStartAddr(src_sf_addr);		

			sfMsMDmaENSwitch(MSSF_ENABLE);
			sfMsDmaENSwitch(MSSF_ENABLE);

			while (1) {
				if(sfCheckMsRdy()){
					break;
				}
			}
			
			if(wait_till_msdma_ok()){
				serial_printf("dma read fail!!!\n");
				return 1;
			}
			transblk = sfReadSUDmaBlock();
			//serial_printf("offset=%x, addr=%x, dma_size=%x, transblk=%x\n", src_sf_addr, dst_dram_addr, dma_size, transblk);

			sfMsDmaENSwitch(MSSF_DISABLE);
			sfMsMDmaENSwitch(MSSF_DISABLE);

			sfChipDisable();
		}
		
		//Calculate Next Dram & SF Address
		src_sf_addr += (dma_size*dma_block);
		dst_dram_addr += (dma_size*dma_block);
	}
	return 0;
}

static int ms_serial_flash_ckim(u32 offset, uchar *addr, u32 len, u32 buff_addr)
{
	// uchar tbuf[0x4000]; 	// 16K
	int	ret = 1;
	uchar *tbuf;

	tbuf = buff_addr;

	serial_printf("compare : offset 0x%08x, buf = 0x%08x , len 0x%08x, sp 0x%08x\n", offset, addr, len, buff_addr);

	sf_mdma_read(serial_flash_current, offset, tbuf, len);
  	ret = memcmp(tbuf,addr,len);

 	return ret;
}



//////add on 2013-09-2
static int ms_serial_flash_protect(void)
{	
	serial_printf("Serial Flash Protected\n");
	
	return serial_flash_protect(serial_flash_current);
}
//////end

#define SPI_BLOCK_SIZE	0x10000
#define SPI_SECTOR_SIZE	0x1000

#define IMAGE_TABLE_TOLAL_SIZE	0x200
#define MIN_REASE_SIZE 	0x1000

extern u32 ONLY_BRINGUP_RTOS;

extern void jump2(u32 addr);

/*
  check if a block of memory is all 0xFF
 */
static int all_ones(const unsigned char *data, u32 size)
{
    u32 i;
    for (i=0; i<size; i++) {
        if (data[i] != 0xFF) {
            return -1;
        }
    }
    return 0;
}

/*
  perform sector by sector update, comparing with new data and only updating as needed
 */
static int ms_serial_flash_compare_update(u32 img_fl_st_addr, u32 img_fl_st_size,
                                          const uchar *image, u32 img_sz)
{
    u32 sector_size = serial_flash_current->sector_size;
    static unsigned char sector_buffer[0x1000];

    if (sector_size > sizeof(sector_buffer)) {
        serial_printf("ERROR: sector_size > sizeof(sector_buffer)\n");
        return -1;
    }
        
    if (sector_buffer == NULL) {
        return -1;
    }

    u32 start_address = img_fl_st_addr;
    u32 total_size = img_sz;
    u32 total_erased = 0;
    u32 total_written = 0;
    u32 total_skipped = 0;

    while (img_fl_st_size > 0) {
        u32 data_size = img_sz>sector_size?sector_size:img_sz;

        if (sf_mdma_read(serial_flash_current, img_fl_st_addr, sector_buffer, sector_size) != 0) {
            serial_printf("sf_mdma_read failed at 0x%x\n", img_fl_st_addr);
            return -1;
        }

        if (memcmp(sector_buffer, image, data_size) == 0 &&
            all_ones(&sector_buffer[data_size], sector_size-data_size) == 0) {
            total_skipped += sector_size;
            goto next;
        }

        if (ms_serial_flash_erase(img_fl_st_addr, sector_size) != 0) {
            serial_printf("ms_serial_flash_erase failed at 0x%x\n", img_fl_st_addr);
            return -1;
        }
        total_erased += sector_size;

        if (img_sz > 0) {	
            if (ms_serial_flash_write(img_fl_st_addr, (uchar *)image, data_size) != 0) {
                serial_printf("ms_serial_flash_write failed at 0x%x for %u\n", img_fl_st_addr, data_size);
                return -1;
            }
            total_written += data_size;
        }

    next:
        image += data_size;
        img_sz -= data_size;
        img_fl_st_addr += sector_size;
        img_fl_st_size -= sector_size;
    }

    serial_printf("flash at 0x%x size=%u erased=%u written=%u skipped=%u\n",
                  start_address, total_size, total_erased, total_written, total_skipped);

    return 0;
}

int ms_serial_flash_update(unsigned int addr, unsigned int size)
{
	u32 i;
	u32 img_tbl_st_addr;
	u32 ddr_start_addr = IMAGE_TABLE_START + IMAGE_TABLE_TOLAL_SIZE;

	u32 align_erase_lens;
	u32 mut;
	u32 rem;

	u32 load_bootsel_addr = 0;
	u32 load_rtos_addr = 0;
	u32 jmp_bootsel_addr = 0;
	u32 tbuff_addr = 0;
	
	img_tbl_st_addr = IMAGE_TABLE_START + 4;

	for (i=0; i < (IMAGE_TABLE_SIZE/IMAGE_TABLE_ENTRY_SIZE); i++) {
		u32 revs = 0;
		u32 img_st_addr = 0;
		u32 img_sz = 0;
		u32 img_fl_st_addr = 0;
		u32 img_fl_end_addr = 0;

		revs = readl(img_tbl_st_addr);
		img_st_addr = readl(img_tbl_st_addr + 4);
		img_sz = readl(img_tbl_st_addr + 8);
		img_fl_st_addr = readl(img_tbl_st_addr + 12);
		img_fl_end_addr = readl(img_tbl_st_addr + 16);	

		img_st_addr += ddr_start_addr;
		serial_printf("img_st_addr = %x\n", img_st_addr);
		serial_printf("img_sz = %x\n", img_sz);
		serial_printf("img_fl_st_addr = %x\n", img_fl_st_addr);
		serial_printf("img_fl_end_addr = %x\n", img_fl_end_addr);

		img_tbl_st_addr = img_tbl_st_addr + 20;

                if (ONLY_BRINGUP_RTOS == 1) {
                    if (img_fl_st_addr == bootsel_st) {
                        load_bootsel_addr = (* (volatile unsigned int *) (0xffff601c)) & 0x0fffffff;
                        memcpy(load_bootsel_addr,img_st_addr,img_sz);
                        
                        serial_printf("cp bootsel image to %x from %x , size =%x\n", load_bootsel_addr,img_st_addr,img_sz);
                    }
                    
                    if (img_fl_st_addr == rtos_st) {
                        load_rtos_addr = (* (volatile unsigned int *) (0xffff6148)) & 0x0fffffff;
                        memcpy(load_rtos_addr,img_st_addr,img_sz);
                        
                        serial_printf("cp rtos image to %x from %x , size =%x\n", load_rtos_addr,img_st_addr,img_sz);
                    }
                }
                else {
                    if (ms_serial_flash_compare_update(img_fl_st_addr, (img_fl_end_addr - img_fl_st_addr + 1),
                                                       (uchar *)img_st_addr, img_sz) != 0) {
                        return -1;
                    }
                }
        }
        
	if (ONLY_BRINGUP_RTOS == 0) { 
            ms_serial_flash_protect();
	}
	else {
            * (volatile unsigned int *) (0xffff6144) = 0x73737062;
            
            jmp_bootsel_addr = (* (volatile unsigned int *) (0xffff6090)) & 0x0fffffff;
            jump2(jmp_bootsel_addr);
            
            serial_printf("ONLY BRINGUP RTOS STOP!\n");
            while(1);
	}
	
	return 0;
}
