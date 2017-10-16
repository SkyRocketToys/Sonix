#ifndef __MEM_TEST_H
#define __MEM_TEST_H

// CB
void *pvPortMalloc_cb( size_t xWantedSize , size_t Module);
void vPortFree_cb( void *pv );
void *pvPortRealloc_cb(void *pv, size_t size , size_t Module);
size_t xPortGetFreeHeapSize_cb( void );
size_t xPortGetMinimumEverFreeHeapSize_cb( void );
size_t* xPortGetHeapUsedSizeByModule_cb( void );
size_t* xPortGetMaximumHeapUsedSizeByModule_cb( void );
void vPortInitialiseBlocks_cb( void );
size_t xPortGetTotalHeapSize_cb( void );

size_t xPortGetHeapStart_cb( void );
// NCB
void *pvPortMalloc_ncnb( size_t xWantedSize , size_t Module);
void vPortFree_ncnb( void *pv );
void *pvPortRealloc_ncnb(void *pv, size_t size , size_t Module);
size_t xPortGetFreeHeapSize_ncnb( void );
size_t xPortGetMinimumEverFreeHeapSize_ncnb( void );
size_t* xPortGetHeapUsedSizeByModule_ncnb( void );
size_t* xPortGetMaximumHeapUsedSizeByModule_ncnb( void );
void vPortInitialiseBlocks_ncnb( void );
size_t xPortGetTotalHeapSize_ncnb( void );

// Pre CB
void *pvPortMalloc_pre_cb( size_t xWantedSize , size_t Module);
void vPortFree_pre_cb( void *pv );
void *pvPortRealloc_pre_cb(void *pv, size_t size , size_t Module);
size_t xPortGetFreeHeapSize_pre_cb( void );
size_t xPortGetMinimumEverFreeHeapSize_pre_cb( void );
size_t* xPortGetHeapUsedSizeByModule_pre_cb( void );
size_t* xPortGetMaximumHeapUsedSizeByModule_pre_cb( void );
void vPortInitialiseBlocks_pre_cb( void );
size_t xPortGetTotalHeapSize_pre_cb( void );

size_t xPortGetHeapStart_pre_cb( void );

// Pre NCB
void *pvPortMalloc_pre_ncnb( size_t xWantedSize , size_t Module);
void vPortFree_pre_ncnb( void *pv );
void *pvPortRealloc_pre_ncnb(void *pv, size_t size , size_t Module);
size_t xPortGetFreeHeapSize_pre_ncnb( void );
size_t xPortGetMinimumEverFreeHeapSize_pre_ncnb( void );
size_t* xPortGetHeapUsedSizeByModule_pre_ncnb( void );
size_t* xPortGetMaximumHeapUsedSizeByModule_pre_ncnb( void );
void vPortInitialiseBlocks_pre_ncnb( void );
size_t xPortGetTotalHeapSize_pre_ncnb( void );



//#if ( configUSE_MEMORY_MONITOR == 1 )
//void MemMonitorInit(void);
//int RegisterMemMonitor(TaskHandle_t *taskTCB);
//int ResignMemMonkitor(TaskHandle_t *taskTCB);
//void *GetMemMonitorList(void);
//#endif
#endif /* __MEM_TEST_H */
