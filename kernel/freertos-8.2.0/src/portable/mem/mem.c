#include <stdlib.h>
//#include <stdio.h>

#include "FreeRTOS.h"
#include <task.h>
#include "mem.h"

extern unsigned int __ld_FootPrint_End;
#include "mmu.h"




#if ( configUSE_MEMORY_MONITOR == 1 )
#define MONITOR_ITEMS		256
TaskHandle_t *MonitorList[MONITOR_ITEMS];

//typedef struct MemoryMonitor
//{
//	TaskHandle_t *tsk_Handle;
	//char tsk_name[configMAX_TASK_NAME_LEN];
//} MemoryMonitor_t;

//MemoryMonitor_t MonitorList[MONITOR_ITEMS];
#endif

static void *pvPortMalloc_UnChecked(size_t xSize, int flags, size_t Module)
{
	void *addr = NULL;
	if(Module>=MODULE_DEF_MAX)
	{
		//printf("Invalid malloc module ID %d(0x%x, %s)\n", Module, xSize, flags==GFP_KERNEL?"GFP_KERNEL":"GFP_DMA");
		return NULL;
	}
	if (flags & GFP_KERNEL) {
		if(flags & GFP_PREALLOCATE) {
			addr = pvPortMalloc_pre_cb(xSize, Module);
		}
		else {
			addr = pvPortMalloc_cb(xSize, Module);
		}
	} else if (flags & GFP_DMA) {
		if(flags & GFP_PREALLOCATE) {
			addr = pvPortMalloc_pre_ncnb(xSize, Module);
		}
		else {
			addr = pvPortMalloc_ncnb(xSize, Module);
		}
	}
	if(addr == NULL) {
			print_msg("\n-------------------------------------\n");
			print_msg("<<Allocation Fail>><%s%s> %d [%d]\n"
						, (flags  & GFP_DMA)?"NCNB":"CB"
						, (flags  & GFP_PREALLOCATE)?"_PREALLOCATE":""						
						, xSize, Module);
			print_msg("-------------------------------------\n\n");

			if (flags & GFP_KERNEL) {
				if(flags & GFP_PREALLOCATE) {
					print_msg("PreCB free size = %d\n", xPortGetFreeHeapSize(GFP_KERNEL|GFP_PREALLOCATE));
				}
				else {
					print_msg("CB free size = %d\n", xPortGetFreeHeapSize(GFP_KERNEL));
				}
			} else if (flags & GFP_DMA) {
				if(flags & GFP_PREALLOCATE) {
					print_msg("PreNCNB free size = %d\n", xPortGetFreeHeapSize(GFP_DMA|GFP_PREALLOCATE));
				}
				else {
					print_msg("NCNB free size = %d\n",xPortGetFreeHeapSize(GFP_DMA));
				}
			}

			print_msg("-------------------------------------\n\n");
	}
	return addr;
}

static void vPortFree_UnChecked(void *pv)
{
//printf("pv=0x%x, ucHeap_cb=0x%x, ucHeap_ncnb=0x%x\n", (unsigned int)pv, (unsigned int)ucHeap_cb, (unsigned int)ucHeap_ncnb);

//	if (((unsigned int)pv >= (unsigned int)ucHeap_cb) && ((unsigned int)pv < (0x0 + configHEAP_CB_SIZE)))
	if (((unsigned int)pv >= (unsigned int)ucHeap_cb) 
			&& ((unsigned int)pv < (0x0 + configHEAP_CB_SIZE))) {
		vPortFree_cb(pv);
	}
	else if (((unsigned int)pv >= (unsigned int)ucHeap_ncnb) 
			&& ((unsigned int)pv < (unsigned int)ucHeap_ncnb + configHEAP_NCNB_SIZE))
		vPortFree_ncnb(pv);
	else if (((unsigned int)pv >= (unsigned int)ucHeap_pre_cb) 
			&& ((unsigned int)pv < (unsigned int)ucHeap_pre_cb + configHEAP_PreCB_SIZE))
		vPortFree_pre_cb(pv);
	else if (((unsigned int)pv >= (unsigned int)ucHeap_pre_ncnb) 
			&& ((unsigned int)pv < (unsigned int)ucHeap_pre_ncnb + configHEAP_PreNCNB_SIZE))
		vPortFree_pre_ncnb(pv);
	
//	else
//		printf("Invalid pv address 0x%x\n", pv);
}

