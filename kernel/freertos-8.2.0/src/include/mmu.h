#ifndef MMU_H
#define MMU_H

#define CONFIG_MMU

#ifndef AUTOCONF_INCLUDED
#include <generated/snx_sdk_conf.h>
#endif


/*********************************** MMU Map ****************************************************/
/*----------------------------------============================================================*/
/*     	|                    		|	__ld_FootPrint_End                                  	*/
/*  CB	| configHEAP_CB_SIZE    	============================================================*/
/*      |                       	|	Mallocate memory pool (GFP_KERNEL)                  	*/
/*      |---------------------------|===========================================================*/
/*		| configHEAP_PreCB_SIZE		|	Preallocate memory pool (GFP_KERNAL | GFP_PREALLOCATE)	*/
/*------|---------------------------============================================================*/
/*      | configHEAP_PreNCNB_SIZE  	|	Preallocate memory pool (GFP_DMA | GFP_PREALLOCATE)		*/
/* NCNB |---------------------------|===========================================================*/
/*		| configHEAP_NCNB_SIZE		|	Mallocate memory pool (GFP_DMA)  						*/
/*      |---------------------------============================================================*/
/*      |                       	|	64KB reserved (FW upgrade)                          	*/
/*==============================================================================================*/
/************************************************************************************************/



#define configDDR_SIZE		(CONFIG_DDR_TOTAL_SIZE * 1024 * 1024)

#define ucHeap_cb		(&__ld_FootPrint_End)
#define configHEAP_CB_SIZE	(CONFIG_DDR_CACHEABLE_SIZE * 1024 * 1024) // 12M

#define ucHeap_pre_cb	(configHEAP_CB_SIZE)
#define configHEAP_PreCB_SIZE (CONFIG_DDR_PRE_CB_SIZE * 1024 * 1024)	// 5M

#define ucHeap_pre_ncnb		(configHEAP_CB_SIZE + configHEAP_PreCB_SIZE)
#define configHEAP_PreNCNB_SIZE (CONFIG_DDR_PRE_NCNB_SIZE * 1024 * 1024)	// 18M

//#define ucHeap_ncnb		(configHEAP_CB_SIZE)
//#define configHEAP_NCNB_SIZE	(configDDR_SIZE - ucHeap_ncnb - 0x10000)	// Reserve 64 kb for fwupgrade
#define ucHeap_ncnb		(configHEAP_CB_SIZE + configHEAP_PreCB_SIZE + configHEAP_PreNCNB_SIZE)
#define configHEAP_NCNB_SIZE	(configDDR_SIZE - ucHeap_ncnb - 0x10000)	// Reserve 64 kb for fwupgrade

#endif /* MMU_H */
