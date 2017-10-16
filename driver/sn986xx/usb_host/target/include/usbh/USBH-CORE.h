// $Source: $				    
// *******************************************************************			  
//	   (C) Copyright 2015 by SONiX Technology Corp.
//
//		       All Rights Reserved
//
// This	program/document/source	code is	an unpublished copyrighted   
// work	which is proprietary to	SONiX Technology Corp.,	and contains 
// confidential	information that is not	to be reproduced or disclosed
// to any other	person or entity without prior written consent from
// SONiX Technology Corp. in each and every instance.
//
// *******************************************************************
// Author(s): IP2
// 
// Description:	
//
// $Date: 
// 
// $Log: $
// *******************************************************************
// SN98660A_Free_RTOS
// SVN $Rev: 
// SVN $Date:
// ------------------------------------------------------------------
/**
 * @file
 * this	is USBH-CORE file, include this	file before use
 * @author IP2 Dept Sonix. (Hammer Huang #1359)
 */
#ifndef	USBH_CORE__H
#define	USBH_CORE__H   

#include "EHCI-HCD.h"

#include <generated/snx_sdk_conf.h> 



extern unsigned	int starttime,midtime,endtime;

// USBH	Interface class
#define	USBH_AUDIO_CLASS_CODE		0x01
#define	USBH_HID_CLASS_CODE		0x03
#define	USBH_MASS_STORAGE_CLASS_CODE	0x08
#define	USBH_HUB_CLASS_CODE		0x09
#define	USBH_VIDEO_CLASS_CODE		0x0E
// USBH	Xfr Type
#define	USBH_NONE_TYPE		0x00
#define	USBH_CX_TYPE		0x01
#define	USBH_BK_OUT_TYPE	0x02
#define	USBH_BK_IN_TYPE		0x03
#define	USBH_INT_OUT_TYPE	0x04
#define	USBH_INT_IN_TYPE	0x05
#define	USBH_ISO_OUT_TYPE	0x06
#define	USBH_ISO_IN_TYPE	0x07

#define	CX_Read		1
#define	CX_Write	2
#define	CX_NoneData	3

#define	USBH_CONFIGURATION_NUM_MAX	0x02
#define	USBH_INTERFACE_NUM_MAX		0x09
#define	USBH_ENDPOINT_NUM_MAX		0x09
#define	USBH_CLASS_NUM_MAX		0x03

#define	USBH_CONFIGURATION_LENGTH	0x09
#define	USBH_INTERFACE_LENGTH		0x09
#define	USBH_IAD_LENGTH			0x08
#define	USBH_ENDPOINT_LENGTHX		0x07
#define	USBH_CLASS_LENGTHX		0x09

//Port define 
#define  USBH_ROOT	0
#define  USBH_HUB	1
#define  USBH_P0	2
#define  USBH_P1	3
#define  USBH_P2	4
#define  USBH_P3	5
    
// String definition	
#define bStringManufactureLength	0x40
#define bStringProductLength		0x40
#define bStringSerialNLength		0x40

// For Descriptor Parser
#define	USBH_ED_ISO			0x01
#define	USBH_ED_BULK			0x02
#define	USBH_ED_INT			0x03
#define	USBH_ED_Control			0x00


/* Standard Descriptor Types */
#define	USBH_DEVICE			0x01
#define	USBH_CONFIGURATION		0x02
#define	USBH_STRING			0x03
#define	USBH_INTERFACE			0x04
#define	USBH_ENDPOINT			0x05
#define	USBH_DEVICE_QUALIFIER		0x06
#define	USBH_OTHER_SPEED_CONFIGURATION	0x07
#define	USBH_INTERFACE_POWER		0x08
#define	USBH_INTERFACE_ASSOCIATION	0x0B


/** \defgroup USBH_AUTO_BK_IN AUTO BK IN Packet	definition
 * \ingroup WIFI_MODULE
 * 
 * @{
 */ 
