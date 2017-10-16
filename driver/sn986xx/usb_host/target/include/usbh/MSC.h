
#ifndef	MSC__H
#define	MSC__H	 

#include <generated/snx_sdk_conf.h> 

/**
 * @file
 * this	is MSC file, include this file before use
 * @author IP2 Dept Sonix. (Hammer Huang #1359)
 */

// MSC Debug Message MACRO
/** \defgroup MSC_DEBUG	Debug definition
 * \ingroup MSC_MODULE
 * 
 * @{
 */ 
#if defined( CONFIG_SN_GCC_SDK )
#if defined( CONFIG_MODULE_USB_MSC_DEBUG )
	#define MSC_DBG(format, args...) print_msg("\nMSC_DBG:"format, ##args);	/**< MSC Debug Level 1 */	
#else
	#define MSC_DBG(args...) /**< MSC Debug Level 0 */
#endif
	#define MSC_INFO(format, args...) print_msg_queue(format, ##args)
#endif 


#if defined( CONFIG_SN_KEIL_SDK )
#include "stdio.h"
#if defined( CONFIG_MODULE_USB_MSC_DEBUG )
	#define MSC_DBG(format, args...) printf("\nMSC_DBG:"format, ##args);	/**< MSC Debug Level 1 */	
#else
	#define MSC_DBG(args...) /**< MSC Debug Level 0 */
#endif
	#define MSC_INFO(format, args...) printf(format, ##args)
#endif

#if defined( CONFIG_XILINX_SDK )
#include "xil_printf.h"
#if defined( CONFIG_MODULE_USB_MSC_DEBUG )
	#define MSC_DBG(format, args...) xil_printf("\nMSC_DBG:"format, ##args);	/**< MSC Debug Level 1 */	
#else
	#define MSC_DBG(args...) /**< MSC Debug Level 0 */
#endif
	#define MSC_INFO(format, args...) xil_printf(format, ##args)
#endif


/** @} */

// MSC Plug & play MACRO
#define	MSC_EXIST	1
#define	MSC_NOT_EXIST	0
#define	MSC_CONNECT	MSC_EXIST
#define	MSC_DISCONNECT	MSC_NOT_EXIST
#define	MSC_ENUM	2
#define	MSC_ACTIVE	3

// MSC MACRO
#define MSC_ENUM_READ_TIMES 1
#define	MSC_ENUM_READ_SIZE	1024
#define	MSC_MAX_DEV		6

// MSC DEVICE TYPE MACRO
#define	DIRECT_ACCESS_DEVICE			0x00
#define	SEQUENTIAL_ACCESS_DEVICE		0x01
#define	PRINTER_DEVICE				0x02
#define	PROCESSOR_DEVICE			0x03
#define	WRITE_ONCE_DEVICE			0x04
#define	CD_DVD_DEVICE				0x05
#define	SCANNER_DEVICE				0x06
#define	OPTICAL_MEMORY_DEVICE			0x07
#define	MEDIUM_CHANGER_DEVICE			0x08
#define	COMMUNICATIONS_DEVICE			0x09
#define	STROAGE_ARRAY_CONTROLLER_DEVICE		0x0c
#define	ENCLOSURE_SERVICES_DEVICE		0x0d
#define	SIMPLIFIED_DIRECT_ACCESS_DEVICE		0x0e
#define	OPTICAL_CARD_READER			0x0f
#define	BRIDGE_CONTROLLER			0x10
#define	OBJECT_BASED_DEVICE			0x11
#define	AUTOMATION_DEVICE			0x12
#define	UNKNOWN_DEVICE				0x1f

// MSC SENSE KEY DESC MACRO
#define	SENSEKEY_DESC_NO_SENSE			0x00
#define	SENSEKEY_DESC_RECOVERED_ERROR		0x01
#define	SENSEKEY_DESC_NOT_READY			0x02
#define	SENSEKEY_DESC_MEDIUM_ERROR		0x03
#define	SENSEKEY_DESC_HARDWARE_ERROR		0x04
#define	SENSEKEY_DESC_ILLEGAL_REQUES		0x05
#define	SENSEKEY_DESC_UNIT_ATTENTION		0x06
#define	SENSEKEY_DESC_DATA_PROTECT		0x07
#define	SENSEKEY_DESC_BLANK_CHECK		0x08

