/** 
* @file 
* this is usb device driver file 
* usbd_msc.h 
* @author IP2/Luka 
*/
#ifndef __SONIX_USBD_MSC_H
#define __SONIX_USBD_MSC_H


#ifdef __cplusplus
extern "C"{
#endif


//#define USBD_MSC_PRINT(fmt, args...) print_msg("[usbd-msc] "fmt, ##args)
#define USBD_MSC_PRINT(fmt, args...)  


/* Bulk-only class specific requests */
#define USB_BULK_RESET_REQUEST		0xff
#define USB_BULK_GET_MAX_LUN_REQUEST	0xfe
#define FSG_NUM_BUFFERS	2
/* Command Block Wrapper */
struct fsg_bulk_cb_wrap {
	unsigned int	Signature;		/* Contains 'USBC' */
	unsigned int	Tag;			/* Unique per command id */
	unsigned int	DataTransferLength;	/* Size of the data */
	unsigned char	Flags;			/* Direction in bit 7 */
	unsigned char	Lun;			/* LUN (normally 0) */
	unsigned char	Length;			/* Of the CDB, <= MAX_COMMAND_SIZE */
	unsigned char	CDB[16];		/* Command Data Block */
};
/* Command Status Wrapper */
struct fsg_bulk_cs_wrap {
	unsigned int	Signature;		/* Should = 'USBS' */
	unsigned int	Tag;			/* Same as original command */
	unsigned int	Residue;		/* Amount not transferred */
	unsigned char	Status;			/* See below */
};
#define USB_BULK_CB_WRAP_LEN	31
#define USB_BULK_CB_SIG		0x43425355	/* Spells out USBC */
#define USB_BULK_IN_FLAG	0x80

#define USB_BULK_CS_WRAP_LEN	13
#define USB_BULK_CS_SIG		0x53425355	/* Spells out 'USBS' */
#define USB_STATUS_PASS		0
#define USB_STATUS_FAIL		1
#define USB_STATUS_PHASE_ERROR	2


/* SCSI commands that we recognize */
#define SC_FORMAT_UNIT						0x04
#define SC_INQUIRY							0x12
#define SC_MODE_SELECT_6					0x15
#define SC_MODE_SELECT_10					0x55
#define SC_MODE_SENSE_6						0x1a
#define SC_MODE_SENSE_10					0x5a
#define SC_PREVENT_ALLOW_MEDIUM_REMOVAL		0x1e
#define SC_READ_6							0x08
#define SC_READ_10							0x28
#define SC_READ_12							0xa8
#define SC_READ_CAPACITY					0x25
#define SC_READ_FORMAT_CAPACITIES			0x23
#define SC_READ_HEADER						0x44
#define SC_READ_TOC							0x43
#define SC_RELEASE							0x17
#define SC_REQUEST_SENSE					0x03
#define SC_RESERVE							0x16
#define SC_SEND_DIAGNOSTIC					0x1d
#define SC_START_STOP_UNIT					0x1b
#define SC_SYNCHRONIZE_CACHE				0x35
#define SC_TEST_UNIT_READY					0x00
#define SC_VERIFY							0x2f
#define SC_WRITE_6							0x0a
#define SC_WRITE_10							0x2a
#define SC_WRITE_12							0xaa
/* SONiX command(SCSI Vendor Command) */
#define SC_SNX_STD_CMD						0xF6
#define SC_SNX_ICV_CMD						0xF7
#define SC_USR_STD_CMD						0xFA


/* SCSI Sense Key/Additional Sense Code/ASC Qualifier values */
#define SS_NO_SENSE                             0
#define SS_COMMUNICATION_FAILURE                0x040800
#define SS_INVALID_COMMAND                      0x052000
#define SS_INVALID_FIELD_IN_CDB                 0x052400
#define SS_LOGICAL_BLOCK_ADDRESS_OUT_OF_RANGE   0x052100
#define SS_LOGICAL_UNIT_NOT_SUPPORTED           0x052500
#define SS_MEDIUM_NOT_PRESENT                   0x023a00
#define SS_MEDIUM_REMOVAL_PREVENTED             0x055302
#define SS_NOT_READY_TO_READY_TRANSITION        0x062800
#define SS_RESET_OCCURRED                       0x062900
#define SS_SAVING_PARAMETERS_NOT_SUPPORTED      0x053900
#define SS_UNRECOVERED_READ_ERROR               0x031100
#define SS_WRITE_ERROR                          0x030c02
#define SS_WRITE_PROTECTED                      0x072700


#define SK(x)		((unsigned char) ((x) >> 16))	/* Sense Key byte, etc. */
#define ASC(x)		((unsigned char) ((x) >> 8))
#define ASCQ(x)		((unsigned char) (x))


#define FSG_MAX_LUNS		8			// Maximal number of LUNs supported in mass storage function 
#define MAX_COMMAND_SIZE	16			// Length of a SCSI Command Data Block 



enum fsg_state {
	/* This one isn't used anywhere */
	FSG_STATE_COMMAND_PHASE = -10,
	FSG_STATE_DATA_PHASE,
	FSG_STATE_STATUS_PHASE,

	FSG_STATE_IDLE = 0,
	FSG_STATE_ABORT_BULK_OUT,
	FSG_STATE_RESET,
	FSG_STATE_INTERFACE_CHANGE,
	FSG_STATE_CONFIG_CHANGE,
	FSG_STATE_DISCONNECT,
	FSG_STATE_EXIT,
	FSG_STATE_TERMINATED
};


/* SCSI device types */
#define TYPE_DISK	0x00
#define TYPE_CDROM	0x05

/* USB protocol value = the transport method */
#define USB_PR_CBI	0x00		/* Control/Bulk/Interrupt */
#define USB_PR_CB	0x01		/* Control/Bulk w/o interrupt */
#define USB_PR_BULK	0x50		/* Bulk-only */
/* USB subclass value = the protocol encapsulation */
#define USB_SC_RBC	0x01		/* Reduced Block Commands (flash) */
#define USB_SC_8020	0x02		/* SFF-8020i, MMC-2, ATAPI (CD-ROM) */
#define USB_SC_QIC	0x03		/* QIC-157 (tape) */
#define USB_SC_UFI	0x04		/* UFI (floppy) */
#define USB_SC_8070	0x05		/* SFF-8070i (removable) */
#define USB_SC_SCSI	0x06		/* Transparent SCSI */


enum data_direction {
	DATA_DIR_UNKNOWN = 0,
	DATA_DIR_FROM_HOST,
	DATA_DIR_TO_HOST,
	DATA_DIR_NONE
};

struct msc_device_desc_info {
	uint16_t *idVendor;
	uint16_t *idProduct;
	uint16_t *bcdDevice;
	char *strVendor;
	char *StrProduct;
	char *StrConfig;

	uint8_t *bmAttributes;
	uint8_t *bMaxPower;

	char *msc_VendorName;
	char *msc_ProductName;
	char *msc_Release;
};


/* callback function type define */
typedef void (*usbd_drv_msc_std_cmd_rdata_cb_t)(uint32_t addr, uint32_t len, uint8_t *buf);
typedef void (*usbd_drv_msc_std_cmd_wdata_cb_t)(uint32_t addr, uint32_t len, uint8_t *buf);
typedef void (*usbd_drv_msc_vendor_cmd_cb_t)(uint8_t *cmd, uint32_t data_len, uint8_t flags);
typedef void (*usbd_drv_msc_vendor_cmd_rdata_cb_t)(uint8_t* cmd, uint32_t len, uint8_t *buf);
typedef void (*usbd_drv_msc_vendor_cmd_wdata_cb_t)(uint8_t* cmd, uint32_t len, uint8_t *buf);

/* MSC device callback functions */
void usbd_drv_msc_std_cmd_rdata_reg_cb(usbd_drv_msc_std_cmd_rdata_cb_t cb);
void usbd_drv_msc_std_cmd_wdata_reg_cb(usbd_drv_msc_std_cmd_wdata_cb_t cb);
void usbd_drv_msc_vendor_cmd_reg_cb(usbd_drv_msc_vendor_cmd_cb_t cb);
void usbd_drv_msc_vendor_cmd_rdata_reg_cb(usbd_drv_msc_vendor_cmd_rdata_cb_t cb);
void usbd_drv_msc_vendor_cmd_wdata_reg_cb(usbd_drv_msc_vendor_cmd_wdata_cb_t cb);


// Variable


// Functions
int usbd_msc_init(void);
int usbd_msc_uninit(void);
void usbd_msc_vendor_read_status();
void usbd_msc_vendor_write_status();
void usbd_drv_msc_set_desc_info(struct msc_device_desc_info *msc_desc_info);



#ifdef __cplusplus
}
#endif


#endif