// For AUTO BULK IN
#define	PACKET_SIZE		2048	/**< Size */
#define	PACKET_MAX_CNT		20	/**< Count */
/** @} */

typedef	enum
{
	USBH_PARSER_ROOT_DET,
	USBH_PARSER_ROOT_RST,
	USBH_PARSER_CX_ENQU,
	USBH_PARSER_NONCX_ENQU,
	USBH_PARSER_WAKE_CLASS_DRV,	
	USBH_PARSER_ROOT_DEQ,   
	USBH_PARSER_ROOT_HUB_DEQ,
	USBH_PARSER_PORT_HUB_DEQ,

}USBH_PARSER_INDEX; 


// USBHOst state machine 
typedef	enum
{
	USBH_ROOT_DET_STATE,
	USBH_ROOT_PLUG_OUT_STATE,
	USBH_ROOT_PLUG_IN_STATE,
	USBH_ROOT_ENUM_STATE,
	USBH_ROOT_PARSER_STATE,
	USBH_ROOT_ACTIVE_STATE,
	USBH_ROOT_ERR_HDL_STATE,
	USBH_ROOT_HALT_STATE
}USBH_ENUM_SM_INDEX;

typedef	enum
{
	BULK_IN_EP = 1,
	BULK_OUT_EP

}BK_EP_INDEX;

// Device Descriptor 
typedef struct
{
	uint8_t bDEVICE_LENGTH;		// bLength
	uint8_t bDT_DEVICE;		// bDescriptorType
	uint8_t bVerLowByte;		// bcdUSB
	uint8_t bVerHighByte;

	uint8_t bDeviceClass;		// bDeviceClass
	uint8_t bDeviceSubClass;	// bDeviceSubClas;
	uint8_t bDeviceProtocol;	// bDeviceProtocol
	uint8_t bEP0MAXPACKETSIZE;	// bMaxPacketSize0

	uint8_t bVIDLowByte;		// idVendor
	uint8_t bVIDHighByte;
	uint8_t bPIDLowByte;		// idProduct
	uint8_t bPIDHighByte;
	uint8_t bRNumLowByte;		// bcdDeviceReleaseNumber
	uint8_t bRNumHighByte;

	uint8_t bManufacturer;		// iManufacturer
	uint8_t bProduct;		// iProduct
	uint8_t bSerialNumber;		// iSerialNumber
	uint8_t bCONFIGURATION_NUMBER;	// bNumConfigurations	  
}USBH_Device_Descriptor_Struct;

typedef struct
{
	uint8_t		bED_Length;
	uint8_t		bED_bDescriptorType;
	uint8_t		bED_EndpointAddress;
	uint8_t		bED_bmAttributes;
	uint8_t		bED_wMaxPacketSizeLowByte;
	uint8_t		bED_wMaxPacketSizeHighByte;
	uint8_t		bED_Interval;
	uint8_t		bED_Toggle;

}USBH_EndPoint_Descriptor_Struct;


typedef struct
{

	uint8_t		bClass_LENGTH;
	uint8_t		bClaNumberss;
	uint8_t		bClassVerLowByte;
	uint8_t		bClassVerHighByte;
	uint8_t		bCityNumber;
	uint8_t		bFollowDescriptorNum;
	uint8_t		bReport;
	uint8_t		bLengthLowByte;
	uint8_t		bLengthHighByte;

}USBH_Class_Descriptor_Struct;

