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
 * this	is USBH-HUB file, include this file before use
 * @author IP2 Dept Sonix. (Hammer Huang #1359)
 */
#ifndef	USBH_HUB__H
#define	USBH_HUB__H
#if defined( __cplusplus )
extern "C"{
#endif

#include <generated/snx_sdk_conf.h> 


#if defined( CONFIG_SN_GCC_SDK )
#if defined( CONFIG_MODULE_USB_HUB_DEBUG )
	#define HUB_DBG(format, args...) print_msg("\nHUB_DBG:"format, ##args);	/**< MSC Debug Level 1 */	
#else
	#define HUB_DBG(args...) /**< MSC Debug Level 0 */
#endif
	#define HUB_INFO(format, args...) print_msg_queue(format, ##args)
#endif 


#if defined( CONFIG_SN_KEIL_SDK )
#include "stdio.h"	
#if defined( CONFIG_MODULE_USB_HUB_DEBUG )
	#define HUB_DBG(format, args...) printf("\nHUB_DBG:"format, ##args);	/**< MSC Debug Level 1 */	
#else
	#define HUB_DBG(args...) /**< MSC Debug Level 0 */
#endif
	#define HUB_INFO(format, args...) printf(format, ##args)
#endif


#if defined( CONFIG_XILINX_SDK )
#include "xil_printf.h"
#if defined( CONFIG_MODULE_USB_HUB_DEBUG )
	#define HUB_DBG(format, args...) xil_printf("\nHUB_DBG:"format, ##args);	/**< MSC Debug Level 1 */
#else
	#define HUB_DBG(args...) /**< MSC Debug Level 0 */
#endif
	#define HUB_INFO(format, args...) xil_printf(format, ##args)
#endif


typedef	struct _queue_message{
	uint32_t Message;
}USBH_HUB_QUEUE_MESSAGE_STRUCT;



// HUB CMD Index
typedef	enum
{
	// HUB Class
	USBH_HUB_CMD_GET_HUB_DESCRIPTOR,
	USBH_HUB_CMD_GETFEATURE_PORT_STATUS,
	USBH_HUB_CMD_CLEARFEATURE_CPORT_CONNECT,

	USBH_HUB_CMD_SETFEATURE_PORTPOWER,
	USBH_HUB_CMD_SETFEATURE_PORT_SUSPEND,
	USBH_HUB_CMD_SETFEATURE_PORT_RESUME,
	USBH_HUB_CMD_SETFEATURE_PORT_RESET,
	USBH_HUB_CMD_CLEARFEATURE_CPORT_RESET,

	USBH_HUB_CMD_SETFEATURE_TESTJ,
	USBH_HUB_CMD_SETFEATURE_TESTK,
	USBH_HUB_CMD_SETFEATURE_TESTSE0,
	USBH_HUB_CMD_SETFEATURE_TESTPACKET,
	USBH_HUB_CMD_SETFEATURE_DeviceRemoteWakeup

}USBH_HUB_CMD_INDEX;


// HUB SM Index
typedef	enum
{
	USBH_HUB_INIT_STATE,
	USBH_HUB_DET_STATE,
	USBH_HUB_CONN_STATE,
	USBH_HUB_STS_STATE,
	USBH_HUB_DEGLICH_STATE,
	USBH_HUB_RESET_STATE,
	USBH_HUB_ENUM_STATE,
	USBH_HUB_PARSER_STATE,
	USBH_HUB_ACTIVE_STATE,
	USBH_HUB_PLUG_OUT,	
	USBH_HUB_ERR_HDL_STATE,
	USBH_HUB_HALT_STATE	

}USBH_HUB_SM_INDEX;



typedef	struct
{

	uint8_t		bLength;
	uint8_t		bDescriptorType;
	uint8_t		bNbrPorts;
	uint16_t	wHubCharacteristics;
	uint8_t		bPwrOn2PwrGood;
	uint8_t		bHubContrCurrent;
	uint8_t		DeviceRemovable;
	uint8_t		PortPwrCtrlMask;

}USBH_HUB_DESCRIPTOR_Structure;


#define	STS_C_PORT_CONNECTION		0x01
#define	STS_C_PORT_ENABLE		0x02
#define	STS_C_PORT_SUSPEND		0x04
#define	STS_C_PORT_OVER_CURRENT		0x08
#define	STS_C_PORT_RESET		0x10
#define	STS_C_PORT_L1			0x20


typedef	struct
{
	uint8_t	STS_PORT_CONNECTION:1;
	uint8_t	STS_PORT_ENABLE:1;
	uint8_t	STS_PORT_SUSPEND:1;
	uint8_t	STS_PORT_OVER_CURRENT:1;
	uint8_t	STS_PORT_RESET:1;
	uint8_t	STS_PORT_L1:1;
	uint8_t	B0_RESERVE:2;

	uint8_t	STS_PORT_POWER:1;
	uint8_t	STS_PORT_LOW_SPEED:1;
	uint8_t	STS_PORT_HIGH_SPEED:1;
	uint8_t	STS_PORT_TEST:1;
	uint8_t	STS_PORT_INDICATOR_CTRL:1;
	uint8_t	B1_RESERVE:3;

	uint8_t	STS_C_PORT;

	uint8_t	B3_RESERVE:8;

}USBH_HUB_PORT_STATUS_Structure;



typedef	struct
{
	uint8_t				SM;
	uint8_t				STS_CHANGE;	  
	USBH_HUB_DESCRIPTOR_Structure	DES;
	USBH_HUB_PORT_STATUS_Structure	PORT_STS;


}USBH_HUB_CLASS_Structure;


extern	SemaphoreHandle_t USBH_SEM_WAKEUP_HUB;

extern void hub_task(void * pvParameters);
extern void hub_task_init(void);
extern void hub_task_uninit(void);
extern void usbh_hub_enum(  USBH_Device_Structure * DEV);



#endif //USBH_HUB__H



