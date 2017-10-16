// $Source: $					
// *******************************************************************			  
//	   (C) Copyright 2015 by SONiX Technology Corp.
//
//			   All Rights Reserved
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
 * this	is UVC file, include this file before use
 * @author IP2 Dept Sonix. (Hammer Huang #1359)
 */

#ifndef	UVC__H
#define	UVC__H	 
#if defined( __cplusplus )
extern "C"{
#endif

#include "EHCI-HCD.h"
#include "USBH-CORE.h"
#include "USBH.h"
#include <generated/snx_sdk_conf.h> 


#if defined( CONFIG_SN_GCC_SDK )
#if defined( CONFIG_MODULE_USB_UVC_DEBUG )
	#define UVC_DBG(format, args...) print_msg("\nUVC_DBG:"format, ##args);	/**< MSC Debug Level 1 */	
#else
	#define UVC_DBG(args...) /**< MSC Debug Level 0 */
#endif
	#define UVC_INFO(format, args...) print_msg_queue("\nUVC_INFO:"format, ##args)
#endif 


#if defined( CONFIG_SN_KEIL_SDK )
#include "stdio.h"
#if defined( CONFIG_MODULE_USB_UVC_DEBUG )
	#define UVC_DBG(format, args...) printf("\nUVC_DBG:"format, ##args);	/**< MSC Debug Level 1 */	
#else
	#define UVC_DBG(args...) /**< MSC Debug Level 0 */
#endif
	#define UVC_INFO(format, args...) printf("\nUVC_INFO:"format, ##args)
#endif	


#if defined( CONFIG_XILINX_SDK )
#include "xil_printf.h"
#if defined( CONFIG_MODULE_USB_UVC_DEBUG )
	#define UVC_DBG(format, args...) xil_printf("\nUVC_DBG:"format, ##args);	/**< MSC Debug Level 1 */	
#else
	#define UVC_DBG(args...) /**< MSC Debug Level 0 */
#endif
	#define UVC_INFO(format, args...) xil_printf(format, ##args)
#endif

// --------------------------------------------------------------------------
// UVC constants
// --------------------------------------------------------------------------
extern SemaphoreHandle_t	USBH_SEM_WAKEUP_UVC_DRV;
#if defined( CONFIG_SN_KEIL_SDK )
extern SemaphoreHandle_t	USBH_SEM_WAKEUP_UVC_APP;
#endif 
// UVC Request cmd
#define	USBH_UVC_STREAM_STOP			0
#define	USBH_UVC_STREAM_START			1

// UVC Return sts
#define	USBH_UVC_STREAM_NORMAL			0
#define	USBH_UVC_STREAM_FAIL			1
#define	USBH_UVC_STREAM_OUT_OF_BANDWIDTH	2

// UVC stream valid
#define	USBH_UVC_STREAM_INVALID			0
#define	USBH_UVC_STREAM_VALID			1

// UVC steram format
#define	USBH_UVC_STREAM_UNKNOW			0
#define	USBH_UVC_STREAM_H264			1
#define	USBH_UVC_STREAM_MJPEG			2
#define	USBH_UVC_STREAM_YUV			3

// UVC stream resolution
#define	USBH_UVC_STREAM_1920X1080		11
#define	USBH_UVC_STREAM_1280X800		1
#define	USBH_UVC_STREAM_1280X720		2
#define	USBH_UVC_STREAM_960X540			3
#define	USBH_UVC_STREAM_848X480			4
#define	USBH_UVC_STREAM_640X480			5
#define	USBH_UVC_STREAM_640X360			6
#define	USBH_UVC_STREAM_424X240			7
#define	USBH_UVC_STREAM_320X240			8
#define	USBH_UVC_STREAM_320X180			9
#define	USBH_UVC_STREAM_160X120			10
// UVC stream frame rate
#define	USBH_UVC_STREAM_5_FPS			0x001E8480
#define	USBH_UVC_STREAM_10_FPS			0x000F4240
#define	USBH_UVC_STREAM_15_FPS			0x000A2C2A
#define	USBH_UVC_STREAM_20_FPS			0x0007A120
#define	USBH_UVC_STREAM_25_FPS			0x00061a80
#define	USBH_UVC_STREAM_30_FPS			0x00051615

#define	MAX_FORMAT_NUM				5
#define	MAX_RESOLUTION_NUM			10
#define	MAX_FRAME_RATE_NUM			10

// UVC preview 
#define USBH_UVC_PREVIEW_START		1
#define USBH_UVC_PREVIEW_STOP		0

//Sonix	ITD 
#define	SNX_ITD_DISCARD_EN			1

// UVC stream buffer size
#define USBH_UVC_BK_STREAM_BUF_SIZE		(MAX_QTD_PER_QH-1)*20*1024 //(MAX_QTD-1)*20k

typedef	struct
{
	SemaphoreHandle_t			mutex;
	SemaphoreHandle_t			sem;
	uint8_t					cmd;
	uint8_t					status;

}USBH_UVC_REQ_Structure;


#define	USBH_UVC_DATA_QUEUE_SIZE		10


typedef	struct
{

	uint32_t			id;
	uint32_t			size;
	uint32_t			*ptr;
	uint32_t			ring_buff_end;
	uint32_t			ring_buff_start;
	uint32_t			err;
	uint32_t			babble;
	uint32_t			discard;
	uint32_t			underflow;

}USBH_UVC_STREAM_Structure;

#if 0

//UVC REQUEST Index
typedef	enum
{
	UVC_STD_ISO_STOP,
	UVC_STD_ISO_SM1,
	UVC_STD_ISO_SM2,	
	UVC_STD_ISO_SM3,	
	UVC_STD_ISO_SM4	
		
}USBH_UVC_STD_ISO_REQUEST_INDEX;

typedef	struct
{

	uint32_t			sm:3;
	uint32_t			reserve:30;
	
}USBH_UVC_STD_ISO_Structure;
#endif


typedef	struct
{

	uint32_t			size;
	uint32_t			*ptr;

}USBH_UVC_DATA_Structure;


typedef	struct
{

	uint8_t				valid;
	uint8_t				format;
	uint16_t			res_hor;
	uint16_t			res_vert;
	USBH_UVC_DATA_Structure		data;

}USBH_UVC_INFO_Structure;

typedef	struct
{

	USBH_UVC_STREAM_Structure	stream[USBH_UVC_DATA_QUEUE_SIZE];
	uint32_t			index;
	SemaphoreHandle_t		mutex;

}USBH_UVC_DATA_QUEUE_Structure;


typedef	struct
{
	uint32_t	dwStreamID;
	uint32_t	dwFrameInterval;
}USBH_UVC_FRAME_RATE_Struct;

typedef	struct
{
	uint8_t		bFpsNum;
	uint8_t		bFrameIndex;
	uint8_t		bPixel;
	uint16_t	wWidth;
	uint16_t	wHeight;

	USBH_UVC_FRAME_RATE_Struct	FPS[MAX_FRAME_RATE_NUM];
}USBH_UVC_RESOLUTION_Struct;

typedef	struct
{
	uint8_t		bResNum;
	uint8_t		bFmtType;
	uint8_t		bFormatIndex;
	uint8_t		bDefFrameIndex;

	USBH_UVC_RESOLUTION_Struct	RES[MAX_RESOLUTION_NUM];
}USBH_UVC_FORMAT_Struct;

typedef	struct
{
	uint8_t			bFmtNum;
	uint8_t			bIntfIndex;
	USBH_UVC_FORMAT_Struct	FMT[MAX_FORMAT_NUM];
	EHCI_ENDP_STRUCT	*EP;
}USBH_UVC_INTF_Struct;

typedef struct
{
	uint8_t		iDevID;
	uint8_t		bPrv;
	uint8_t		bIntfNum;	
	USBH_UVC_INTF_Struct	INTF[USBH_INTERFACE_NUM_MAX];
}USBH_UVC_DEV_Struct;

typedef	struct
{
	uint8_t		bCurDevCnt;
	USBH_UVC_DEV_Struct		DEV[USBH_MAX_PORT];
}USBH_UVC_INFO_Struct;

typedef struct
{
	uint8_t  stream_ctrl;
	uint32_t stream_size;
	uint32_t *stream_ptr;
	uint32_t stream_id;	
}USBH_UVC_PREVIEW;

#if 0
typedef	struct
{
	uint8_t	bFmtNum;
	USBH_UVC_FORMAT_Struct	FMT[MAX_FORMAT_NUM];	
}USBH_UVC_INFO_Struct;
#endif


#define	UVC_INTERFACE_NUM_MAX	0x09
#define	UVC_ENDPOINT_NUM_MAX	0x09
#define	UVC_FRAME_NUM_MAX	0x10
#define	MAX_DES_NUM		0x06
#define	UVC_STREAMID_NUM_MAX	0x10

// UVC SM Index
typedef	enum
{
	USBH_UVC_INIT_STATE	=0,
	USBH_UVC_PARSER_STATE,
	USBH_UVC_WAKEUP_APP_STATE,
	USBH_UVC_GET_DATA_STATE
}USBH_UVC_SM_INDEX;

//UVC REQUEST Index
typedef	enum
{
	USBH_UVC_PROBE_NONE,
	USBH_UVC_GET_CUR,
	USBH_UVC_SET_CUR,
	USBH_UVC_GET_MAX,
	USBH_UVC_GET_MIN,
	USBH_UVC_GET_RES,
	USBH_UVC_GET_LEN,
	USBH_UVC_GET_INFO,
	USBH_UVC_GET_DEF,
	USBH_UVC_PROBE_DONE
}USBH_UVC_REQUEST_INDEX;

 // INTERFACE_ASSOCIATION Descriptor 
 typedef struct
{

	uint8_t  bLength;
	uint8_t  bDescriptorType;
	uint8_t  bFirstInterface;
	uint8_t  bInterfaceCount;
	uint8_t  bFunctionClass;
	uint8_t  bFunctionSubClass;
	uint8_t  bFunctionProtocol;
	uint8_t  iFunction;
	
}USBH_Interface_Association_Descriptor_Struct;
 
 //======================== VIDEO DESCRIPTOR STRUCTURE =======================/
// Input Terminal Descriptor(Camera)
typedef	struct
{
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint8_t bDescriptorSubType;
	uint8_t bTerminalID;
	uint8_t bTerminalTypeLowByte;			//wTerminalType
	uint8_t bTerminalTypeHighByte;	
	uint8_t bAssocTerminal;
	uint8_t iTerminal;
	uint8_t bObjectiveFocalLengthMinLowByte;	//wObjectiveFocalLengthMin
	uint8_t bObjectiveFocalLengthMinHighByte;
	uint8_t bObjectiveFocalLengthMaxLowByte;	//wObjectiveFocalLengthMax
	uint8_t bObjectiveFocalLengthMaxHighByte;
	uint8_t bOcularFocalLengthLowByte;		//wOcularFocalLength
	uint8_t bOcularFocalLengthHighByte;
	uint8_t bControlSize;
	uint8_t bmControls[3];

}USBH_UVC_Camera_IT_Descriptor_Struct;

// Input Terminal Descriptor(Media Transport)
typedef	struct
{
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint8_t bDescriptorSubType;
	uint8_t bTerminalID;
	uint8_t bTerminalTypeLowByte;		//wTerminalType
	uint8_t bTerminalTypeHighByte;
	uint8_t bAssocTerminal;
	uint8_t iTerminal;
	uint8_t bControlSize;
	uint8_t bmControls;
	uint8_t bTransportModeSize;
	uint8_t bmTransportModes[5];

}USBH_UVC_Media_Transport_IT_Descriptor_Struct;

// Output Terminal Descriptor
typedef	struct
{
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint8_t bDescriptorSubType;
	uint8_t bTerminalID;
	uint8_t bTerminalTypeLowByte;		//wTerminalType
	uint8_t bTerminalTypeHighByte;
	uint8_t bAssocTerminal;
	uint8_t bSourceID;
	uint8_t iTerminal;

}USBH_UVC_OT_Descriptor_Struct;

// Selector Unit Descriptor
typedef	struct
{
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint8_t bDescriptorSubType;
	uint8_t bUnitID;
	uint8_t bNrInPins;
	uint8_t baSourceID;
	uint8_t iSelector;

}USBH_UVC_SU_Descriptor_Struct;

// Processing Unit Descriptor
typedef	struct
{
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint8_t bDescriptorSubType;
	uint8_t bUnitID;
	uint8_t bSourceID;
	uint8_t bMaxMultiplierLowByte;		//wMaxMultiplier
	uint8_t bMaxMultiplierHighByte;
	uint8_t bControlSize;
	uint8_t *bmControls;			//bmControls
	uint8_t iProcessing;

}USBH_UVC_PU_Descriptor_Struct;

// Extension Unit Descriptor
typedef	struct
{
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint8_t bDescriptorSubType;
	uint8_t bUnitID;
	uint8_t bGuidExtensionCode[16];
	uint8_t bNumControls;
	uint8_t bNrInPins;
	uint8_t baSourceID;
	uint8_t bControlSize;
	uint8_t *bmControls;
	uint8_t iExtension;

}USBH_UVC_XU_Descriptor_Struct;

// Class - specific Interrupt Endpoint Descriptor
typedef	struct
{
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint8_t bDescriptorSubType;
	uint8_t bMaxTransferSizeLowByte;		//wMaxTransferSize
	uint8_t bMaxTransferSizeHighByte;
}USBH_UVC_CS_EndPoint_Descriptor_Struct;

// Class - specific VC Interface Descriptor
typedef	struct
{
	uint8_t	bLength;
	uint8_t	bDescriptorType;
	uint8_t	bDescriptorSubType;
	uint8_t	bVerLowByte;			//bcdUSB
	uint8_t	bVerHighByte;
	uint8_t	bTotalLengthLowByte;		//wTotalLength
	uint8_t	bTotalLengthHighByte;
	uint8_t	bClockFrequency[4];		//dwClockFrequency
	uint8_t	bInCollection;
	uint8_t	*baInterfaceNr;

	USBH_UVC_CS_EndPoint_Descriptor_Struct CS_ENDPDes[UVC_ENDPOINT_NUM_MAX];
}USBH_UVC_CS_Interface_Descriptor_Struct;

// Class - specific Color Matching Descriptor
typedef	struct
{
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint8_t bDescriptorSubType;
	uint8_t bColorPrimaries;
	uint8_t TransferChart;
	uint8_t bMatrixCoefficients;
}USBH_UVC_CS_Color_Matching_Descriptor_Struct;

#if 0
// Class - specific Still Image	Frame Descriptor 
typedef	struct
{
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint8_t bDescriptorSubType;
	uint8_t bEndpointAddress;
	uint8_t bNumImageSizePatterns;
	uint16_t *wWidth;
	uint16_t *wHigh;
	uint8_t bNumCompressionPtn;
	uint8_t bCompression;
}USBH_UVC_Class_Specific_Still_Image_Frame_Descriptor_Struct;
#endif

// Class - specific VS Frame Descriptor
typedef	struct
{
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint8_t bDescriptorSubType;
	uint8_t bFrameIndex;
	uint8_t bmCapabilities;
	uint8_t bWidthLowByte;			//wWidth
	uint8_t bWidthHighByte;
	uint8_t bHeightLowByte;			//wHeight
	uint8_t bHeightHighByte;
	uint8_t bMinBitRate[4];			//dwMinBitRate
	uint8_t bMaxBitRate[4];			//dwMaxBitRate
	uint8_t bMaxVideoFrameBufSize[4];	//dwMaxVideoFrameBufSize
	uint8_t bDefaultFrameInterval[4];	//dwDefaultFrameInterval 
	uint8_t bFrameIntervalType;
	uint32_t *dwFrameInterval;		//dwFrameInterval
}USBH_UVC_CS_Frame_Descriptor_Struct;

// Frame Based Payload Video Frame Descriptor
typedef	struct
{
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint8_t bDescriptorSubType;
	uint8_t bFrameIndex;
	uint8_t bmCapabilities;
	uint8_t bWidthLowByte;			//wWidth
	uint8_t bWidthHighByte;
	uint8_t bHeightLowByte;			//wHeight
	uint8_t bHeightHighByte;
	uint8_t bMinBitRate[4];			//dwMinBitRate
	uint8_t bMaxBitRate[4];			//dwMaxBitRate
	uint8_t bDefaultFrameInterval[4];	//dwDefaultFrameInterval
	uint8_t bFrameIntervalType;
	uint8_t bBytesPerLine[4];		//dwBytesPerLine
	uint32_t *dwFrameInterval;		//dwFrameInterval
}USBH_UVC_Frame_Based_Frame_Descriptor_Struct;

// Uncompressed	Video Frame Descriptor
typedef	struct
{
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint8_t bDescriptorSubType;
	uint8_t bFrameIndex;
	uint8_t bmCapabilities;
	uint8_t bWidthLowByte;			//wWidth
	uint8_t bWidthHighByte;
	uint8_t bHeightLowByte;			//wHeight
	uint8_t bHeightHighByte;
	uint8_t bMinBitRate[4];			//dwMinBitRate
	uint8_t bMaxBitRate[4];			//dwMaxBitRate
	uint8_t bMaxVideoFrameBufferSize[4];	//dwMaxVideoFrameBufferSize
	uint8_t bDefaultFrameInterval[4];		//dwDefaultFrameInterval
	uint8_t bFrameIntervalType;
	uint32_t *dwFrameInterval;			//dwFrameInterval
}USBH_UVC_YUV_Frame_Descriptor_Struct;

// Class - specific VS Format Descriptor
typedef	struct
{
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint8_t bDescriptorSubType;
	uint8_t bFormatIndex;
	uint8_t bNumFrameDescriptors;
	uint8_t bmFlags;
	uint8_t bDefaultFrameIndex;
	uint8_t bAspectRatioX;
	uint8_t bAspectRatioY;
	uint8_t bmInterlaceFlags;
	uint8_t bCopyProtect;   

	USBH_UVC_CS_Frame_Descriptor_Struct	VS_FRAME_Des[UVC_FRAME_NUM_MAX];
}USBH_UVC_CS_Format_Descriptor_Struct;

// Frame Based Payload Video Format Descriptor
typedef	struct
{
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint8_t bDescriptorSubType;
	uint8_t bFormatIndex;
	uint8_t bNumFrameDescriptors;
	uint8_t guidFormat[16];
	uint8_t bBitsPerPixel;
	uint8_t bDefaultFrameIndex;
	uint8_t bAspectRatioX;
	uint8_t bAspectRatioY;
	uint8_t bmInterlaceFlags;
	uint8_t bCopyProtect;
	uint8_t bVariableSize;

	USBH_UVC_Frame_Based_Frame_Descriptor_Struct	FB_FRAME_Des[UVC_FRAME_NUM_MAX];
}USBH_UVC_Frame_Based_Format_Descriptor_Struct;

// Uncompressed	Video Format Descriptor
typedef	struct
{
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint8_t bDescriptorSubType;
	uint8_t bFormatIndex;
	uint8_t bNumFrameDescriptors;
	uint8_t guidFormat[16];
	uint8_t bBitsPerPixel;
	uint8_t bDefaultFrameIndex;
	uint8_t bAspectRatioX;
	uint8_t bAspectRatioY;
	uint8_t bmInterlaceFlags;
	uint8_t bCopyProtect;

	USBH_UVC_YUV_Frame_Descriptor_Struct	YUV_FRAME_Des[UVC_FRAME_NUM_MAX];
}USBH_UVC_YUV_Format_Descriptor_Struct;

// Color Matching Descriptor
typedef	struct
{
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint8_t bDescriptorSubType;
	uint8_t bColorPrimaries;
	uint8_t bTransferChart;
	uint8_t bMatrixCoefficients; 
}USBH_UVC_Clr_Match_Format_Descriptor_Struct;

// Class - specific VS Header Descriptor
typedef	struct
{
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint8_t bDescriptorSubType;
	uint8_t bNumFormats;
	uint8_t bTotalLengthLowByte;		//wTotalLength
	uint8_t bTotalLengthHighByte;
	uint8_t bEndpointAddress;
	uint8_t bmInfo;
	uint8_t bTerminalLink;
	uint8_t bStillCaptureMethod;
	uint8_t bTriggerSupport;
	uint8_t bTriggerUsage;
	uint8_t bControlSize;
	uint8_t *bmaControls;

	USBH_UVC_CS_Format_Descriptor_Struct		VS_FORMAT_Des;
	USBH_UVC_Frame_Based_Format_Descriptor_Struct	FB_FORMAT_Des;
	USBH_UVC_YUV_Format_Descriptor_Struct		YUV_FORMAT_Des;
	USBH_UVC_Clr_Match_Format_Descriptor_Struct	CLRMCH_FORMAT_Des;
}USBH_UVC_CS_Header_Descriptor_Struct;

//======================== AUDIO DESCRIPTOR STRUCTURE DEFINITION =======================//
typedef	struct
{
	uint8_t	bLength;
	uint8_t	bDescriptorType;
	uint8_t	bDescriptorSubType;
	uint8_t	bUnitID;
	uint8_t	bExtensionCodeLowByte;	//wExtensionCode
	uint8_t	bExtensionCodeHighbyte;
	uint8_t	bNrInPins;
	uint8_t	*baSourceID;
	uint8_t	bNrChannels;
	uint8_t	bChannelConfigLowByte;	//wChannelConfig
	uint8_t	bChannelConfigHighByte;
	uint8_t	iChannelNames;
	uint8_t	bControlSize;
	uint8_t	*bmControls;
	uint8_t	iExtension;
}USBH_UAC_XU_Descriptor_Struct;

// Processing Unit Descriptor(
// ex:
// Up/Down-mix,
// Dolby Prologic,
// 3D-Stereo Extender,
// Reverberation,
// Chorus,
// Dynamic Range Compressor)
typedef	struct
{
	uint8_t	bLength;
	uint8_t	bDescriptorType;
	uint8_t	bDescriptorSubType;
	uint8_t	bUnitID;
	uint8_t	bProcessTypeLowByte;	//wProcessType	
	uint8_t	bProcessTypeHighByte;
	uint8_t	bNrInPins;
	uint8_t	bSourceID;
	uint8_t	bNrChannels;
	uint8_t	bChannelConfigLowByte;	//wChannelConfig
	uint8_t	bChannelConfigHighByte;
	uint8_t	iChannelNames;
	uint8_t	bControlSize;
	uint8_t	*bmControls;
	uint8_t	iProcessing;	
	uint8_t	bNrModes;
	uint8_t	*waModes[2];
}USBH_UAC_PU_Descriptor_Struct;

#if UAC_2_0
// Effect Unit Descriptor(
// ex: 
// parametric Equalizer	Section, 
// Reverberation, 
// Modulation Delay,
// Dynamic Range Compressor)
typedef	struct
{
	uint8_t	bLength;
	uint8_t	bDescriptorType;
	uint8_t	bDescriptorSubType;
	uint8_t	bUnitID;
	uint8_t	bEffectTypeLowByte;	//wEffectType
	uint8_t	bEffectTypeHighByte;
	uint8_t	bSourceID;
	uint8_t	*bmaControls[4];
	uint8_t	iEffects;
}USBH_UAC_EU_Descriptor_Struct;

// Sampling Rate Converter Descriptor
typedef	struct
{
	uint8_t	bLength;
	uint8_t	bDescriptorType;
	uint8_t	bDescriptorSubType;
	uint8_t	bUnitID;
	uint8_t	bSourceID;
	uint8_t	bCSourceInID;
	uint8_t	bCSourceOutID;
	uint8_t	iSRC;
}USBH_UAC_RU_Descriptor_Struct;
#endif

// Feature Unit	Descriptor
typedef	struct
{
	uint8_t	bLength;
	uint8_t	bDescriptorType;
	uint8_t	bDescriptorSubType;
	uint8_t	bUnitID;
	uint8_t	bSourceID;
	uint8_t	bControlSize;
	uint8_t	bmaControls[10][2];
	uint8_t	iFeature;
}USBH_UAC_FU_Descriptor_Struct;

// Selector Unit Descriptor 
typedef	struct
{
	uint8_t	bLength;
	uint8_t	bDescriptorType;
	uint8_t	bDescriptorSubType;
	uint8_t	bUnitID;
	uint8_t	bNrInPins;
	uint8_t	*baSourceID;
	uint8_t	iSelector;
}USBH_UAC_SU_Descripotr_Struct;

// Mixer Unit Descriptor
typedef	struct
{
	uint8_t	bLength;
	uint8_t	bDescriptorType;
	uint8_t	bDescriptorSubType;
	uint8_t	bUnitID;
	uint8_t	bNrInPins;
	uint8_t	*baSourceID;
	uint8_t	bNrChannels;
	uint8_t	bChannelConfigLowByte;		//wChannelConfig
	uint8_t	bChannelConfigHighByte;
	uint8_t	iChannelNames;
	uint8_t	*bmControls;
	uint8_t	iMixer;
}USBH_UAC_MU_Descriptor_Struct;

// Output Terminal Descriptor
typedef	struct
{
	uint8_t	bLength;
	uint8_t	bDescriptorType;
	uint8_t	bDescriptorSubType;
	uint8_t	bTerminalID;
	uint8_t	bTerminalTypeLowByte;		//wTerminalType
	uint8_t	bTerminalTypeHighByte;
	uint8_t	bAssocTerminal;
	uint8_t	bSourceID;
	uint8_t	iTerminal;
}USBH_UAC_OT_Descriptor_Struct;

// Input Terminal Descriptor
typedef	struct
{
	uint8_t	bLength;
	uint8_t	bDescriptorType;
	uint8_t	bDescriptorSubType;
	uint8_t	bTerminalID;
	uint8_t	bTerminalTypeLowByte;		//wTerminalType
	uint8_t	bTerminalTypeHighByte;
	uint8_t	bAssocTerminal;
	uint8_t	bNrChannels;
	uint8_t	bChannelConfigLowByte;		//wChannelConfig
	uint8_t	bChannelConfigHighByte;
	uint8_t	iChannelNames;
	uint8_t	iTerminal;
}USBH_UAC_IT_Descriptor_Struct;

#if UAC_2_0
// Clock Multiplier Descriptor
typedef	struct
{
	uint8_t	bLength;
	uint8_t	bDescriptorType;
	uint8_t	bDescriptorSubType;
	uint8_t	bClockID;
	uint8_t	bCSourceID;
	uint8_t	bmControls;
	uint8_t	iClockMultiplier;
}USBH_UAC_CLK_MULT_Descriptor_Struct;

// Clock Selector Descriptor
typedef	struct
{
	uint8_t	bLength;
	uint8_t	bDescriptorType;
	uint8_t	bDescriptorSubType;
	uint8_t	bClockID;
	uint8_t	bNrInPins;
	uint8_t	*baCSourceID;
	uint8_t	bmControls;
	uint8_t	iClockSelector;
}USBH_UAC_CLK_SEL_Descriptor_Struct;

// Clock Source	Descriptor
typedef	struct
{
	uint8_t	bLength;
	uint8_t	bDescriptorType;
	uint8_t	bDescriptorSubType;
	uint8_t	bClockID;
	uint8_t	bmAttributes;
	uint8_t	bmControls;
	uint8_t	bAssocTerminal;
	uint8_t	iClockSource;
}USBH_UAC_CLK_SRC_Descriptor_Struct;
#endif

// Class-Specific AC Interface Descriptor
typedef	struct
{
	uint8_t	bLength;
	uint8_t	bDescriptorType;
	uint8_t	bDescriptorSubType;
	uint8_t	bcdADC[2];			//bcdADC
	uint8_t	bTotalLengthLowByte;		//wTotalLength
	uint8_t	bTotalLengthHighByte;
	uint8_t	bInCollection;
}USBH_UAC_AC_Interface_Descriptor_Struct;

// Class-Specific AS Format Type Descriptor
typedef	struct
{
	uint8_t	bLength;
	uint8_t	bDescriptorType;
	uint8_t	bDescriptorSubType;
	uint8_t	bFormatType;
	uint8_t	bNrChannels;
	uint8_t	bSubFrameSize;
	uint8_t	bBitResolution;
	uint8_t	bSamFreqType;
	uint8_t	tSamFreq[20][3];
}USBH_UAC_AS_FMT_TYPE_Descriptor_Struct;
// Class-Specific AS Interface Descriptor
typedef	struct
{
	uint8_t	bLength;
	uint8_t	bDescriptorType;
	uint8_t	bDescriptorSubType;
	uint8_t	bTerminalLink;
	uint8_t	bDelay;
	uint8_t	bmFormats[2];
	
	USBH_UAC_AS_FMT_TYPE_Descriptor_Struct	AS_TFMD;
}USBH_UAC_AS_Interface_Descriptor_Struct;

#if 0
typedef	struct
{
	USBH_UAC_AC_Interface_Descriptor_Struct	AC_Interface;
#if UAC_2_0
	USBH_UAC_CLK_SRC_Descriptor_Struct	AC_CSD;
	USBH_UAC_CLK_SEL_Descriptor_Struct	AC_CXD;
	USBH_UAC_CLK_MULT_Descriptor_Struct	AC_CMD;
#endif
	USBH_UAC_IT_Descriptor_Struct		AC_ITD;
	USBH_UAC_OT_Descriptor_Struct		AC_OTD; 
	USBH_UAC_MU_Descriptor_Struct		AC_MUD;
	USBH_UAC_SU_Descripotr_Struct		AC_SUD;
	USBH_UAC_FU_Descriptor_Struct		AC_FUD;
#if UAC_2_0 
	USBH_UAC_RU_Descriptor_Struct		AC_RUD;
	USBH_UAC_EU_Descriptor_Struct		AC_EUD;
#endif	
	USBH_UAC_PU_Descriptor_Struct		AC_PUD;
	USBH_UAC_XU_Descriptor_Struct		AC_XUD;
}USBH_UAC_DESCRIPTOR_Structure;
#endif
typedef	struct
{   
//======================== VIDEO DESCRIPTOR STRUCTURE =======================/	
	USBH_UVC_CS_Interface_Descriptor_Struct		VC_Interface;
	USBH_UVC_Camera_IT_Descriptor_Struct		VC_Camera_IT_Des;
	USBH_UVC_Media_Transport_IT_Descriptor_Struct	VC_Media_IT_Des;
	USBH_UVC_OT_Descriptor_Struct			VC_OT_Des[MAX_DES_NUM];
	USBH_UVC_SU_Descriptor_Struct			VC_SU_Des;
	USBH_UVC_PU_Descriptor_Struct			VC_PU_Des;
	USBH_UVC_XU_Descriptor_Struct			VC_XU_Des[MAX_DES_NUM];
	USBH_UVC_CS_Header_Descriptor_Struct		VS_Interface[UVC_INTERFACE_NUM_MAX];
//======================== AUDIO DESCRIPTOR STRUCTURE =======================/	
	USBH_UAC_AC_Interface_Descriptor_Struct		AC_Interface;
#if UAC_2_0
	USBH_UAC_CLK_SRC_Descriptor_Struct		AC_CSD;
	USBH_UAC_CLK_SEL_Descriptor_Struct		AC_CXD;
	USBH_UAC_CLK_MULT_Descriptor_Struct		AC_CMD;
#endif
	USBH_UAC_IT_Descriptor_Struct			AC_ITD[MAX_DES_NUM];
	USBH_UAC_OT_Descriptor_Struct			AC_OTD[MAX_DES_NUM];	
	USBH_UAC_MU_Descriptor_Struct			AC_MUD;
	USBH_UAC_SU_Descripotr_Struct			AC_SUD[MAX_DES_NUM];
	USBH_UAC_FU_Descriptor_Struct			AC_FUD;
#if UAC_2_0 
	USBH_UAC_RU_Descriptor_Struct			AC_RUD;
	USBH_UAC_EU_Descriptor_Struct			AC_EUD;
#endif	
	USBH_UAC_PU_Descriptor_Struct			AC_PUD;
	USBH_UAC_XU_Descriptor_Struct			AC_XUD; 
	USBH_UAC_AS_Interface_Descriptor_Struct		AS_Interface[UVC_INTERFACE_NUM_MAX];
}USBH_UVC_DESCRIPTOR_Structure;

typedef	struct
{
	uint8_t	bInftIndex;
	uint8_t	bFmtIndex;
	uint8_t	bResIndex;
	uint8_t	bFpsIndex;
}USBH_UVC_COMMIT_INFO;

// Video Probe and Commit Controls
typedef	struct
{
	uint8_t	bmHint[2];
	uint8_t	bFormatIndex;
	uint8_t	bFrameIndex;
	uint8_t	bFrameInterval[4];		//dwFrameInterval
	uint8_t	bKeyFrameRate[2];		//wKeyFrameRate
	uint8_t	bPFrameRate[2];			//wPFrameRate
	uint8_t	bCompQuality[2];		//wCompQuality
	uint8_t	bCompWindowSize[2];		//wCompWindowSize
	uint8_t	bDelay[2];			//wDelay
	uint8_t	bMaxVideoFrameSize[4];		//dwMaxVideoFrameSize
	uint8_t	bMaxPayloadTransferSize[4]; 	//dwMaxPayloadTransferSize
	uint8_t	bClockFrequency[4];		//dwClockFrequency
	uint8_t	bmFramingInfo;
	uint8_t	bPreferedVersion;
	uint8_t	bMinVersion;
	uint8_t	bMaxVersion;
	uint8_t	bIntfNum;
	uint8_t	bTgtFmt;
	uint8_t	bTgtRes;
	uint32_t	dwTgtFps;
	uint32_t	dwStreamID;

}USBH_UVC_STREAMING_CONTROL;

typedef	struct
{  
	uint8_t				SM;
	uint8_t				STS_CHANGE;
	uint8_t				CurIntfNum;	
	uint8_t				UVC_IntfIndex;
	uint8_t				UAC_IntfIndex;
	uint8_t				MaxStreamCnt;
	uint8_t				CurStreamCnt;
	USBH_UVC_DESCRIPTOR_Structure   DES;
	USBH_UVC_STREAMING_CONTROL		PROBE;
	USBH_UVC_COMMIT_INFO		COMMIT;
}USBH_UVC_CLASS_Structure;


#define	SC_UNDEFINED			0x00
#define	SC_VIDEOCONTROL			0x01
#define	SC_VIDEOSTREAMING		0x02
#define	SC_VIDEO_INTERFACE_COLLECTION	0x03

#define	PC_PROTOCOL_UNDEFINED		0x00

/* Video Class-Specific	Descriptor Types */
#define	CS_UNDEFINED			0x20
#define	CS_DEVICE			0x21
#define	CS_CONFIGURATION		0x22
#define	CS_STRING			0x23
#define	CS_INTERFACE			0x24
#define	CS_ENDPOINT			0x25

/* VideoControl	class specific interface descriptor */
#define	VC_DESCRIPTOR_UNDEFINED		0x00
#define	VC_HEADER			0x01
#define	VC_INPUT_TERMINAL		0x02
#define	VC_OUTPUT_TERMINAL		0x03
#define	VC_SELECTOR_UNIT		0x04
#define	VC_PROCESSING_UNIT		0x05
#define	VC_EXTENSION_UNIT		0x06

/* VideoStreaming class	specific interface descriptor */
#define	VS_UNDEFINED			0x00
#define	VS_INPUT_HEADER			0x01
#define	VS_OUTPUT_HEADER		0x02
#define	VS_STILL_IMAGE_FRAME		0x03
#define	VS_FORMAT_UNCOMPRESSED		0x04
#define	VS_FRAME_UNCOMPRESSED		0x05
#define	VS_FORMAT_MJPEG			0x06
#define	VS_FRAME_MJPEG			0x07
#define	VS_FORMAT_MPEG2TS		0x0a
#define	VS_FORMAT_DV			0x0c
#define	VS_COLORFORMAT			0x0d
#define	VS_FORMAT_FRAME_BASED		0x10
#define	VS_FRAME_FRAME_BASED		0x11
#define	VS_FORMAT_STREAM_BASED		0x12

/* Endpoint type */
#define	EP_UNDEFINED			0x00
#define	EP_GENERAL			0x01
#define	EP_ENDPOINT			0x02
#define	EP_INTERRUPT			0x03

/* Request codes */
#define	RC_UNDEFINED			0x00
#define	SET_CUR				0x01
#define	GET_CUR				0x81
#define	GET_MIN				0x82
#define	GET_MAX				0x83
#define	GET_RES				0x84
#define	GET_LEN				0x85
#define	GET_INFO			0x86
#define	GET_DEF				0x87

/* VideoControl	interface controls */
#define	VC_CONTROL_UNDEFINED		0x00
#define	VC_VIDEO_POWER_MODE_CONTROL	0x01
#define	VC_REQUEST_ERROR_CODE_CONTROL	0x02

/* Terminal controls */
#define	TE_CONTROL_UNDEFINED		0x00

/* Selector Unit controls */
#define	SU_CONTROL_UNDEFINED		0x00
#define	SU_INPUT_SELECT_CONTROL		0x01

/* Camera Terminal controls */
#define	CT_CONTROL_UNDEFINED			0x00
#define	CT_SCANNING_MODE_CONTROL		0x01
#define	CT_AE_MODE_CONTROL			0x02
#define	CT_AE_PRIORITY_CONTROL			0x03
#define	CT_EXPOSURE_TIME_ABSOLUTE_CONTROL	0x04
#define	CT_EXPOSURE_TIME_RELATIVE_CONTROL	0x05
#define	CT_FOCUS_ABSOLUTE_CONTROL		0x06
#define	CT_FOCUS_RELATIVE_CONTROL		0x07
#define	CT_FOCUS_AUTO_CONTROL			0x08
#define	CT_IRIS_ABSOLUTE_CONTROL		0x09
#define	CT_IRIS_RELATIVE_CONTROL		0x0a
#define	CT_ZOOM_ABSOLUTE_CONTROL		0x0b
#define	CT_ZOOM_RELATIVE_CONTROL		0x0c
#define	CT_PANTILT_ABSOLUTE_CONTROL		0x0d
#define	CT_PANTILT_RELATIVE_CONTROL		0x0e
#define	CT_ROLL_ABSOLUTE_CONTROL		0x0f
#define	CT_ROLL_RELATIVE_CONTROL		0x10
#define	CT_PRIVACY_CONTROL			0x11

/* Processing Unit controls */
#define	PU_CONTROL_UNDEFINED				0x00
#define	PU_BACKLIGHT_COMPENSATION_CONTROL		0x01
#define	PU_BRIGHTNESS_CONTROL				0x02
#define	PU_CONTRAST_CONTROL				0x03
#define	PU_GAIN_CONTROL					0x04
#define	PU_POWER_LINE_FREQUENCY_CONTROL			0x05
#define	PU_HUE_CONTROL					0x06
#define	PU_SATURATION_CONTROL				0x07
#define	PU_SHARPNESS_CONTROL				0x08
#define	PU_GAMMA_CONTROL				0x09
#define	PU_WHITE_BALANCE_TEMPERATURE_CONTROL		0x0a
#define	PU_WHITE_BALANCE_TEMPERATURE_AUTO_CONTROL	0x0b
#define	PU_WHITE_BALANCE_COMPONENT_CONTROL		0x0c
#define	PU_WHITE_BALANCE_COMPONENT_AUTO_CONTROL		0x0d
#define	PU_DIGITAL_MULTIPLIER_CONTROL			0x0e
#define	PU_DIGITAL_MULTIPLIER_LIMIT_CONTROL		0x0f
#define	PU_HUE_AUTO_CONTROL				0x10
#define	PU_ANALOG_VIDEO_STANDARD_CONTROL		0x11
#define	PU_ANALOG_LOCK_STATUS_CONTROL			0x12

#define	LXU_MOTOR_PANTILT_RELATIVE_CONTROL		0x01
#define	LXU_MOTOR_PANTILT_RESET_CONTROL			0x02
#define	LXU_MOTOR_FOCUS_MOTOR_CONTROL			0x03

/* VideoStreaming interface controls */
#define	VS_CONTROL_UNDEFINED		0x00
#define	VS_PROBE_CONTROL		0x01
#define	VS_COMMIT_CONTROL		0x02
#define	VS_STILL_PROBE_CONTROL		0x03
#define	VS_STILL_COMMIT_CONTROL		0x04
#define	VS_STILL_IMAGE_TRIGGER_CONTROL	0x05
#define	VS_STREAM_ERROR_CODE_CONTROL	0x06
#define	VS_GENERATE_KEY_FRAME_CONTROL	0x07
#define	VS_UPDATE_FRAME_SEGMENT_CONTROL	0x08
#define	VS_SYNC_DELAY_CONTROL		0x09

#define	TT_VENDOR_SPECIFIC		0x0100
#define	TT_STREAMING			0x0101

/* Input Terminal types	*/
#define	ITT_VENDOR_SPECIFIC		0x0200
#define	ITT_CAMERA			0x0201
#define	ITT_MEDIA_TRANSPORT_INPUT	0x0202

/* Output Terminal types */
#define	OTT_VENDOR_SPECIFIC		0x0300
#define	OTT_DISPLAY			0x0301
#define	OTT_MEDIA_TRANSPORT_OUTPUT	0x0302

/* External Terminal types */
#define	EXTERNAL_VENDOR_SPECIFIC	0x0400
#define	COMPOSITE_CONNECTOR		0x0401
#define	SVIDEO_CONNECTOR		0x0402
#define	COMPONENT_CONNECTOR		0x0403

#define	UVC_TERM_INPUT			0x0000
#define	UVC_TERM_OUTPUT			0x8000

#define	UVC_ENTITY_TYPE(entity)		((entity)->type	& 0x7fff)
#define	UVC_ENTITY_IS_UNIT(entity)	(((entity)->type & 0xff00) == 0)
#define	UVC_ENTITY_IS_TERM(entity)	(((entity)->type & 0xff00) != 0)
#define	UVC_ENTITY_IS_ITERM(entity) \
	(((entity)->type & 0x8000) == UVC_TERM_INPUT)
#define	UVC_ENTITY_IS_OTERM(entity) \
	(((entity)->type & 0x8000) == UVC_TERM_OUTPUT)

#define	UVC_STATUS_TYPE_CONTROL		1
#define	UVC_STATUS_TYPE_STREAMING	2

//======================== AUDIO DEFINITION =======================//
/* Audio Interface Subclass Codes */
#define	SC_AUDIOCONTROL			0x01
#define	SC_AUDIOSTREAMING		0x02
#define	SC_MIDISTREAMING		0x03

/* AudioControl	class specific interface descriptor */
#define	AC_DESCRIPTOR_UNDEFINED		0x00
#define	AC_HEADER			0x01
#define	AC_INPUT_TERMINAL		0x02
#define	AC_OUTPUT_TERMINAL		0x03
#define	AC_MIXER_UNIT			0x04
#define	AC_SELECTOR_UNIT		0x05
#define	AC_FEATURE_UNIT			0x06
#define	AC_PROCESSING_UNIT		0x07
#define	AC_EXTENSION_UNIT		0x08

/* Audio Class-Specific	AS Interface Descriptor	Subtypes */
#define	AS_DESCRIPTOR_UNDEFINED		0x00
#define	AS_GENERAL			0x01
#define	AS_FORMAT_TYPE			0x02
#define	AS_FORMAT_SPECIFIC		0x03

/* Audio Processing Unit Process Types */
#define	PU_UP_DOWNMIX_PROCESS		0x01
#define	PU_DOLBY_PROLOGIC_PROCESS	0x02
#define	PU_3D_STEREO_EXTENDER_PROCESS	0x03
#define	PU_REVERBERATION_PROCESS	0x04
#define	PU_CHORUS_PROCESS		0x05
#define	PU_DYN_RANGE_COMP_PROCESS	0x06

/* Audio Terminal Control Selectors */
#define	TC_TE_CONTROL_UNDEFINED		0x00
#define	TC_COPY_PROTECT_CONTROL		0x01

/* Audio Feature Unit Control Selectors	*/
#define	FU_CONTROL_UNDEFINED		0x00
#define	FU_MUTE_CONTROL			0x01
#define	FU_VOLUME_CONTROL		0x02
#define	FU_BASS_CONTROL			0x03
#define	FU_MID_CONTROL			0x04
#define	FU_TREBLE_CONTROL		0x05
#define	FU_GRAPHIC_EQUALIZER_CONTROL	0x06
#define	FU_AUTOMATIC_GAIN_CONTROL	0x07
#define	FU_DELAY_CONTROL		0x08
#define	FU_BASS_BOOST_CONTROL		0x09
#define	FU_LOUDNESS_CONTROL		0x0A

/* Audio Processing Unit Control Selectors */
/*   Up/Down-mix */
#define	PU_UD_CONTROL_UNDEFINED		0x00
#define	PU_UD_ENABLE_CONTROL		0x01
#define	PU_UD_MODE_SELECT_CONTROL	0x02

/*   Dolby Prologic */
#define	PU_DP_CONTROL_UNDEFINED		0x00
#define	PU_DP_ENABLE_CONTROL		0x01
#define	PU_DP_MODE_SELECT_CONTROL	0x02

/*   3D	Stereo Extender	*/
#define	PU_3D_CONTROL_UNDEFINED		0x00
#define	PU_3D_ENABLE_CONTROL		0x01
#define	PU_SPACIOUSNESS_CONTROL		0x02

/*   Reverberation */
#define	PU_RV_CONTROL_UNDEFINED		0x00
#define	PU_RV_ENABLE_CONTROL		0x01
#define	PU_REVERB_LEVEL_CONTROL		0x02
#define	PU_REVERB_TIME_CONTROL		0x03
#define	PU_REVERB_FEEDBACK_CONTROL	0x04

/*   Chorus */
#define	PU_CH_CONTROL_UNDEFINED		0x00
#define	PU_CH_ENABLE_CONTROL		0x01
#define	PU_CHORUS_LEVEL_CONTROL		0x02
#define	PU_CHORUS_RATE_CONTROL		0x03
#define	PU_CHORUS_DEPTH_CONTROL		0x04

/*   Dynamic Range Compressor */
#define	PU_DR_CONTROL_UNDEFINED		0x00
#define	PU_DR_ENABLE_CONTROL		0x01
#define	PU_COMPRESSION_RATE_CONTROL	0x02
#define	PU_MAXAMPL_CONTROL		0x03
#define	PU_THRESHOLD_CONTROL		0x04
#define	PU_ATTACK_TIME			0x05
#define	PU_RELEASE_TIME			0x06

/* Audio Extension Unit	Control	Selectors */
#define	XU_CONTROL_UNDEFINED		0x00
#define	XU_ENABLE_CONTROL		0x01


// bmHeaderInfo
#define	FrameID			0x01
#define	EndofFrame		0x02
#define	PresentationTime	0x04
#define	SourceClockRef		0x08 
#define	StillImage		0x20
#define	Error			0x40
#define	Endofheader		0x80


//======================== AUDIO DESCRIPTOR STRUCTURE DEFINITION =======================//

//extern void uvc_give_stream_data(EHCI_ENDP_STRUCT *EP);

extern void uvc_task(void * pvParameters);
extern void uvc_app_task(void *	pvParameters);
extern void uvc_task_init(uint8_t dev_id);
extern void uvc_task_uninit(uint8_t dev_id);

// UVC API
extern uint32_t	uvc_start(USBH_Device_Structure	*UVC_DEV, uint8_t bFmt,	uint8_t	bRes, uint32_t dwFps,uint32_t *ptr,uint32_t size); // erick recovery
extern uint8_t uvc_stop(USBH_Device_Structure *UVC_DEV,uint32_t	stream);

// UVC FUNCTION
extern uint32_t	uvc_get_info(void);

extern uint8_t uvc_set_res_val(uint16_t	wWidth,	uint16_t wHeight);
extern uint32_t	uvc_get_streamid(void);
extern void uvc_unregister_streamid(USBH_Device_Structure *UVC_DEV, uint32_t dwStreamID);
extern uint8_t uvc_streamid_to_intfnum(USBH_Device_Structure *UVC_DEV, uint32_t	streamID);
extern EHCI_ENDP_STRUCT * uvc_streamid_to_ep(uint32_t streamID);
extern uint8_t uvc_streamid_to_devid(uint32_t streamID);
extern uint8_t uvc_set_streamid(USBH_Device_Structure *UVC_DEV);
extern	QueueHandle_t	USBH_QUEUE_STREAM_DATA;
extern void uvc_stream_complete(	USBH_UVC_STREAM_Structure	*uvc_stream);


// UVC PRINT INFORMATION
extern void uvc_print_descriptor_info(USBH_Device_Structure *UVC_DEV);
extern void uvc_print_cs_intf_info(USBH_UVC_DESCRIPTOR_Structure	*UVC_DES);
extern void uvc_print_ot_info(USBH_UVC_DESCRIPTOR_Structure *UVC_DES);
extern void uvc_print_xu_info(USBH_UVC_DESCRIPTOR_Structure *UVC_DES);
extern void uvc_print_iit_camera_info(USBH_UVC_DESCRIPTOR_Structure *UVC_DES);
extern void uvc_print_pu_info(USBH_UVC_DESCRIPTOR_Structure *UVC_DES);
extern void uvc_print_yuv_info(USBH_UVC_YUV_Format_Descriptor_Struct	*YUV_FORMAT_Des);
extern void uvc_print_fb_info(USBH_UVC_Frame_Based_Format_Descriptor_Struct *FB_FORMAT_Des);
extern void uvc_print_vs_info(USBH_UVC_CS_Format_Descriptor_Struct  *VS_FORMAT_Des);
extern void uvc_print_vs_intf_info(USBH_UVC_DESCRIPTOR_Structure	*UVC_DES);
extern void uac_print_ac_intf_info(USBH_UVC_DESCRIPTOR_Structure	*UVC_DES);
extern void uac_print_fu_info(USBH_UVC_DESCRIPTOR_Structure   *UVC_DES);
extern void uac_print_su_info(USBH_UVC_DESCRIPTOR_Structure   *UVC_DES);
extern void uac_print_mu_info(USBH_UVC_DESCRIPTOR_Structure   *UVC_DES);
extern void uac_print_ot_info(USBH_UVC_DESCRIPTOR_Structure   *UVC_DES);
extern void uac_print_it_info(USBH_UVC_DESCRIPTOR_Structure *UVC_DES);
extern void uac_print_as_intf_info(USBH_UVC_DESCRIPTOR_Structure   *UVC_DES);
extern void uac_print_as_fmt_type_info(USBH_UAC_AS_FMT_TYPE_Descriptor_Struct  *AS_TFMD);

extern uint32_t	uvc_get_time(uint32_t prt);
extern uint8_t uvc_get_iframe(uint32_t ptr);


extern void uvc_print_info_data(USBH_UVC_INFO_Struct *UVC_Info);
extern xTaskHandle	xTASK_HDL_UVC_DRV[USBH_MAX_PORT];
extern TaskHandle_t	xTASK_HDL_UVC_APP;


// UVC data queue
extern USBH_UVC_DATA_QUEUE_Structure	UVC_DATA;
#if defined(	CONFIG_SNX_ISO_ACCELERATOR )
extern void uvc_process_data(EHCI_ENDP_STRUCT *EP);
#else
extern void uvc_process_data(EHCI_ENDP_STRUCT *EP);
#endif



#endif //UVC__H