//	Interface Descriptor
typedef struct
{

	uint8_t bINTERFACE_LENGTH;		// bLength
	uint8_t bDT_INTERFACE;			// bDescriptorType INTERFACE
	uint8_t bInterfaceNumber;		// bInterfaceNumber
	uint8_t bAlternateSetting;		// bAlternateSetting
	uint8_t bEP_NUMBER;			// bNumEndpoints(excluding endpoint	zero)
	uint8_t bInterfaceClass;		// bInterfaceClass
	uint8_t bInterfaceSubClass;		// bInterfaceSubClass
	uint8_t bInterfaceProtocol;		// bInterfaceProtocol
	uint8_t bInterface;			// iInterface

	//USBH_Interface_Association_Descriptor_Struct  ASSOCDes;	    
	USBH_Class_Descriptor_Struct		CLASSDes[USBH_CLASS_NUM_MAX];
	USBH_EndPoint_Descriptor_Struct		ENDPDes[USBH_ENDPOINT_NUM_MAX];	   
	
}USBH_Interface_Descriptor_Struct;

 
// Config Descriptor 
typedef struct
{
	uint8_t  bCONFIG_LENGTH;		// bLength
	uint8_t  bDT_CONFIGURATION;		// bDescriptorType CONFIGURATION
	uint8_t  bTotalLengthLowByte;		// wTotalLength, include all descriptors
	uint8_t  bTotalLengthHighByte;
	uint8_t  bINTERFACE_NUMBER;		// bNumInterface
	uint8_t  bConfigurationValue;		// bConfigurationValue
	uint8_t  bConfiguration;		// iConfiguration
	uint8_t  bAttribute;			// bmAttribute
	uint8_t  bMaxPower;			// iMaxPower (2mA	units)
	USBH_Interface_Descriptor_Struct	Interface[USBH_INTERFACE_NUM_MAX];

}USBH_Configuration_Descriptor_Struct;

// USB STATUS Descriptor 
typedef struct
{
	uint16_t	SelfPowered:1;
	uint16_t	RemoteWakeup:1; 
	uint16_t	RESERVE:14;	
	
}USBH_STANDARD_STATUS_Struct;


// Resource buff 
typedef struct
{
	uint32_t	size;
	void		*ptr;

}USBH_REOSURCE_BUFF_Struct;


// Plug IN - Out CallBack
typedef struct
{	
	void					*in;
	void					*out;

}USBH_CB_Struct;

typedef struct
{
	uint32_t				class_type;
	USBH_CB_Struct				CB;

}USBH_PLUG_Struct;


// Device Structure 

//!  USB Host Device Structure 
/*!
*/
typedef struct
{

	//<1>.ENUM Information
	uint32_t				ENUM_SM;					//!< Enum State Machine
	uint32_t				SPEED;						//!< USB Speed , High speed	, Full speed or	Low speed
	uint32_t				bDevIsConnected;				//!< Reserve for HUB Class
	uint32_t				bAdd;						//!< Device	Address
	uint32_t				CLASS_DRV;					//!< Class Driver attribute
	void					*CLASS_STRUCT_PTR;				//!< Class Structure pointer
	USBH_REOSURCE_BUFF_Struct		BUFF;

	//<2>.Descriptor Information
	USBH_Device_Descriptor_Struct		DEVDes;						//!< USB Device Descriptor 
	USBH_Configuration_Descriptor_Struct	CFGDes[USBH_CONFIGURATION_NUM_MAX];		//!< USB Configuration descriptor
	USBH_STANDARD_STATUS_Struct		STDSTS;

	//<3>.String  Information
	uint8_t					bStringLanguage[10];				//!< USB String language
	uint8_t					bStringManufacture[bStringManufactureLength];	//!< USB String Manufacture
	uint8_t					bStringProduct[bStringProductLength];		//!< USB String Product
	uint8_t					bStringSerialN[bStringSerialNLength];		//!< USB String Serial Number

	//<4>.EHCI Strucutre Information
	EHCI_ENDP_STRUCT			EP[MAX_QH_PER_DEVICE];				//!< EHCI ENDP Data Strucutre

	//<5>.Other	Information
	uint8_t					bData[2014];

	uint8_t					iEPNUM;						// current endpoint number
	uint8_t					iDEVID;						// current dev id

	uint8_t					iPortNum;					// port number

}USBH_Device_Structure;
/** @}	*/

