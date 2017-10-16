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
 * this	is EHCIHCD file, include this file before use
 * @author IP2 Dept Sonix. (Hammer Huang #1359)
 */
#ifndef	USB_HCD__H
#define	USB_HCD__H  

#include <generated/snx_sdk_conf.h> 

/** \defgroup USBH_IP_ADDR_DEF IP address definition
* \ingroup USBH_MODULE
* 
* @{
*/ 
// EHCI	Pre-Defination 
#if defined( CONFIG_PLATFORM_SN98660 )
#define	EHCI_REG_BASE_ADDRESS			0x90800000		/**< IP Physical Address */
#endif

#if defined( CONFIG_PLATFORM_SN58510 )
#if defined ( CONFIG_SN_KEIL_SDK ) && defined (EHCI_REG_1)
	#define	EHCI_REG_BASE_ADDRESS		EHCI_1_BASE 		/**< IP Physical Address */
#elif defined ( CONFIG_SN_KEIL_SDK ) && defined (EHCI_REG_2)
	#define	EHCI_REG_BASE_ADDRESS		EHCI_2_BASE 		/**< IP Physical Address */
#else
	#define	EHCI_REG_BASE_ADDRESS		0x94200000		/**< IP Physical Address */
//	#define	EHCI_REG_BASE_ADDRESS		0x90800000		/**< IP Physical Address */

#endif
#endif

#if defined ( CONFIG_PLATFORM_SN7300 )
	#include "SNC7300.h"
	#define SemaphoreHandle_t	xSemaphoreHandle
	#define QueueHandle_t	    	xQueueHandle
	#define TaskHandle_t	    	xTaskHandle
	#define EHCI_REG_BASE_ADDRESS		0x42000000  
#endif 

#if defined ( CONFIG_PLATFORM_XILINX_ZYNQ_7000 )
#define	EHCI_REG_BASE_ADDRESS			0xE0002000 		/**< IP Physical Address */
#endif

/** @} */




// Sonix ISO 
#define	Sonix_ISO_Accelerator_Enable	1
#define	MaxFrameSize			600*1024
#define	ThersholdPosition		3

// Standard ISO
#define	STREAM_PIPE_MAX			1
#define	Standard_iTD_EP_Max_Count	128
#define	Standard_iTD_interval		32
#define	Max_STD_ISO_FrameSize		200*1024






/** \defgroup CAPACITY_DEF Max capacity	definition
* \ingroup USBH_MODULE
* 
* @{
*/ 
#define	MAX_QH_PER_DEVICE			5			/**< MAX QH	Per Device (5*ENDP/Device) */
#define	MAX_QTD_PER_QH				22			/**< MAX QTD Per QH	(10*20KB/ENDP) */
/** @} */

// EHCI	QH/QTD Manage 
#define	EHCI_QH_SIZE				64
#define	EHCI_QTD_SIZE				32
#define	EHCI_ITD_SIZE				64
#define	EHCI_SXITD_SIZE				64



// EHCI	QH/QTD Manage 
#define	EHCI_QH_MAX				16
#define	EHCI_QTD_MAX				128
#define	EHCI_ITD_MAX				128
#define	EHCI_SXITD_MAX				4



// EHCI	PERIODIC TABLE	Manage
#define	EHCI_PERIODIC_TABLE_MAX			10

// EHCI	Frame_list size	
#define	Host20_Preiodic_Frame_List_MAX		256	
#define	Host20_Preiodic_Frame_List_MASK		(Host20_Preiodic_Frame_List_MAX-1)

// EHCI	MEM Manager 
#define	EHCI_MEM_FREE	0
#define	EHCI_MEM_USED	1

#define	UINT32MAX				0xFFFFFFFF
#define	QH_DW0_T_MIN				0
#define	QH_DW0_T_MASK				0x00000001
#define	QH_DW0_Typ_MIN				1
#define	QH_DW0_Typ_MASK				0x00000006
#define	QH_DW1_DeviceAddr_MIN			0
#define	QH_DW1_DeviceAddr_MASK			0x0000007F
#define	QH_DW1_ENDPT_MIN			8
#define	QH_DW1_ENDPT_MASK			0x00000F00
#define	QH_DW1_EPS_MIN				12
#define	QH_DW1_EPS_MASK				0x00003000
#define	QH_DW1_Hbit_MIN				15
#define	QH_DW1_Hbit_MASK			0x00008000
#define	QH_DW1_RL_MIN				28
#define	QH_DW1_RL_MASK				0xF0000000
#define	QH_DW1_MaxPacketLength_MIN		16
#define	QH_DW1_MaxPacketLength_MASK		0x07FF0000
#define	QH_DW1_C_MIN				27
#define	QH_DW1_C_MASK				0x08000000
#define	QH_DW1_DEVICE_ADDRESS_MIN		0
#define	QH_DW1_DEVICE_ADDRESS_MASK		0x0000007F
#define	QH_DW2_SMASK_MIN			0
#define	QH_DW2_SMASK_MASK			0x000000FF
#define	QH_DW2_CMASK_MIN			8
#define	QH_DW2_CMASK_MASK			0x0000FF00
#define	QH_DW2_PORTNUM_MIN			23
#define	QH_DW2_PORTNUM_MASK			0x3F800000
#define	QH_DW2_HUBADDR_MIN			16
#define	QH_DW2_HUBADDR_MASK			0x007F0000
#define	QH_DW4_T_MIN				0
#define	QH_DW4_T_MASK				0x00000001
#define	QH_DW4_Next_qTD_Ptr_MIN			5
#define	QH_DW4_Next_qTD_Ptr_MASK		0xFFFFFFE0
#define	QH_DW6_STS_SplitXState_MIN		1
#define	QH_DW6_STS_SplitXState_MASK		0x00000002
#define	QH_DW6_STS_Halted_MIN			6
#define	QH_DW6_STS_Halted_MASK			0x00000040
#define	QH_DW6_STS_Active_MIN			7
#define	QH_DW6_STS_Active_MASK			0x00000080
#define	QH_DW6_IOC_MIN				15
#define	QH_DW6_IOC_MASK				0x00008000
#define	QH_DW6_TOTAL_BYTES_TO_TRANSFER_MIN	16
#define	QH_DW6_TOTAL_BYTES_TO_TRANSFER_MASK	0x7FFF0000
#define	QH_DW6_dt_MIN				31
#define	QH_DW6_dt_MASK				0x80000000

//>>>>>>> QTD DEFINE
#define	QTD_DW0_Next_qTD_Ptr_MIN		5
#define	QTD_DW0_Next_qTD_Ptr_MASK		0xFFFFFFE0
#define	QTD_DW2_STS_Halted_MIN			6
#define	QTD_DW2_STS_Halted_MASK			0x00000040
#define	QTD_DW2_STS_Active_MIN			7
#define	QTD_DW2_STS_Active_MASK			0x00000080
#define	QTD_DW2_TOTAL_BYTES_To_TRANSFER_MIN	16
#define	QTD_DW2_TOTAL_BYTES_To_TRANSFER_MASK	0x7FFF0000


#define	ehci_clear(STR,DESCRIPTION)			(STR&((UINT32MAX)&(~(DESCRIPTION##_MASK))))
#define	ehci_insert(STR,DESCRIPTION,VALUE)		(CLEARSTR(STR,DESCRIPTION)|(VALUE<<	DESCRIPTION##_MIN ))
#define	ehci_get_value(STR,DESCRIPTION)			((STR&(DESCRIPTION##_MASK))>>DESCRIPTION##_MIN )
#define	ehci_update_value(STR,DESCRIPTION,VALUE)	(STR = INSERTSTR(STR,DESCRIPTION,VALUE))

typedef	enum
{
	EHCI_PARSER_ROOT_DET,
	EHCI_PARSER_ROOT_RST,
	EHCI_PARSER_CX_ENQU,
	EHCI_PARSER_NONCX_ENQU,
	EHCI_PARSER_WAKE_CLASS_DRV,
	EHCI_PARSER_ROOT_DEQ,
	EHCI_PARSER_ROOT_HUB_DEQ,
	EHCI_PARSER_PORT_HUB_DEQ,

}EHCI_PARSER_INDEX; 


typedef	enum
{
	EHCI_NO_CHK_ADV,
	EHCI_CHK_ADV

}EHCI_CHK_ADV_INDEX;

typedef	enum
{
	EHCI_MEM_TYPE_QH,
	EHCI_MEM_TYPE_QTD,
	EHCI_MEM_TYPE_ITD,
	EHCI_MEM_TYPE_SXITD
	
}EHCI_GET_STRUCTURE_INDEX;

typedef	enum
{
	EHCI_NEXT_TYPE_ITD,
	EHCI_NEXT_TYPE_QH,
	EHCI_NEXT_TYPE_SITD,
	EHCI_NEXT_TYPE_FSTN,
	
}EHCI_NEXT_TYPE_STRUCTURE_INDEX;

typedef	enum
{
	EHCI_OUT_TOKEN, 
	EHCI_IN_TOKEN,
	EHCI_SETUP_TOKEN,
	EHCI_RESERVE
}EHCI_PID_INDEX;

typedef	enum
{
	EHCI_ITD_TYPE,  
	EHCI_QH_TYPE,
	EHCI_SITD_TYPE,
	EHCI_FSTN_TYPE
}EHCI_TYPE_FIELD_INDEX;



// EHCI	Controller Register Structure 
typedef	struct{

	uint32_t CAPLENGTH		: 8;	// [07:00]	[0x10]
	uint32_t RESERVED0		: 8;	// [15:08]	[0x00]
	uint32_t HCIVERSION		: 16;	// [31:16]	[0x96]
	
}EHCI_VER_CPLGTH;

typedef	struct{

	uint32_t N_PORTS		: 4;	// [03:00]	[0x01]
	uint32_t PPC			: 1;	// [04]		[0x01]
	uint32_t RESERVED0		: 2;	// [06:05]	[0x00]
	uint32_t PRT_RT_RL		: 1;	// [7]		[0x00]
	uint32_t N_PCC			: 4;	// [11:08]	[0x01]
	uint32_t N_CC			: 4;	// [15:12]	[0x00]
	uint32_t P_INDICATOR		: 1;	// [16]		[0x01]
	uint32_t RESERVED1		: 15;	// [31:17]	[0x00]
}EHCI_HCSPARAMS;

typedef	struct{

	uint32_t RESERVED0		: 1;	// [00]		[0x00]
	uint32_t FRAME_FLAG		: 1;	// [01]		[0x01]
	uint32_t PARK_CAP		: 1;	// [02]		[0x01]
	uint32_t RESERVED1		: 29;	// [31:03]	[0x00]
}EHCI_HCCPARAMS;

typedef	struct{

	uint32_t RUN_NSTOP		: 1;	// [00]		[0x00]
	uint32_t HCRESET		: 1;	// [01]		[0x00]
	uint32_t FRAME_CTR		: 2;	// [03:02]	[0x00]
	uint32_t PERIODIC_EN		: 1;	// [04]		[0x00]
	uint32_t ASYNC_EN		: 1;	// [05]		[0x00]
	uint32_t DOORBELL_EN		: 1;	// [06]		[0x00]
	uint32_t RESERVED0		: 1;	// [07]		[0x00]
	uint32_t PARK_CNT		: 2;	// [09:08]	[0x03]
	uint32_t RESERVED1		: 1;	// [10]		[0x00]
	uint32_t PARK_EN		: 1;	// [11]		[0x01]
	uint32_t RESERVED2		: 4;	// [15:12]	[0x00]
	uint32_t INT_CTR		: 8;	// [23:16]	[0x08]
	uint32_t RESERVED3		: 8;	// [31:24]	[0x00]
}EHCI_USBCMD;

#if defined( CONFIG_PLATFORM_SN98660 ) || defined( CONFIG_PLATFORM_SN7300 ) || defined( CONFIG_PLATFORM_XILINX_ZYNQ_7000 )
typedef	struct{

	uint32_t USB_INT		: 1;	// [00]		[0x00]
	uint32_t USB_ERR_INT		: 1;	// [01]		[0x00]
	uint32_t PO_CHG_INT		: 1;	// [02]		[0x00]
	uint32_t ROLLOVER_INT		: 1;	// [03]		[0x00]
	uint32_t SYS_ERR_INT		: 1;	// [04]		[0x00]
	uint32_t ASYNC_ADVANCE_INT	: 1;	// [05]		[0x00]
	uint32_t RESERVED0		: 6;	// [11:06]	[0x00]
	uint32_t HC_HALTED		: 1;	// [12]		[0x01]
	uint32_t RECLAMATION		: 1;	// [13]		[0x00]
	uint32_t PERIODIC_STS		: 1;	// [14]		[0x00]
	uint32_t ASYNC_STS		: 1;	// [15]		[0x00]
	uint32_t RESERVED1		: 16;	// [31:16]	[0x00]
}EHCI_USBSTS;
#endif 


#if defined( CONFIG_PLATFORM_SN58510 )

typedef	struct{

	uint32_t USB_INT		: 1;	// [00]		[0x00]
	uint32_t USB_ERR_INT		: 1;	// [01]		[0x00]
	uint32_t PO_CHG_INT		: 1;	// [02]		[0x00]
	uint32_t ROLLOVER_INT		: 1;	// [03]		[0x00]
	uint32_t SYS_ERR_INT		: 1;	// [04]		[0x00]
	uint32_t ASYNC_ADVANCE_INT	: 1;	// [05]		[0x00]
	uint32_t RESERVED0		: 6;	// [11:06]	[0x00]
	uint32_t HC_HALTED		: 1;	// [12]		[0x01]
	uint32_t RECLAMATION		: 1;	// [13]		[0x00]
	uint32_t PERIODIC_STS		: 1;	// [14]		[0x00]
	uint32_t ASYNC_STS		: 1;	// [15]		[0x00]
	uint32_t RESERVED1		: 15;	// [30:16]	[0x00]
	uint32_t SOF_INT		: 1;	// [31]		[0x00]
}EHCI_USBSTS;
#endif


#if defined( CONFIG_PLATFORM_SN98660 ) || defined( CONFIG_PLATFORM_SN7300 ) || defined( CONFIG_PLATFORM_XILINX_ZYNQ_7000 )
typedef	struct{

	uint32_t USB_INT_EN		: 1;	// [00]		[0x00]
	uint32_t USB_ERR_INT_EN		: 1;	// [01]		[0x00]
	uint32_t PO_CHG_INT_EN		: 1;	// [02]		[0x00]
	uint32_t ROLLOVER_INT_EN	: 1;	// [03]		[0x00]
	uint32_t SYS_ERR_INT_EN		: 1;	// [04]		[0x00]
	uint32_t ASYNC_ADVANCE_INT_EN	: 1;	// [05]		[0x00]
	uint32_t RESERVED0		: 26;	// [31:06]	[0x00]
}EHCI_USBINTR;
#endif


#if defined( CONFIG_PLATFORM_SN58510 )

typedef	struct{

	uint32_t USB_INT_EN		: 1;	// [00]		[0x00]
	uint32_t USB_ERR_INT_EN		: 1;	// [01]		[0x00]
	uint32_t PO_CHG_INT_EN		: 1;	// [02]		[0x00]
	uint32_t ROLLOVER_INT_EN	: 1;	// [03]		[0x00]
	uint32_t SYS_ERR_INT_EN		: 1;	// [04]		[0x00]
	uint32_t ASYNC_ADVANCE_INT_EN   : 1;	// [05]		[0x00]
	uint32_t RESERVED0		: 25;   // [30:06]	[0x00]
	uint32_t SOF_INT_EN		: 1;	// [31]		[0x00]	
}EHCI_USBINTR;
#endif 

typedef	struct{

	uint32_t FRINDEX		: 14;	// [13:00]	[0x00]
	uint32_t RESERVED0		: 18;	// [31:14]	 [0x00]
}EHCI_FRINDEX;

typedef	struct{

	uint32_t RESERVED0		: 12;	// [11:00]	[0x00]
	uint32_t BASE_ADDR		: 20;	// [31:12]	[0x00]
}EHCI_PERIODICLISTBASE;

typedef	struct{

	uint32_t RESERVED0		: 5;	// [04:00]	[0x00]
	uint32_t LPL			: 27;	// [31:05]	[0x00]
}EHCI_ASYNCLISTADDR;

typedef	struct{

	uint32_t CURR_CNNT_ST		: 1;	// [00]		[0x00]
	uint32_t CNNT_ST_CHG		: 1;	// [01]		[0x00]
	uint32_t PORTENABLED		: 1;	// [02]		[0x00]
	uint32_t PO_DIS_CHG		: 1;	// [03]		[0x00]
	uint32_t OVER_CURR_ACT		: 1;	// [04]		[0x00]
	uint32_t OVER_CURR_CHG		: 1;	// [05]		[0x00]
	uint32_t FORC_PO_RESU		: 1;	// [06]		[0x00]
	uint32_t SUSPEND		: 1;	// [07]		[0x00]
	uint32_t PORTRESET		: 1;	// [08]		[0x00]
	uint32_t RESERVED0		: 1;	// [09]		[0x00]
	uint32_t LINESTATUS		: 2;	// [11:10]	[0x--]
	uint32_t PP			: 1;	// [12]		[0x--]
	uint32_t RESERVED1		: 1;	// [13]		[0x00]
	uint32_t PIC			: 2;	// [15:14]	[0x00]
	uint32_t PTC			: 4;	// [19:16]	[0x00]
	uint32_t WKCNNT_E		: 1;	// [20]		[0x00]
	uint32_t WKDSCNNT		: 1;	// [21]		[0x00]
	uint32_t WKOC_E			: 1;	// [22]		[0x00]
	uint32_t RESERVED2		: 9;	// [31:20]	[0x00]
}EHCI_PORTSC;

typedef	struct{

	uint32_t FLADJ			: 6;	// [05:00]	[0x--]
	uint32_t RESERVED0		: 26;	// [31:06]	[0x00]
}EHCI_FLADJ;

typedef	struct{

	uint32_t OV_BYPASS		: 1;	// [00]		[0x00]
	uint32_t PG_EN			: 1;
	uint32_t SOF_BYPASS		: 1;
	uint32_t SOF_1_BYPASS		: 1;
	uint32_t SOF_MASK		: 1;
	uint32_t INT_OV			: 1;
	uint32_t HS_BYPASS		: 1;
	uint32_t RESERVED7		: 1;
	uint32_t HK_DEL_CNT		: 5;	
	uint32_t RESERVED13		: 26;	// [31:01]	[0x00]

}EHCI_EC_CTL;

typedef	struct{

	uint32_t TYPE_FAIL		: 1;	// [00]		[0x00]
	uint32_t INT_ERR		: 1;	// [01]		[0x00]
	uint32_t TX_BABBLE		: 1;	// [02]		[0x00]
	uint32_t PO_FSDIS_CHG		: 1;
	uint32_t RESERVED4		: 28;	// [31:04]	[0x00]
}EHCI_EC_STS;

typedef	struct{

	uint32_t RESERVED0		:16;
	uint32_t RootHub_TEST		: 1;
	uint32_t Pass_Chirp_Test	: 1;
	uint32_t HS_DISC_DISABLE	: 1;
	uint32_t Pass_Reset		: 1;
	uint32_t HS_SWEN		: 1;
	uint32_t DB_MODE_SEL		: 1;
	uint32_t RESERVED22		: 1;
	uint32_t RootHub_Status		: 9;
}EHCI_UTMI_VC;

typedef	struct{
	
	uint32_t SQLVL			: 2;
	uint32_t RESERVED2		:30;
	
}EHCI_UTMI_PW;

typedef	struct{
	
	uint32_t	RESERVED0	: 1;
	uint32_t	HOST_SPEED	: 2;
	uint32_t	PO_FSEN		: 1;
	uint32_t	RESERVED4	: 4;
	uint32_t	EHCI_GPIO	: 4;
	uint32_t	FS_TEST_PKT_EN	: 1;
	uint32_t	FS_TEST_SE0	: 1;
	uint32_t	FS_TEST_SE1	: 1;
	uint32_t	PARK_MOD_PLUS	: 1;
	uint32_t	PARK_PLUS_CNT	: 4;
	uint32_t	ASYN_PRE_FETCH	: 1;
	uint32_t	RESERVED20	:10;
	uint32_t	SUSP_CLK_ON	: 1;
	
}EHCI_MISC;


typedef	struct{
	
	uint32_t	SOF_FREQ	: 3;
	uint32_t	RESERVED3	:29;		
	
}EHCI_USB_CMD1;

#if defined( CONFIG_PLATFORM_SN98660 ) || defined( CONFIG_PLATFORM_SN7300 )
typedef	struct{
	EHCI_VER_CPLGTH		VER_CPLGTH;		// 0x00
	EHCI_HCSPARAMS		HCSPARAMS;		// 0x04
	EHCI_HCCPARAMS		HCCPARAMS;		// 0x08
	uint32_t		HCSP_PORTROUTE;		// 0x0C
	EHCI_USBCMD		USBCMD;			// 0x10
	EHCI_USBSTS		USBSTS;			// 0x14
	EHCI_USBINTR		USBINTR;		// 0x18
	EHCI_FRINDEX		FRINDEX;		// 0x1C
	uint32_t		SEGMENT_4G;		// 0x20	
	EHCI_PERIODICLISTBASE	PERIODICLISTBASE;	// 0x24
	EHCI_ASYNCLISTADDR	ASYNCLISTADDR;		// 0x28
	uint32_t		RESERVE_2[9];		// 0x2C	~ 4F
	uint32_t		CONFIG_FLAG;		// 0x50
	EHCI_PORTSC		PORTSC[4];		// 0x54	~ 0x63
	uint32_t		RESERVE[7];		// 0x64	~ 0x7C
	EHCI_FLADJ		FLADJ;			// 0x80;
	EHCI_EC_CTL		EC_CTL;			// 0x84
	EHCI_EC_STS		EC_STS;			// 0x88
	EHCI_UTMI_VC		UTMI_VC;		// 0x8C
	uint32_t		RESERVE90[12];		// 0x90	~ 0xBC
	EHCI_UTMI_PW		UTMI_PW;		// 0xC0
	EHCI_MISC		MISC;			// 0xC4	
	
}SONIX_EHCI_STRUCTURE;
#endif


#if defined( CONFIG_PLATFORM_SN58510 )

typedef	struct{
	EHCI_VER_CPLGTH		VER_CPLGTH;		// 0x00
	EHCI_HCSPARAMS		HCSPARAMS;		// 0x04
	EHCI_HCCPARAMS		HCCPARAMS;		// 0x08
	uint32_t		HCSP_PORTROUTE;		// 0x0C
	EHCI_USBCMD		USBCMD;			// 0x10
	EHCI_USBSTS		USBSTS;			// 0x14
	EHCI_USBINTR		USBINTR;		// 0x18
	EHCI_FRINDEX		FRINDEX;		// 0x1C
	uint32_t		SEGMENT_4G;		// 0x20	
	EHCI_PERIODICLISTBASE	PERIODICLISTBASE;	// 0x24
	EHCI_ASYNCLISTADDR	ASYNCLISTADDR;		// 0x28
	uint32_t		RESERVE_2[9];		// 0x2C	~ 4F
	uint32_t		CONFIG_FLAG;		// 0x50
	EHCI_PORTSC		PORTSC[4];		// 0x54	~ 0x63
	uint32_t		RESERVE[7];		// 0x64	~ 0x7C
	EHCI_FLADJ		FLADJ;			// 0x80;
	EHCI_EC_CTL		EC_CTL;			// 0x84
	EHCI_EC_STS		EC_STS;			// 0x88
	EHCI_UTMI_VC		UTMI_VC;		// 0x8C
	uint32_t		RESERVE90[12];		// 0x90	~ 0xBC
	EHCI_UTMI_PW		UTMI_PW;		// 0xC0
	EHCI_MISC		MISC;			// 0xC4	
	uint32_t		RESERVEC8;		// 0xC8
	EHCI_USB_CMD1		USB_CMD1;		// 0xCC		
	
}SONIX_EHCI_STRUCTURE;
#endif


#if defined ( CONFIG_PLATFORM_XILINX_ZYNQ_7000 )
typedef	struct{
	uint32_t 		reserve_000_08C[36];	// 0x000 ~ 0x90
	uint32_t		SBUSCFG;		// 0x90
	uint32_t		reserve_094_0FC[27];	// 0x094 ~ 0x100


	EHCI_VER_CPLGTH		VER_CPLGTH;		// 0x100
	EHCI_HCSPARAMS		HCSPARAMS;		// 0x104
	EHCI_HCCPARAMS		HCCPARAMS;		// 0x108
	uint32_t		HCSP_PORTROUTE;		// 0x10C

	uint32_t		reserve_110_140[12];	// 0x110 ~ 0x140

	EHCI_USBCMD		USBCMD;			// 0x140
	EHCI_USBSTS		USBSTS;			// 0x144
	EHCI_USBINTR		USBINTR;		// 0x148
	EHCI_FRINDEX		FRINDEX;		// 0x14C
	uint32_t		SEGMENT_4G;		// 0x150
	EHCI_PERIODICLISTBASE	PERIODICLISTBASE;	// 0x154
	EHCI_ASYNCLISTADDR	ASYNCLISTADDR;		// 0x158
	uint32_t		TTCTRL;			// 0x15C

	uint32_t		reserve_160_180[8];	// 0x160 ~ 0x180

	uint32_t		CONFIG_FLAG;		// 0x180
	EHCI_PORTSC		PORTSC[5];		// 0x184 ~ 0x194

	uint32_t		reserve_194_1A0[3];	// 0x194 ~ 0x1A0
	uint32_t		OTGSC;			// 0x1A4
	uint32_t		USBMODE;		// 0x1A8

	uint32_t		ENDPSETUPSTART;		// 0x1AC
	uint32_t 		ENDPTPRIME;		// 0x1B0
	uint32_t		ENDPTFLUSH;		// 0x1B4
	uint32_t		ENDPTSTAT;		// 0x1B8
	uint32_t		ENDPTCOMPLETE;		// 0x1BC
	uint32_t		ENDPTCTRL0;		// 0x1C0
	uint32_t		ENDPTCTRL[11];		// 0x1C4 ~ 0x1F0

}SONIX_EHCI_STRUCTURE;
#endif


// EHCI	Host Controller	Status
typedef	struct{

	uint32_t USB_INT		: 1;	// [00]		[0x00]
	uint32_t USB_ERR_INT		: 1;	// [01]		[0x00]
	uint32_t PO_CHG_INT		: 1;	// [02]		[0x00]
	uint32_t ROLLOVER_INT		: 1;	// [03]		[0x00]
	uint32_t SYS_ERR_INT		: 1;	// [04]		[0x00]
	uint32_t ASYNC_ADVANCE_INT	: 1;	// [05]		[0x00]
	uint32_t RESERVED0		: 6;	// [11:06]	[0x00]
	uint32_t HC_HALTED		: 1;	// [12]		[0x01]
	uint32_t RECLAMATION		: 1;	// [13]		[0x00]
	uint32_t PERIODIC_STS		: 1;	// [14]		[0x00]
	uint32_t ASYNC_STS		: 1;	// [15]		[0x00]
	uint32_t RESERVED1		: 16;	// [31:16]	[0x00]
}USBH_STS;

// QTD STATUS
#define	EHCI_QTD_STATUS_Active			0x80
#define	EHCI_QTD_STATUS_Halted			0x40
#define	EHCI_QTD_STATUS_BufferError		0x20
#define	EHCI_QTD_STATUS_Babble			0x10
#define	EHCI_QTD_STATUS_TransactionError	0x08
#define	EHCI_QTD_STATUS_MissMicroFrame		0x04
#define	EHCI_QTD_STATUS_Split			0x02
#define	EHCI_QTD_STATUS_Ping			0x01
#define EHCI_QTD_STATUS_CSW_NOT_VALID	0x03
#define EHCI_QTD_STATUS_TIMEOUT   		0x00
#define EHCI_QTD_STATUS_SUCCESS			  0x78

//qTD Structure	Definition****************************************
 typedef struct	QTD {
	 //<1>.Next_qTD_Pointer Word
	uint32_t	bTerminate		:1;
	uint32_t	bReserve_1		:4;
	uint32_t	bNextQTDPointer		:27;
	
	//<2>.Alternate Next qTD Word
	uint32_t	bAlternateTerminate	:1;
	uint32_t	bReserve_2		:4;
	uint32_t	bAlternateQTDPointer	:27;

	//<3>.Status Word
	uint32_t	bStatus			:8;
	
	uint32_t	bPID			:2;
	uint32_t	bErrorCounter		:2;
	uint32_t	CurrentPage		:3;
	uint32_t	bInterruptOnComplete	:1;
	uint32_t	bTotalBytes		:15;
	uint32_t	bDataToggle		:1;

	//<4>.Buffer Pointer Word Array	 
	uint32_t	ArrayBufferPointer_Word[5];
	
 } EHCI_QTD_STRUCTURE;

//qHD Structure	Definition****************************************
 typedef struct	QH {

	//<1>.Next_qHD_Pointer Word
	uint32_t	bTerminate			:1;
	uint32_t	bType				:2;
	uint32_t	bReserve_1			:2;
	uint32_t	bNextQHDPointer			:27;

	//<2>.qHD_2 Word
	uint32_t	bDeviceAddress			:7;
	uint32_t	bInactiveOnNextTransaction	:1;
	uint32_t	bEdNumber			:4;
	uint32_t	bEdSpeed			:2;
	uint32_t	bDataToggleControl		:1;
	uint32_t	bHeadOfReclamationListFlag	:1;
	uint32_t	bMaxPacketSize			:11;
	uint32_t	bControlEdFlag			:1;
	uint32_t	bNakCounter			:4;

	//<3>.qHD_3 Word
	uint32_t	bInterruptScheduleMask		:8;
	uint32_t	bSplitTransactionMask		:8;
	uint32_t	bHubAddr			:7;
	uint32_t	bPortNumber			:7;
	uint32_t	bHighBandwidth			:2;

	//<4>.Overlay_CurrentqTD
	uint32_t	bOverlay_CurrentqTD;		

	//<5>.Overlay_NextqTD
	uint32_t	bOverlay_NextTerminate		:1;
	uint32_t	bOverlay_Reserve2		:4;
	uint32_t	bOverlay_NextqTD		:27;

	//<6>.Overlay_AlternateNextqTD
	uint32_t	bOverlay_AlternateNextTerminate	:1;
	uint32_t	bOverlay_NanCnt			:4;
	uint32_t	bOverlay_AlternateqTD		:27;

	//<7>.Overlay_TotalBytes
	uint32_t	bOverlay_Status			:8;
	uint32_t	bOverlay_PID			:2;
	uint32_t	bOverlay_ErrorCounter		:2;
	uint32_t	bOverlay_C_Page			:3;
	uint32_t	bOverlay_InterruptOnComplete	:1;
	uint32_t	bOverlay_TotalBytes		:15;
	uint32_t	bOverlay_DT			:1;

	//<8>.Overlay_BufferPointer0	 
	uint32_t	bOverlay_CurrentOffset		:12;
	uint32_t	bOverlay_BufferPointer_0	:20;

	//<9>.Overlay_BufferPointer1	 
	uint32_t	bOverlay_C_Prog_Mask		:8;
	uint32_t	bOverlay_Reserve3		:4;
	uint32_t	bOverlay_BufferPointer_1	:20;
	
	//<10>.Overlay_BufferPointer2	 
	uint32_t	bOverlay_FrameTag		:5;
	uint32_t	bOverlay_S_Bytes		:7;
	uint32_t	bOverlay_BufferPointer_2	:20;

	//<11>.Overlay_BufferPointer3	 
	uint32_t	bOverlay_Reserve4		:12;
	uint32_t	bOverlay_BufferPointer_3	:20;

	//<12>.Overlay_BufferPointer4	 
	uint32_t	bOverlay_Reserve5		:12;
	uint32_t	bOverlay_BufferPointer_4	:20;
	uint32_t	reserve[4];
 } EHCI_QH_STRUCTURE;

  typedef struct _iTD_Status {

 #if 0
	uint32_t	bOffset:12;			//Bit11~0
	uint32_t	bPageSelect:3;			//Bit14~12	  
#endif 	
	uint32_t	bOffset:15;			//Bit14~0

	uint32_t	bInterruptOnComplete:1;		//Bit15  
	uint32_t	bLength:12;			//Bit27~16
	uint32_t	bStatus:4;			//Bit31~28			  
 
 }EHCI_ITD_STATUS_STRUCT;
 
#define	HOST20_iTD_Status_Active		0x08
#define	HOST20_iTD_Status_DataBufferError	0x04
#define	HOST20_iTD_Status_BabbleDetect		0x02
#define	HOST20_iTD_Status_TransctionError	0x01
 
 
typedef struct	_iTD_BufferPointer {

	uint32_t	bParameter:12;			//Bit0~11
	uint32_t	bBufferPointer:20;		//Bit31~12		
	
}EHCI_ITD_BUFFPOINTER_STRUCT;
 
 typedef struct	_iTD {

	uint32_t			bTerminate:1;
	uint32_t			bType:2;
	uint32_t			bReserve_1:2;
	uint32_t			bNextLinkPointer:27;
		 
	EHCI_ITD_STATUS_STRUCT		ArrayStatus_Word[8];

	EHCI_ITD_BUFFPOINTER_STRUCT	ArrayBufferPointer_Word[7];
	  
 }EHCI_ITD_STRUCTURE; 

typedef	struct _SNXiTD {
	//0x00 
	uint32_t		bTerminate:1;	
	uint32_t		bType:2;		
	uint32_t		bSX_EN:1;
	uint32_t		bReserve_1:1;	
	uint32_t		bNextLinkPointer:27; 

	//0x04
	uint32_t		RESERVE[8];

	//0x24
	uint32_t		bDEV_ADDR:7;
	uint32_t		bReserve_2:1;
	uint32_t		bEndPt:4;
	uint32_t		bRING_BUFFER_START:20;

	//0x28
	uint32_t		bMAX_PK_SIZE:11;
	uint32_t		bInactive_On_Next_Trans:1;
	uint32_t		bRING_BUFFER_END:20;

	//0x2C
	uint32_t		bMult:2;
	uint32_t		bRELOAD:1;
	uint32_t		bHEADER:1;
	uint32_t		bDISCARD_EN:1;
	uint32_t		bIOC:1;
	uint32_t		bTransaction_Error:1;
	uint32_t		bBabble_Det:1;
	uint32_t		bACTIVE:1;
	uint32_t		bReserve_3:3;
	uint32_t		bRING_THRESHOLD:20;	

	//0x30
	uint32_t		HW_CURRENT_POINTER;			

	//0x34
	uint32_t		HW_FRAME_END;

	//0x38
	uint32_t		bDATA_DISCARD:1;
	uint32_t		bUNDERFLOW:1;
	uint32_t		bFID_Count:2;
	uint32_t		bFID:1;
	uint32_t		bReserve_4:27;

	//0x3C
	uint32_t		FW_FRAME_END;

 }EHCI_SXITD_STRUCTURE;		
 
 // EHCI integate Structure 
typedef	struct{
	uint32_t			XfrType;
	uint32_t			DataSize;
	uint32_t			DataTog;
	EHCI_QH_STRUCTURE		*pQH;
	EHCI_QTD_STRUCTURE		*pQTD[MAX_QTD_PER_QH];		
	uint32_t			LastQTD;
	EHCI_ITD_STRUCTURE		*pITD[Standard_iTD_EP_Max_Count];
	EHCI_ITD_STRUCTURE		*StartITD;
	EHCI_ITD_STRUCTURE		*LastITD;	
	EHCI_SXITD_STRUCTURE		*pSXITD;
	uint32_t			LastFID;
	uint32_t			LastFwFrameEnd;
	uint32_t 			STD_ISO_SM;
	SemaphoreHandle_t		SEM;
	SemaphoreHandle_t		Mutex;
	uint32_t			Dummy[16];
	uint8_t				*PING_FRAME_PTR;
	uint8_t				*PONG_FRAME_PTR;	
	uint32_t			PING_FRAME_SIZE;
	uint32_t			PONG_FRAME_SIZE;		
	uint32_t 			LAST_TOG:1;
	uint32_t			PING_FRAME_DONE:1;	
	uint32_t			PONG_FRAME_DONE:1;		
	uint32_t			RESERVE:29;	
	uint8_t				UVC_Header;
	uint32_t			ISO_MAXPKT_SIZE;
	uint8_t				status;
	uint8_t				EdNumber; 
}EHCI_ENDP_STRUCT;
 
/** \defgroup USBH_SPEED_DEF USB SPEED definition
 * \ingroup USBH_MODULE
 * 
 * @{
 */ 
 // EHCI SPEED
#define	EHCI_FULL_SPEED		0x0		/**< USB Full speed	*/
#define	EHCI_LOW_SPEED		0x1		/**< USB  Low speed	*/
#define	EHCI_HIGH_SPEED		0x2		/**< USB High speed	*/
/** @} */

// EHCI	Xfr Type
#define	EHCI_NONE_TYPE		USBH_NONE_TYPE
#define	EHCI_CX_TYPE		USBH_CX_TYPE
#define	EHCI_BK_OUT_TYPE	USBH_BK_OUT_TYPE
#define	EHCI_BK_IN_TYPE		USBH_BK_IN_TYPE
#define	EHCI_INT_OUT_TYPE	USBH_INT_OUT_TYPE
#define	EHCI_INT_IN_TYPE	USBH_INT_IN_TYPE
#define	EHCI_ISO_OUT_TYPE	USBH_ISO_OUT_TYPE
#define	EHCI_ISO_IN_TYPE	USBH_ISO_IN_TYPE

// EHCI	ATTRIBUTE
#define	EHCI_CONTROL		0x00
#define	EHCI_ISO		0x01
#define	EHCI_BULK		0x02 
#define	EHCI_INT		0x03
#define	EHCI_SXISO		0x04


#define	EHCI_DATA_IN		1
#define	EHCI_DATA_OUT		0
 
 typedef struct{
	uint32_t	Speed;
	uint32_t	Addr;
	uint32_t	Endp;
	uint32_t	MaxPktSize;
	uint32_t	XfrType;
	uint32_t	DataDir;
	uint32_t	DataSize;
	uint32_t	StructType;
	uint32_t	NextStructType;
	uint32_t	NakCount;
	 
	uint32_t	HubAddr;
	uint32_t	PortNumber;
	uint32_t	SMask;
	uint32_t	CMask;

} EHCI_ALLOC_REQUEST_Struct;

typedef	struct node
{
	struct node	*next;
	struct node	*prev;
	
} NODE;


typedef	struct
{
	uint32_t		Num[EHCI_QH_MAX];
	EHCI_QH_STRUCTURE	*pQH[EHCI_QH_MAX];
	uint32_t		link_tail;
	
}EHCI_QH_LINK_LIST;

typedef	struct
{
	uint32_t	type;	
	uint32_t	order;
	void		*ptr;
	
}EHCI_PERIODIC_ELEMENT_Struct;



typedef	struct
{
	
	EHCI_PERIODIC_ELEMENT_Struct	element[EHCI_PERIODIC_TABLE_MAX];
	uint32_t			link_tail;
	
}EHCI_PERIODIC_TABLE_Struct;


typedef	struct
{
	uint32_t		CMD;
	uint32_t		SIZE;
	uint32_t		ACT_SIZE;
	uint32_t		*pBUFF;
	uint8_t			CLASS[8];
	uint8_t			bRequest;
	uint16_t		wValue;
	uint16_t		wIndex;
	uint8_t			CX_Case;
	uint8_t			*SETUP_CMD_BUFF;
	uint8_t			*STS_DUMMY_BUFF;

}EHCI_CX_XFR_REQ_Struct;

typedef	struct
{
	uint32_t		XfrType;
	uint32_t		NUM;
	uint32_t		SIZE;
	uint32_t		ACT_SIZE;
	uint32_t		*pBUFF;
}EHCI_BK_XFR_REQ_Struct;


typedef	struct
{
	uint32_t		XfrType;
	uint32_t		NUM;
	uint32_t		SIZE;
	uint32_t		ACT_SIZE;
	uint32_t		*pBUFF;
}EHCI_INT_XFR_REQ_Struct;

typedef	struct
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
}EHCI_ISO_XFR_REQ_Struct;