#define	SENSEKEY_DESC_VENDOR_SPECIFIC		0x09
#define	SENSEKEY_DESC_COPY_ABORTED		0x0a
#define	SENSEKEY_DESC_ABORTED_COMMAN		0x0b
#define	SENSEKEY_DESC_OBSOLETE			0x0c
#define	SENSEKEY_DESC_VOLUME_OVERFLOW		0x0d
#define	SENSEKEY_DESC_MISCOMPARE		0x0e
#define	SENSEKEY_DESC_RESERVED			0x0f

typedef	enum
{
	MSC_DEVICE_ENUM_STATE,
	MSC_DEVICE_ACTIVE_STATE,
	MSC_DEVICE_ERR_STATE,
	MSC_DEVICE_DET_STATE,
}MSC_SM_INDEX;

typedef	enum
{
	MSC_ENUM_NONE,
	MSC_GET_MAX_LUN,
	MSC_GET_INQUIRY,
	MSC_TEST_UNIT_READY,
	MSC_GET_CAPACITY,
	MSC_REQUEST_SENSE,
	MSC_READ10,
	MSC_WRITE10,
	MSC_ENUM_DONE
}MSC_CMD_INDEX;

typedef	enum
{
	// CBW
	CBW_BULK_INQUIRY,
	CBW_BULK_READ_CAPACITY,
	CBW_BULK_TEST_UNIT_READY,
	CBW_BULK_REQUEST_SENSE,
	CBW_BULK_MODE_SENSE,
	CBW_BULK_READ10,
	CBW_BULK_WRITE10,
	CBW_BULK_READ_FORMAT_CAPACITIES ,
	CBW_PREVENT_ALLW_MDM_RMVL,
	CBW_MODE_SENSE,
	CBW_READ_FORMAT_CAPACITIES,

	// DATA 
	//DATA_OUT,
	//DATA_IN,

	// CSW
	//CSW,

	// Vendor
	//VENDOR_OUT,
	//VENDOR_IN,

	CBW_BULK_VENDOR_READ,
	CBW_BULK_VENDOR_WRITE,
	CBW_BULK_VENDOR_NONDATA
	
}BK_CMD_INDEX;

typedef enum {

	CBW_STAGE = 1,
	DATA_STAGE,
	CSW_STAGE

}BK_CMD_STAGE;

typedef struct{
	uint8_t		inquiry[36];
	uint8_t		request_sense[18];
	uint64_t	capacity;
	uint32_t	block_len;
	uint32_t	lba;
}MSC_DEVICE_INFO;
//!  Mass Storage Class	Device Structure
/*!
*/
typedef	struct{
	USBH_ERR_HDL_STRUCT	*err_hdl;		//!< USB Error Handle Structure
	MSC_DEVICE_INFO		info[MSC_MAX_DEV];	//!< Mass Storage Device Information
	uint8_t			enum_sm;		//!< Enum State Machine
	uint8_t			enum_cmd;		//!< Enum Command
	uint8_t			status;			//!< Mass Storage Device Status
	uint8_t			lun;			//!< Logical Unit Number of	Mass Storage Device
}MSC_DEVICE_STRUCT;
/** @} */

//!  Mass Storage Class	Request	Structure
/*!
*/
typedef	struct
{
	uint32_t	cmd;			//!< Bulk Command
	uint32_t	size;			//!< Bulk Data Stage Size
	uint64_t	capacity;		//!< Device	Capacity
	uint32_t	block_len;		//!< Device	Block Length
	uint32_t	act_size;		//!< Actually Transfer Size
	uint8_t		*pbuff;			//!< Data Stage Data Pointer
	uint32_t	lba;			//!< Device	Logical	Block Address
	uint8_t		cbw[31];		//!< Data of Command Block Wrapper
	uint8_t		csw[13];		//!< Data of Command Status	Wrapper
	uint8_t		inquiry[36];		//!< Data of Inquiry
	uint8_t		request_sense[18];	//!< Data of Request Sense
	uint8_t		lun;			//!< Logical Unit Numner of	Mass Storage Device
}MSC_REQ_Struct;
/** @} */

typedef	struct
{
	uint32_t	signature;			//!< Signature to Represent a CBW
	uint32_t	tag;				//!< CBW Tag
	uint32_t	transfer_length;	//!< Data Transfer Length
	uint8_t		flag;				//!< 0x00 Data-Out / 0x80 Data-In
	uint8_t		lun;				//!< Logical Unit Number
	uint8_t		cblength;			//!< Valid Command Block Length
	uint8_t		cb[16];				//!< Command Block to be Executed by Device
}MSC_VENDOR_REQ_Struct;

