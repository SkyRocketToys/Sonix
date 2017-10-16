/**
 * @file
 * this	is USBH	header file, include this file before use
 * @author IP2 Dept Sonix. (Hammer Huang #1359)
 */
#ifndef	USBH__H
#define	USBH__H	  

#if defined( __cplusplus )
extern "C"{
#endif


#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "USBH-CORE.h"
#include "USBH-ERR-HDL.h"
#include "EHCI-HCD.h"

#include <generated/snx_sdk_conf.h> 



/** \defgroup USBH_DEBUG Debug definition
 * \ingroup USBH_MODULE
 * 
 * @{
 */ 
#define CONFIG_MODULE_USBH_DEBUG 1


#if defined( CONFIG_SN_GCC_SDK )
#if defined( CONFIG_MODULE_USBH_DEBUG )
	#define USBH_DBG(format, args...) print_msg("\nUSBH_DBG:"format, ##args);	/**< MSC Debug Level 1 */	
#else
	#define USBH_DBG(args...) /**< MSC Debug Level 0 */
#endif
	#define USBH_INFO(format, args...) print_msg(format, ##args)
#endif 
	
	
#if defined( CONFIG_SN_KEIL_SDK )
#include "stdio.h"
#if defined( CONFIG_MODULE_USBH_DEBUG )
	#define USBH_DBG(format, args...) printf("\nUSBH_DBG:"format, ##args);	/**< MSC Debug Level 1 */	
#else
	#define USBH_DBG(args...) /**< USBH Debug Level 0 */
#endif
	#define USBH_INFO(format, args...) printf(format, ##args)
#endif


#if defined( CONFIG_XILINX_SDK )
#include "xil_printf.h"
#if defined( CONFIG_MODULE_USBH_DEBUG )
	#define USBH_DBG(format, args...) xil_printf("\r\nUSBH_DBG:"format, ##args);	/**< MSC Debug Level 1 */
#else
	#define USBH_DBG(args...) /**< MSC Debug Level 0 */
#endif
	#define USBH_INFO(format, args...) xil_printf(format, ##args)
#endif

// Call Back 
extern	USBH_PLUG_Struct	plug[];



/** @} */

/** \defgroup USBH_MODULE USBH Driver modules
 * 
 * @{
 */

/** @} */

/** \defgroup MSC_MODULE Mass Storage Class modules
 * 
 * @{
 */

/** @} */


/** \defgroup WIFI_MODULE WIFI Class modules
 * 
 * @{
 */

/** @} */
typedef	enum
{
	USBH_UNKNOW_CLASS=0,
	USBH_WIFI_CLASS,
	USBH_MSC_CLASS,
	USBH_UVC_BULK_CLASS,
	USBH_UVC_ISO_CLASS,
	USBH_HID_CLASS,
	USBH_HUB_CLASS

}USBH_CLASS_DRV_INDEX;

/** \defgroup STATUS_DEF Status	definition
 * \ingroup USBH_MODULE
 * 
 * @{
 */ 
#define	SUCCESS		0	/**< success */
#define	FAIL		1	/**< fail */
#define	NYET		2	/**< not yet */
#define	TIME_OUT	3	/**< timeout */
#define DEV_NOT_EXIST	4		/**< device not exist */
/** @} */

/** \defgroup CAPACITY_DEF Max capacity	definition
 * \ingroup USBH_MODULE
 * 
 * @{
 */ 
#define	USBH_QUEUE_SIZE		10	/**< Max ENDP support */
#define	USBH_MAX_PORT		6	/**< Max Port support */
/** @} */


#if defined( CONFIG_SN_GCC_SDK ) && defined( CONFIG_PLATFORM_SN98660 )
#define	USBH_5ms	5/portTICK_RATE_MS
#define	USBH_10ms	10/portTICK_RATE_MS
#define	USBH_20ms	20/portTICK_RATE_MS
#define	USBH_30ms	30/portTICK_RATE_MS
#define	USBH_40ms	40/portTICK_RATE_MS
#define	USBH_50ms	50/portTICK_RATE_MS
#define	USBH_60ms	60/portTICK_RATE_MS
#define	USBH_70ms	70/portTICK_RATE_MS
#define	USBH_80ms	80/portTICK_RATE_MS
#define	USBH_90ms	90/portTICK_RATE_MS
#define	USBH_100ms	100/portTICK_RATE_MS
#define	USBH_120ms	120/portTICK_RATE_MS
#define	USBH_200ms	200/portTICK_RATE_MS
#define	USBH_500ms	500/portTICK_RATE_MS
#define	USBH_1000ms	1000/portTICK_RATE_MS
#define	USBH_2000ms	2000/portTICK_RATE_MS
#define	USBH_5000ms	5000/portTICK_RATE_MS
#define	USBH_10000ms	10000/portTICK_RATE_MS
#define	USBH_MAX	0xffffffff
#endif 

#if defined( CONFIG_SN_KEIL_SDK  ) && defined( CONFIG_PLATFORM_SN58510 )
#if 1
#define	USBH_10ms	10/portTICK_RATE_MS
#define	USBH_20ms	20/portTICK_RATE_MS
#define	USBH_30ms	30/portTICK_RATE_MS
#define	USBH_40ms	40/portTICK_RATE_MS
#define	USBH_50ms	50/portTICK_RATE_MS
#define	USBH_60ms	60/portTICK_RATE_MS
#define	USBH_70ms	70/portTICK_RATE_MS
#define	USBH_80ms	80/portTICK_RATE_MS
#define	USBH_90ms	90/portTICK_RATE_MS
#define	USBH_100ms	100/portTICK_RATE_MS
#define	USBH_120ms	120/portTICK_RATE_MS
#define	USBH_200ms	200/portTICK_RATE_MS
#define	USBH_500ms	500/portTICK_RATE_MS
#define	USBH_1000ms	1000/portTICK_RATE_MS
#define	USBH_2000ms	2000/portTICK_RATE_MS
#define	USBH_5000ms	5000/portTICK_RATE_MS
#define	USBH_10000ms	10000/portTICK_RATE_MS
#define	USBH_MAX	0xffffffff
#else
#define	USBH_10ms	10
#define	USBH_20ms	20
#define	USBH_30ms	30
#define	USBH_40ms	40
#define	USBH_50ms	50
#define	USBH_60ms	60
#define	USBH_70ms	70
#define	USBH_80ms	80
#define	USBH_90ms	90
#define	USBH_100ms	100
#define	USBH_120ms	120
#define	USBH_200ms	200
#define	USBH_500ms	500
#define	USBH_1000ms	1000
#define	USBH_2000ms	2000
#define	USBH_5000ms	5000
#define	USBH_10000ms	10000
#define	USBH_MAX	0xffffffff
#endif
#endif

/** \defgroup TIMEOUT_DEF Timeout value	definition
 * \ingroup USBH_MODULE
 * 
 * @{
 */ 
#define	USBH_CX_TIMEOUT			USBH_500ms	/**< CX Transfer timeout value */
#define	USBH_BK_OUT_TIMEOUT		USBH_1000ms	/**< BK OUT	Transfer timeout value */
#define	USBH_BK_IN_TIMEOUT		USBH_1000ms	/**< BK IN Transfer	timeout	value */
#define	USBH_WIFI_BK_IN_TIMEOUT	USBH_MAX	/**< BK IN Transfer	timeout	value */
#define	USBH_INT_OUT_TIMEOUT	USBH_500ms
#define	USBH_INT_IN_TIMEOUT	USBH_MAX


/** @} */

#define	BIT0	0x01
#define	BIT1	0x02
#define	BIT2	0x04
#define	BIT3	0x08
#define	BIT4	0x10
#define	BIT5	0x20
#define	BIT6	0x40
#define	BIT7	0x80
#define	BIT8	0x0100
#define	BIT9	0x0200
#define	BIT10	0x0400
#define	BIT11	0x0800
#define	BIT12	0x1000
#define	BIT13	0x2000
#define	BIT14	0x4000
#define	BIT15	0x8000
#define	BIT16	0x010000
#define	BIT17	0x020000
#define	BIT18	0x040000
#define	BIT19	0x080000
#define	BIT20	0x100000
#define	BIT21	0x200000
#define	BIT22	0x400000
#define	BIT23	0x800000
#define	BIT24	0x01000000
#define	BIT25	0x02000000
#define	BIT26	0x04000000
#define	BIT27	0x08000000
#define	BIT28	0x10000000
#define	BIT29	0x20000000
#define	BIT30	0x40000000
#define	BIT31	0x80000000

typedef struct _automount_info_usb{
    xSemaphoreHandle xSEM_USBH_PLUG_IN;
    xSemaphoreHandle xSEM_USBH_PLUG_OUT;
 }automount_info_usbh;

// Semaphore 

extern	SemaphoreHandle_t	USBH_SEM_FRAME_LIST_ROLLOVER;
extern	SemaphoreHandle_t	USBH_SEM_ASYNC_ADV;
extern	SemaphoreHandle_t	USBH_SEM_WAKEUP_WIFI;
extern	SemaphoreHandle_t	USBH_SEM_WAKEUP_AUTO_BKIN;
extern	SemaphoreHandle_t	USBH_SEM_WAKEUP_ERR_HDL;
extern	SemaphoreHandle_t	USBH_SEM_AUTO_BKIN_CNT;
extern	SemaphoreHandle_t	USBH_SEM_PACKET_CNT_MUTEX;
// Queue
extern	QueueHandle_t	USBH_QUEUE_ERR_HDL;	
extern	QueueHandle_t	USBH_QUEUE_ERR_RLT;
// Task	Handle 
extern	TaskHandle_t	xTASK_HDL_USBH_ENUM;
extern	TaskHandle_t	xTASK_HDL_USB_XFR;
extern	TaskHandle_t	xTASK_HDL_USBH_ERRHDL;


extern void usbh_init(void);
extern void usbh_uninit(void); 
extern void usbh_freertos_init(void);
extern void usbh_freertos_uninit(void);
extern void usbh_intr_enable(void);
extern void usbh_enum_task(void	*pvParameters);
extern void usbh_auto_bkin_task(void *pvParameters);
extern void USBH_XFR_TASK(void *pvParameters);
extern void USBH_ERR_HDL_TASK(void *pvParameters);
extern uint32_t	wifi_init(uint8_t ID);
extern uint32_t	msc_init(uint8_t ID);
extern uint32_t	uvc_init(uint8_t ID);
extern uint32_t	hub_init(uint8_t ID);
extern void usbh_demo_task(void	*pvParameters);
extern void usbh_auto_bkin_init(void);
extern void usbh_auto_bkin_uninit(void);

extern void usbh_plug_cb_reg(uint32_t CLASS_TYPE ,void (*in_callback)(void),void (*out_callback)(void));
extern uint8_t usbh_plug_in_cb(uint32_t CLASS_TYPE );
extern uint8_t usbh_plug_out_cb(uint32_t CLASS_TYPE );

extern uint8_t usbh_root_one_suspend();
extern uint8_t usbh_root_one_resume();



/** @} */

#if defined( __cplusplus )
}
#endif


#endif //USBH__H