static void *pvPortRealloc_UnChecked(void *pv, size_t size, size_t Module)
{

	if(Module>=MODULE_DEF_MAX)
	{
		//printf("Invalid realloc module ID %d(0x%x, %s)\n", Module, xSize, flags==GFP_KERNEL?"GFP_KERNEL":"GFP_DMA");
		return 0;
	}
	if (((unsigned int)pv >= (unsigned int)ucHeap_cb) 
		&& ((unsigned int)pv < (0x0 + configHEAP_CB_SIZE)))
		return pvPortRealloc_cb(pv, size, Module);
	else if (((unsigned int)pv >= (unsigned int)ucHeap_ncnb) 
		&& ((unsigned int)pv < (unsigned int)ucHeap_ncnb + configHEAP_NCNB_SIZE))
		return pvPortRealloc_ncnb(pv, size, Module);
	else if (((unsigned int)pv >= (unsigned int)ucHeap_pre_cb) 
		&& ((unsigned int)pv < (unsigned int)ucHeap_pre_cb + configHEAP_PreCB_SIZE))
		return pvPortRealloc_pre_cb(pv, size, Module);
	else if (((unsigned int)pv >= (unsigned int)ucHeap_pre_ncnb)
		&& ((unsigned int)pv < (unsigned int)ucHeap_pre_ncnb + configHEAP_PreNCNB_SIZE))
		return pvPortRealloc_pre_ncnb(pv, size, Module);
	else if (pv == NULL)
		return pvPortMalloc_cb(size, Module);

	return 0;
}

size_t xPortGetFreeHeapSize(int flags)
{
	size_t size = 0;
	if (flags & GFP_KERNEL)
		if(flags & GFP_PREALLOCATE) 
			size = xPortGetFreeHeapSize_pre_cb();
		else
			size = xPortGetFreeHeapSize_cb();
	else if (flags & GFP_DMA)
		if(flags & GFP_PREALLOCATE) 
			size = xPortGetFreeHeapSize_pre_ncnb();
		else
			size = xPortGetFreeHeapSize_ncnb();

//	print_msg("size == %d\n", size);
	return size;

//	return 0;
}

size_t xPortGetTotalHeapSize(int flags)
{
	if (flags & GFP_KERNEL)
		if(flags & GFP_PREALLOCATE) 
			return xPortGetTotalHeapSize_pre_cb();
		else
			return xPortGetTotalHeapSize_cb();
	else if (flags & GFP_DMA) {
		if(flags & GFP_PREALLOCATE) 
			return xPortGetTotalHeapSize_pre_ncnb();
		else
			return xPortGetTotalHeapSize_ncnb();
	}
	return 0;
}

size_t xPortGetMinimumEverFreeHeapSize(int flags)
{
	if (flags & GFP_KERNEL)
		if(flags & GFP_PREALLOCATE) 
			return xPortGetMinimumEverFreeHeapSize_pre_cb();
		else
			return xPortGetMinimumEverFreeHeapSize_cb();
	else if (flags & GFP_DMA)
		if(flags & GFP_PREALLOCATE) 
			return xPortGetMinimumEverFreeHeapSize_pre_ncnb();
		else
			return xPortGetMinimumEverFreeHeapSize_ncnb();

	return 0;
}

size_t xPortGetHeapStart(void)
{
	return xPortGetHeapStart_cb();
}

size_t* xPortGetHeapUsedSizeByModule(int flags)
{
	if (flags & GFP_KERNEL)
		if(flags & GFP_PREALLOCATE) 
			return xPortGetHeapUsedSizeByModule_pre_cb();
		else
			return xPortGetHeapUsedSizeByModule_cb();
	else if (flags & GFP_DMA)
		if(flags & GFP_PREALLOCATE) 
			return xPortGetHeapUsedSizeByModule_pre_ncnb();
		else
			return xPortGetHeapUsedSizeByModule_ncnb();

	return 0;

}

size_t* xPortGetMaximumHeapUsedSizeByModule(int flags)
{
	if (flags & GFP_KERNEL)
		if(flags & GFP_PREALLOCATE) 
			return xPortGetMaximumHeapUsedSizeByModule_pre_cb();
		else
			return xPortGetMaximumHeapUsedSizeByModule_cb();
	else if (flags & GFP_DMA)
		if(flags & GFP_PREALLOCATE) 
			return xPortGetMaximumHeapUsedSizeByModule_pre_ncnb();
		else
			return xPortGetMaximumHeapUsedSizeByModule_ncnb();

	return 0;

}

void vPortInitialiseBlocks(int flags)
{
	if (flags & GFP_KERNEL)
		if(flags & GFP_PREALLOCATE) 
			return vPortInitialiseBlocks_pre_cb();
		else		
			return vPortInitialiseBlocks_cb();
	else if (flags & GFP_DMA)
		if(flags & GFP_PREALLOCATE) 
			return vPortInitialiseBlocks_pre_ncnb();
		else
			return vPortInitialiseBlocks_ncnb();
}

#if ( configUSE_MEMORY_MONITOR == 1 )
void MemMonitorInit()
{
	int i = 0;

	for (i = (MONITOR_ITEMS - 1) ; i >= 0; i--) {
		MonitorList[i] = NULL;
	}
}

int RegisterMemMonitor(TaskHandle_t *taskTCB)
{
	int i = 0;
	int ret = pdFAIL;

	for (i = 0 ; i < MONITOR_ITEMS ; i++) {
		if (MonitorList[i] == NULL) {
			MonitorList[i] = taskTCB;
			ret = pdPASS;

			break;
		}
	}
	return ret;
}