//!  USB Host Control transfer	request	strucutre 
/*!
*/
typedef	struct
{
	uint32_t		CMD;		//!< CX	Command	
	uint32_t		SIZE;		//!< CX	Data statge size 
	uint32_t		ACT_SIZE;	//!< Actually transfer size 
	uint32_t		*pBUFF;		//!< Data stage	Data pointer 
	uint8_t			CLASS[8];	//!< Class/Vendor Setup	command	
	uint8_t			bRequest;	//!< Request type
	uint16_t		wValue;		//!< wValue
	uint16_t		wIndex;		//!< wIndex
	uint8_t			CX_Case;	//!< CX	Read / CX Write	/ CX none Data
	uint8_t			*SETUP_CMD_BUFF;//!< Setup stage command buffer	
	uint8_t			*STS_DUMMY_BUFF;//!< status stage buffer 
	uint16_t		TimeoutMsec;
	
}USBH_CX_XFR_REQ_Struct;
/** @}	*/

//!  USB Host bulk transfer request strucutre
/*!
*/
typedef	struct
{
	uint32_t		XfrType;	//!< BK Transfer type
	uint32_t		NUM;		//!< Endp order
	uint32_t		SIZE;		//!< BK Transfer Size
	uint32_t		ACT_SIZE;	//!< BK Actually transfer size
	uint32_t		*pBUFF;		//!< BK Data stage data pointer
	uint32_t		*BULK_PING_PTR; //for uvc bulk use
	uint32_t		*BULK_PONG_PTR; //for uvc bulk use
	uint32_t		BULK_SM; //for uvc bulk use
	uint32_t		Start; //for uvc bulk
	uint32_t		XfrDone;
	uint16_t		TimeoutMsec;
}USBH_BK_XFR_REQ_Struct;

/** @}	*/

//!  USB Host	INT transfer request strucutre
/*!
*/
typedef	struct
{
	uint32_t		XfrType;	//!< INT Transfer type
	uint32_t		NUM;		//!< Endp order
	uint32_t		SIZE;		//!< INT Transfer Size
	uint32_t		ACT_SIZE;	//!< INT Actually transfer size
	uint32_t		*pBUFF;		//!< INT Data stage data pointer
	uint16_t		TimeoutMsec;
}USBH_INT_XFR_REQ_Struct;


//UVC REQUEST Index
typedef	enum
{
	USBH_STD_ISO_W=0,	
	USBH_STD_ISO_X=1,	
	USBH_STD_ISO_Y=2,	
	USBH_STD_ISO_Z=3,		
		
}USBH_UVC_STD_ISO_REQUEST_INDEX;


/** @}	*/

//!	USB Host ISO transfer request strucutre
/*!
*/
typedef struct
{
#if 0
	uint32_t		START;
	uint32_t		XfrType;	//!< ISO Transfer type 
	uint32_t		NUM;		//!< Endp order 
	uint32_t		SIZE;		//!< ISO Transfer Size  
	uint32_t		*RING_BUFF_PTR;
	uint32_t		RING_BUFF_SIZE;
	uint32_t		SXITD_Discard_En:1;
	uint32_t		SXITD_Header_Filter:1;
	uint32_t		STD_ISO_REQ:6;
	uint32_t		STD_ISO_SM:4;	
	uint32_t		SXITD_Reserve:20;	
	uint32_t		*STD_ISO_PING_PTR;	
	uint32_t		*STD_ISO_PONG_PTR;	
	uint32_t		IntfNum;
	uint32_t		MaxPktSize;
#else
	uint32_t		START;
	uint32_t		XfrType;	//!< ISO Transfer type 
	uint32_t		NUM;		//!< Endp order 
	uint32_t		SIZE;		//!< ISO Transfer Size	
	uint32_t		*RING_BUFF_PTR;
	uint32_t		RING_BUFF_SIZE;	
	uint32_t		*STD_ISO_PING_PTR;	
	uint32_t		*STD_ISO_PONG_PTR;	
	uint32_t		IntfNum;
	uint32_t		MaxPktSize;
	uint32_t		SXITD_Discard_En;
	uint32_t		SXITD_Header_Filter;
	uint32_t		STD_ISO_REQ;
	uint32_t		STD_ISO_SM;	
	uint32_t		SXITD_Reserve;	

#endif 
}USBH_ISO_XFR_REQ_Struct;

