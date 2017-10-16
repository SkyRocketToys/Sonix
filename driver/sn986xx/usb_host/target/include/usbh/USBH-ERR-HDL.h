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
 * this	is USBH-ERR-HDL	file, include this file	before use
 * @author IP2 Dept Sonix. (Hammer Huang #1359)
 */
#ifndef	USBH_ERR_HDL__H
#define	USBH_ERR_HDL__H	

#include <generated/snx_sdk_conf.h> 


typedef	enum{
	MSC,
	UVC,
	WIFI,
	HID
}USBH_CLASS_DRIVER;

typedef	enum{
	CMD_FAIL,
	PHASE_ERR,
	EP_STALL,
	CSW_NOT_VALID,
	TIMEOUT,
	CONTINUOUS_ERR,
	NO_RESPONSE
}USBH_MSC_ERR_TYPE;

typedef enum {
	MSC_RESET = 1
}USBH_MEC_ERR_HANDLE_TYPE;

typedef	struct _USBH_ERR_HDL_STRUCT{
	uint8_t DriverClass;
	uint8_t *pbuff;
}USBH_ERR_HDL_STRUCT;

typedef	struct _USBH_ERR_RLT_STRUCT{
	uint8_t *pbuff;
}USBH_ERR_RLT_STRUCT;

void usbh_err_hdl_process(USBH_ERR_HDL_STRUCT *err_hdl,	USBH_ERR_RLT_STRUCT *err_rlt);

extern void usbh_err_hdl_task(void *pvParameters);
#endif //USBH_ERR_HDL__H