int ResignMemMonkitor(TaskHandle_t *taskTCB)
{
	int i = 0;
	int ret = pdFAIL;

	for (i = 0 ; i < MONITOR_ITEMS ; i++) {
		if (MonitorList[i] == taskTCB) {
			MonitorList[i] = NULL;
			ret = pdPASS;

			break;
		}
	}

	return ret;
}

void *GetMemMonitorList()
{
	return (void *)MonitorList;
}

#endif


/*
  memory checking wrappers to catch memory corruption

  This is disabled by default as it causes issues with recording to microSD
 */
#define MEM_CHECK_ENABLED 0

#undef pvPortMalloc

#if MEM_CHECK_ENABLED
#define MEM_GUARD_SIZE 4
#define MEM_GUARD_BASE 42

struct mem_header {
    size_t size;
    const char *location;
    const char *file;
    unsigned line;
    uint8_t guard[MEM_GUARD_SIZE];
};
struct mem_footer {
    uint8_t guard[MEM_GUARD_SIZE];
};

#define GUARD_VAL(i) ((uint8_t)((i)+MEM_GUARD_BASE))

void mem_check_guards_FL(void *ptr, const char *file, unsigned line)
{
    struct mem_header *h = ((struct mem_header *)ptr)-1;
    struct mem_footer *f = (struct mem_footer *)(((char *)ptr)+h->size);
    if (ptr == NULL) {
        return;
    }
    unsigned i;
    for (i=0; i<MEM_GUARD_SIZE; i++) {
        if (h->guard[i] != GUARD_VAL(i)) {
            print_msg("bad header guard %u at %s:%u\n", i, file, line);
            return;
        }
    }
    for (i=0; i<MEM_GUARD_SIZE; i++) {
        if (f->guard[i] != GUARD_VAL(i)) {
            print_msg("bad footer guard %u at %s:%u\n", i, file, line);
            return;
        }
    }
}

void *pvPortMalloc(size_t xSize, int flags, size_t Module)
{
    size_t csize = xSize + sizeof(struct mem_header) + sizeof(struct mem_footer);
    void *ptr = pvPortMalloc_UnChecked(csize, flags, Module);
    if (ptr == NULL) {
        return NULL;
    }
    struct mem_header *h = (struct mem_header *)ptr;
    struct mem_footer *f = (struct mem_footer *)(((char *)ptr)+xSize+sizeof(struct mem_header));
    unsigned i;
    h->size = xSize;
    h->file = NULL;
    h->line = 0;
    for (i=0; i<MEM_GUARD_SIZE; i++) {
        h->guard[i] = GUARD_VAL(i);
        f->guard[i] = GUARD_VAL(i);
    }
    return (void*)(h+1);
}

void vPortFree(void *pv)
{
    struct mem_header *h = ((struct mem_header *)pv)-1;
    if (pv != NULL) {
        mem_check_guards(pv);
        vPortFree_UnChecked(h);
    }
}

void *pvPortRealloc(void *pv, size_t size, size_t Module)
{
    struct mem_header *h = ((struct mem_header *)pv)-1;
    struct mem_footer *f;
    const char *file;
    unsigned line;
    if (pv == NULL) {
        return NULL;
    }
    if (size == 0) {
        vPortFree(pv);
        return;
    }
    mem_check_guards(pv);

    file = h->file;
    line = h->line;
    
    pv = pvPortRealloc_UnChecked(h, size+sizeof(struct mem_header)+sizeof(struct mem_footer), Module);
    if (pv == NULL) {
        return NULL;
    }
    h = (struct mem_header *)pv;
    f = (struct mem_footer *)(((char *)pv)+size+sizeof(struct mem_header));
    unsigned i;
    h->size = size;
    h->file = file;
    h->line = line;
    for (i=0; i<MEM_GUARD_SIZE; i++) {
        h->guard[i] = GUARD_VAL(i);
        f->guard[i] = GUARD_VAL(i);
    }
    return (void*)(h+1);
}

/*
  alloc with file and line
 */
void *pvPortMalloc_FL(size_t xSize, int flags, size_t Module, const char *file, unsigned line)
{
    void *pv = pvPortMalloc(xSize, flags, Module);
    if (pv) {
        struct mem_header *h = ((struct mem_header *)pv)-1;
        h->file = file;
        h->line = line;
    }
    return pv;
}

#else // MEM_CHECK_ENABLED

void *pvPortMalloc(size_t xSize, int flags, size_t Module)
{
    return pvPortMalloc_UnChecked(xSize, flags, Module);
}

void vPortFree(void *pv)
{
    vPortFree_UnChecked(pv);
}

void *pvPortRealloc(void *pv, size_t size, size_t Module)
{
    return pvPortRealloc_UnChecked(pv, size, Module);
}

void *pvPortMalloc_FL(size_t xSize, int flags, size_t Module, const char *file, unsigned line)
{
    return pvPortMalloc(xSize, flags, Module);
}
#endif // MEM_CHECK_ENABLED