typedef	struct	 {

	  uint32_t		bTerminal:1;		 //Bit11~0
	  uint32_t		bType:2;			//Bit11~0
	  uint32_t		bReserved:2;		//Bit14~12	 
	  uint32_t		bLinkPointer:27;		//Bit15  

}EHCI_PERIODIC_FRAME_LIST_CELL_STRUCT;



typedef	struct	{

	EHCI_PERIODIC_FRAME_LIST_CELL_STRUCT	sCell[Host20_Preiodic_Frame_List_MAX]; 

}EHCI_PERIODIC_FRAME_LIST_STRUCT; 

typedef	struct{
	EHCI_ENDP_STRUCT	*EP;
	EHCI_ISO_XFR_REQ_Struct	*ISO_REQ;
}EHCI_STD_ISO_REQ_STRUCT;
 
typedef struct{
	EHCI_ENDP_STRUCT    *EP;
	EHCI_BK_XFR_REQ_Struct *BULK_REQ;
}EHCI_BULK_REQ_STRUCT;

// EHCI	Global declare 
extern volatile	SONIX_EHCI_STRUCTURE		*EHCI2;
extern volatile	EHCI_QH_STRUCTURE		*pHEAD_QH;
extern volatile	EHCI_PERIODIC_TABLE_Struct	PERIODIC_TABLE;

