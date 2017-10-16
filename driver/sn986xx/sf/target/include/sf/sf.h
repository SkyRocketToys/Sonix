/**
 * @file
 * this is sf driver header file, include this file before use
 * @author CJ
 */

#ifndef __SF_H__ 
#define __SF_H__ 

//--------------------------------
//Register definition!
//--------------------------------
#define	MS1_BASE_ADDRESS		(0x90900000)

#define	SF_CTL      			(MS1_BASE_ADDRESS)
#define	SF_DMA_SIZE				(MS1_BASE_ADDRESS + 0x04)
#define	SF_CRC					(MS1_BASE_ADDRESS + 0x08)
#define SF_MDMA_BYTE			(MS1_BASE_ADDRESS + 0x20)
#define SF_CACHE_RW_CMD			(MS1_BASE_ADDRESS + 0x24)
#define SF_WEN_CMD				(MS1_BASE_ADDRESS + 0x28)
#define SF_4BIT					(MS1_BASE_ADDRESS + 0x2c)
#define	SF_MS_IO_I				(MS1_BASE_ADDRESS + 0x30)
#define	SF_MS_IO_O				(MS1_BASE_ADDRESS + 0x34)
#define	SF_MS_IO_OE				(MS1_BASE_ADDRESS + 0x38)
#define SF_SPI_CMD				(MS1_BASE_ADDRESS + 0x3c)
#define	SF_DMA_BLKSU			(MS1_BASE_ADDRESS + 0x48)
#define	SF_TMCNT				(MS1_BASE_ADDRESS + 0x4c)
#define	SF_MDMAECC				(MS1_BASE_ADDRESS + 0x50)
#define	SF_DMA_ADDR				(MS1_BASE_ADDRESS + 0x5c)
#define	SF_REG_DATA				(MS1_BASE_ADDRESS + 0x74)
#define	SF_VERSION				(MS1_BASE_ADDRESS + 0x80)

//--------------------------------
//SF_CTL 0x00
//--------------------------------
#define SF_CTL_MODE				0x00000007
#define SF_CTL_REGRW			0x00000008
#define SF_CTL_DMAEN			0x00000010
#define SF_CTL_DMARW			0x00000020
#define SF_CTL_ECCEN			0x00000080
#define SF_CTL_MSRDY			0x00000100
#define SF_CTL_CRCEN			0x00000200
#define SF_CTL_CMDNIBTRG		0x00000800
#define SF_CTL_MSSPEED			0xff000000

//--------------------------------
//SF_DMA_SIZE 0x04
//--------------------------------
#define SF_DMASIZE_MSDMASIZE	0x0001ffff
#define SF_CTL_W_MODE			0x00060000
#define SF_CACHE_W_CMD			0xff000000

//--------------------------------
//SF_CRC 0x08
//--------------------------------
#define SF_CRC_CRC16_L			0x000000ff
#define SF_CRC_CRC16_H			0x0000ff00

//--------------------------------
//SF_MDMA_BYTE 0x20
//--------------------------------
#define SF_READ_DUMMY_BYTE		0x0000000f
#define SF_WRITE_DUMMY_BYTE		0x000000f0
#define SF_ADDR_CYC				0x00000100
#define SF_CMD_NIBBLE			0x00007000

//--------------------------------
//SF_CACHE_RW_CMD 0x24
//--------------------------------

#define SF_CACHE_R_CMD			0x0000ff00
#define SF_ERASE_CMD			0xff000000

//--------------------------------
//SF_W_EN_CMD 0x28
//--------------------------------
#define SF_W_DIS_CMD			0x0000ff00
#define SF_W_EN_CMD				0x00ff0000
#define SF_STATUS_CMD			0xff000000

//--------------------------------
//SF_4BIT/Double Rate 0x2c
//--------------------------------
#define SCK_PHS_SEL				0x00000007
#define SF_QPI_MODE				0x00000200
#define NF_SF_4BITS				0x00000400
#define SF_DT_EN				0x00000800
#define SF_DUMMY_EN				0x00001000
#define SF_DUMMY_CYC			0x00070000
#define SF_WIP_ADDR				0x07000000
#define SF_WIP_INV				0x08000000

//--------------------------------
//SF_MS_IO 0x30 ~ 0x38
//--------------------------------
#define SF_MSIO_I				0x0000003f
#define SF_MSIO_O				0x0000003f
#define SF_MSIO_OE				0x0000003f
#define SF_MS_IO_O_3			0x00000008 // CS
#define SF_MS_IO_OE_3			0x00000008 // CS

//--------------------------------
//SF_SPI_CMD 0x3c
//--------------------------------
#define SF_SPI_CMD_CMD			0xffffffff

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