/** @} */


// CX CMD Index
typedef	enum
{
	// Standard	
	USBH_CX_CMD_GETDESCRIPTOR_DEVICE,	  
	USBH_CX_CMD_GETDESCRIPTOR_CONFIG,	  
	USBH_CX_CMD_SETADDRESS,
	USBH_CX_CMD_SETCONFIGURATION,
	USBH_CX_GETDESCRIPTOR_STR_LANG,
	USBH_CX_CMD_GETDESCRIPTOR_STR_Manufacturer,
	USBH_CX_CMD_GETDESCRIPTOR_STR_Product,
	USBH_CX_CMD_GETDESCRIPTOR_STR_Serial,
	USBH_CX_CMD_GETSTATUS_DEVICE,
	USBH_CX_CMD_GETSTATUS_INTERFACE,
	USBH_CX_CMD_GETSTATUS_ENDPOINT,
	USBH_CX_CMD_CLEAR_FEATURE,
	USBH_CX_CMD_SET_INTERFACE,

	// Vendor Class 
	USBH_CX_CMD_CLASS_Read,
	USBH_CX_CMD_CLASS_Write,
	USBH_CX_CMD_CLASS_NoneData
	
}USBH_CX_CMD_INDEX;

// AUTO	BULK IN	sturcture
typedef	struct AUTO_BKIN{
	uint32_t	base_addr;
	uint32_t	boundary_addr;
	uint32_t	start_addr;
	uint8_t		xfr_cnt;
	uint8_t		buff_full;
}AUTO_BKIN_STRUCTURE;


typedef	struct{
	void *EP;
	void *ISO_REQ;
}USBH_STD_ISO_QUEUE_STRUCT;


typedef	struct{
	void *EP;
	void *BULK_REQ;
}USBH_BULK_QUEUE_STRUCT;

#if defined( CONFIG_SNX_ISO_ACCELERATOR )
#else
	extern QueueHandle_t	USBH_QUEUE_STD_ISO_REQ;		
#endif
	extern QueueHandle_t	USBH_QUEUE_BULK_REQ[];

// Global 
extern volatile	USBH_Device_Structure USBH_DEV[];
extern AUTO_BKIN_STRUCTURE AUTO_BK_IN;
// Extern Prototype 
extern void EHCI_DEBUG ( uint8_t DebugLevel,const char * format, ... );
extern void usbh_core_init(void);
extern void usbh_root_enum(void);
extern uint8_t usbh_cx_xfr(USBH_Device_Structure *DEV,USBH_CX_XFR_REQ_Struct *CX_REQ);
extern uint8_t usbh_bk_xfr(USBH_Device_Structure *DEV,USBH_BK_XFR_REQ_Struct *BK_REQ);
extern uint8_t usbh_int_xfr(USBH_Device_Structure *DEV,USBH_INT_XFR_REQ_Struct *INT_REQ);
extern uint8_t	usbh_iso_xfr(USBH_Device_Structure *DEV,USBH_ISO_XFR_REQ_Struct	*ISO_REQ);
extern void usbh_iso_struct_init(void* EP, USBH_ISO_XFR_REQ_Struct *ISO_REQ);


extern AUTO_BKIN_STRUCTURE get_auto_bkin_struct(void);
extern void set_auto_bkin_processed_cnt(int xfr_cnt);
extern uint8_t	usbh_parser_dev(USBH_Device_Structure *	DEV,uint8_t FUN);
extern uint8_t	usbh_enum(  USBH_Device_Structure * DEV,uint8_t	addr);
extern void usbh_force_root_plug_out(void);

 
#endif //USB_CORE__H