// Extern prototype 
extern void ehci_hcd_init(void);
extern void ehci_hcd_uninit(void);
extern void ehci_struct_init(void);
extern void ehci_struct_uninit(void);
#if defined( CONFIG_SN_GCC_SDK )
extern void ehci_isr(int irq);
#endif
#if defined( CONFIG_SN_KEIL_SDK ) && defined ( CONFIG_PLATFORM_SN58510 )
extern void ehci_isr(void);
#endif
#if defined( CONFIG_SN_KEIL_SDK ) && defined ( CONFIG_PLATFORM_SN7300 )
extern void USBHOST_IRQHandler(void);
#endif
#if defined( CONFIG_XILINX_SDK ) && defined ( CONFIG_PLATFORM_XILINX_ZYNQ_7000 )
extern void ehci_isr(void *HandlerRef);
#endif

extern void ehci_cfr_chk_isr(void);
extern uint8_t ehci_chk_xfr_result(EHCI_ENDP_STRUCT *EP);
extern EHCI_ENDP_STRUCT	ehci_cx_allocate(EHCI_ALLOC_REQUEST_Struct *AllocReq);
extern uint32_t	ehci_get_structure(EHCI_ALLOC_REQUEST_Struct *AllocReq);
extern void ehci_release_structure(uint32_t  StructType,uint32_t PTR);
extern void ehci_adding_qh_tail(EHCI_QH_STRUCTURE* ADD_QH);
extern void removing_qh(EHCI_QH_STRUCTURE* RMV_QH);
extern void ehci_enable_xfr(EHCI_ENDP_STRUCT *EP);
extern void ehci_disable_xfr(EHCI_ENDP_STRUCT *EP);
extern void ehci_stop_xfr(EHCI_ENDP_STRUCT *EP);
extern void ehci_cx_struct_init(EHCI_ENDP_STRUCT* EP,EHCI_CX_XFR_REQ_Struct *CX_REQ);
extern uint32_t	ehci_get_cx_act_size(EHCI_ENDP_STRUCT*EP,EHCI_CX_XFR_REQ_Struct*CX_REQ);
extern void ehci_bk_struct_init(EHCI_ENDP_STRUCT* EP,EHCI_BK_XFR_REQ_Struct *BK_REQ);
extern uint32_t	ehci_get_bk_act_size(EHCI_ENDP_STRUCT*EP,EHCI_BK_XFR_REQ_Struct*BK_REQ);
extern void ehci_int_struct_init(EHCI_ENDP_STRUCT* EP,EHCI_INT_XFR_REQ_Struct *INT_REQ);
extern void ehci_iso_struct_init(EHCI_ENDP_STRUCT* EP,EHCI_ISO_XFR_REQ_Struct *ISO_REQ);
extern EHCI_ENDP_STRUCT	ehci_bk_allocate(EHCI_ALLOC_REQUEST_Struct *AllocReq);
extern EHCI_ENDP_STRUCT	ehci_int_allocate(EHCI_ALLOC_REQUEST_Struct *AllocReq);
extern EHCI_ENDP_STRUCT	ehci_iso_allocate(EHCI_ALLOC_REQUEST_Struct *AllocReq);

extern void ehci_rollover_enable(void);
extern void ehci_rollover_disable(void);

extern uint8_t ehci_take_ep_sem(EHCI_ENDP_STRUCT* EP,uint32_t TimeoutMsec);
extern uint8_t ehci_move_itd_data_to_buff(EHCI_ENDP_STRUCT* EP,EHCI_ISO_XFR_REQ_Struct *ISO_REQ);


//extern uint8_t	ehci_parser_dev(USBH_Device_Structure * DEV,uint8_t	FUN);
#endif //USB_HCD__H