//--------------------------------
// SF error message
//--------------------------------
#define	SF_ERR_PASS						0	/**< define return message*/
#define	SF_ERR_SEMAPHORE_CREATE_FAIL	1	/**< define return message*/
#define	SF_ERR_SEMAPHORE_TAKE_FAIL		2	/**< define return message*/
#define SF_ERR_ID						3	/**< define return message*/
#define SF_ERR_SECOTR_ADDR				4	/**< define return message*/
#define SF_ERR_SECOTR_SIZE				5	/**< define return message*/
#define SF_ERR_CHIP_SIZE				6	/**< define return message*/
#define SF_ERR_PARM						7	/**< define return message*/
#define SF_ERR_MDMA_FAIL				8	/**< define return message*/
#define SF_ERR_INTR_RQ				9	/**< define return message*/


//SF Read/Write DMA Size
#define MAX_SF_RDMA_SIZE 				0x8000//32K Bytes  : SF->Dram
#define MAX_SF_WDMA_SIZE 				0x0100//256 Bytes : Dram->SF

#define	TAKE_SEMAPHORE_WAIT_TIME		1000

#define SF_INTERRUPT_MODE			1
#define SF_POLL_MODE				0

#ifndef size_t
	#define size_t	uint32_t
#endif

/**
 * @brief sf command info structure
 */
typedef struct serialflash_instr {
	uint32_t WREN;		/**< sf write enable command */
	uint32_t WRDI;		/**< sf write disable command */
	uint32_t RDID;		/**< sf read id command */
	uint32_t RDSR;		/**< sf read status command */
	uint32_t WRSR;		/**< sf write status command */
	uint32_t READ;		/**< sf read data command */
	uint32_t PP;		/**< sf write date command */
	uint32_t SE;		/**< sf sector erase command */
	uint32_t BE;		/**< sf block erase command */
	uint32_t CE;		/**< sf chip erase command */
}serialflash_instr_t;

/**
 * @brief sf id and size info structure
 */
typedef struct serial_flash{
	char *name;					/**< sf vender */
	
	const uint32_t id0;			/**< sf id0 */
	const uint32_t id1;			/**< sf id1 */
	const uint32_t id2;			/**< sf id2 */
	
	uint32_t page_size;			/**< page size. unit: bytes, set to 0x0 if not support page program */
	uint32_t sector_size;		/**< sector size. unit: bytes, set to 0x0 if not support sector erase */
	uint32_t block_size;		/**< block size. unit: bytes, set to 0x0 if not support block erase  */
	uint32_t chip_size;			/**< chip size. unit: bytes */
}serial_flash_t;

/**
 * @brief flash information on the total structure contains the callback function
 */
typedef struct serialflash_int {
	serialflash_instr_t *sf_inst;		/**< sf command information struct */
	serial_flash_t	serial_flash;		/**< id and size information struct */
	
	uint32_t max_hz;			/**< max frequency. unit: Mhz */

	void (*readID)(serialflash_instr_t *sf_inst, uint8_t *id);	/**< read ID callback function pointer */
	uint8_t (*sector_erase)(serialflash_instr_t *sf_inst, 
		serial_flash_t size_info, void *addr, size_t size);	/**< sector erase callback function pointer */
	uint8_t (*chip_erase)(serialflash_instr_t *sf_inst);		/**< chip erase callback function pointer */
	uint8_t (*block_erase)(serialflash_instr_t *sf_instr, 		
		serial_flash_t size_info, void *addr, size_t size);	/**< block erase callback function pointer */
	uint8_t (*sf_write)(serialflash_instr_t *sf_instr, 
		serial_flash_t size_info, void *dst_addr, 
		void *src_addr, size_t size);				/**< flash write data callback function pointer */
	uint8_t (*sf_read)(serialflash_instr_t *sf_instr, 
		serial_flash_t size_info, void *dst_addr, 
		void *src_addr, size_t size);				/**< flash read data callback function pointer */
	void (*clearSR)(serialflash_instr_t *sf_inst, uint8_t pre_status);			/**< disable write protect callback function pointer */
	void (*protectSR)(serialflash_instr_t *sf_inst, uint8_t pre_status);			/**< enable write protect callback function pointer */
}serialflash_int_t;


//--------------------------------
// SF Function
//--------------------------------
uint8_t sf_init(void);
uint8_t sf_chip_erase(void);
uint8_t sf_sector_erase(void *addr, size_t size);
uint8_t sf_block_erase(void *addr, size_t size);
uint8_t sf_write_sf(void *dst_addr, void *src_addr, size_t size);
uint8_t sf_read_sf(void *dst_addr, void *src_addr, size_t size);
uint8_t sf_get_capacity(serial_flash_t* sf_cap);
uint8_t sf_set_mode (uint8_t value);
uint8_t sf_license(serialflash_int_t* sf_param,uint32_t Add_cnt);
//uint8_t sf_test(void);

#endif