extern uint8_t BULK_READ10_31[31];			//!< Predefined Array to Transfer Read10 Command
extern uint8_t BULK_WRITE10_31[31];			//!< Predefined Array to Transfer Write10 Command
extern uint8_t BULK_VENDOR_READ_31[31];		//!< Predefined Array to Transfer Vendor Read Command
extern uint8_t BULK_VENDOR_WRITE_31[31];		//!< Predefined Array to Transfer Vendor Write Command
extern uint8_t BULK_VENDOR_NONDATA_31[31];	//!< Predefined Array to Transfer Vendor None Data Command

MSC_REQ_Struct msc_req_dev;
extern uint8_t msc_sm;
extern SemaphoreHandle_t USBH_SEM_WAKEUP_MSC_DRV;
extern SemaphoreHandle_t USBH_SEM_WAKEUP_MSC_APP;
extern xTaskHandle xTASK_HDL_MSC_DRV;
extern xTaskHandle xTASK_HDL_MSC_APP;
extern MSC_DEVICE_STRUCT MSC_DEV;
extern uint8_t msc_transfer(USBH_Device_Structure *dev,	MSC_REQ_Struct *msc_req);
extern uint8_t msc_rw_test(USBH_Device_Structure *dev,uint16_t sec_count ,uint32_t times);

// MSC BULK ENUMERATION
void msc_check_dev_sts(USBH_Device_Structure *dev, MSC_DEVICE_STRUCT *msc_dev);
uint8_t	msc_bulk_enum(USBH_Device_Structure *dev, MSC_DEVICE_STRUCT *msc_dev);
void msc_print_enum_info(MSC_DEVICE_STRUCT *msc_dev);

// CX/MSC COMMAND Function
uint8_t	msc_cx_get_status(USBH_Device_Structure *dev, USBH_CX_XFR_REQ_Struct *cx_req);
uint8_t	msc_cx_get_max_lun(USBH_Device_Structure *dev, USBH_CX_XFR_REQ_Struct *cx_req);
extern uint8_t msc_cx_bkonly_reset(USBH_Device_Structure *dev, USBH_CX_XFR_REQ_Struct *cx_req);
extern uint8_t msc_reset_recovery(USBH_Device_Structure *dev, USBH_CX_XFR_REQ_Struct *cx_req);
extern uint8_t msc_inquiry(USBH_Device_Structure *dev, MSC_REQ_Struct *msc_req);
extern uint8_t msc_test_unit_ready(USBH_Device_Structure *dev, MSC_REQ_Struct *msc_req);
extern uint8_t msc_get_capacity(USBH_Device_Structure *dev, MSC_REQ_Struct *msc_req);
extern uint8_t msc_request_sense(USBH_Device_Structure *dev, MSC_REQ_Struct *msc_req, uint8_t lun);
extern uint8_t msc_read10(USBH_Device_Structure	*dev, MSC_REQ_Struct *msc_req, uint32_t	size, uint32_t lba);
extern uint8_t msc_write10(USBH_Device_Structure *dev, MSC_REQ_Struct *msc_req,	uint32_t size, uint32_t	lba);
extern uint8_t msc_random_test(uint32_t time, uint32_t sector, uint32_t compare);

// MSC TASK Function
void msc_enum_task(void	*pvParameters);
void msc_app_task(void *pvParameters);
void msc_task_init(void);
void msc_task_uninit(void);

// MSC API Function
uint32_t msc_dev_init(uint8_t ID);
uint8_t msc_read(USBH_Device_Structure *dev, void *pdata_buf, uint32_t size, uint32_t lba);
uint8_t msc_write(USBH_Device_Structure *dev, void *pdata_buf, uint32_t size, uint32_t lba);

uint8_t msc_vendor_read(USBH_Device_Structure *dev, MSC_VENDOR_REQ_Struct *msc_vendor_req, void *pdata_buf, uint32_t size);
uint8_t msc_vendor_write(USBH_Device_Structure *dev, MSC_VENDOR_REQ_Struct *msc_vendor_req, void *pdata_buf, uint32_t size);
uint8_t msc_vendor_nonedata(USBH_Device_Structure *dev, MSC_VENDOR_REQ_Struct *msc_vendor_req);


automount_info_usbh* get_automount_info_usbh(void);

#endif
