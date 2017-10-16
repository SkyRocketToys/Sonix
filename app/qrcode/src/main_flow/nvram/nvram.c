
//#include <libmid_nvram/snx_mid_nvram.h>
#include <FreeRTOS.h>
#include <task.h>
#include <bsp.h>
#include <timers.h>
#include <nonstdlib.h>
#include <string.h>
#include <stdio.h>
#include <queue.h>
#include <semphr.h>
#include "sglib.h"
#include "nvram.h"
#include "debug.h"
#include <libmid_nvram/snx_mid_nvram.h>

nvram_handle_t nvram_the_list;

int NVRAMLIST_COMPARATOR(nvram_info_t *s1,nvram_info_t *s2)
{
	return strcmp(s1->nvram_ckg_name,s2->nvram_ckg_name);
}

SGLIB_DEFINE_DL_LIST_PROTOTYPES(nvram_info_t, NVRAMLIST_COMPARATOR, ptr_to_previous, ptr_to_next);
SGLIB_DEFINE_DL_LIST_FUNCTIONS(nvram_info_t, NVRAMLIST_COMPARATOR, ptr_to_previous, ptr_to_next);

static int del_all_nvramnode(nvram_info_t **list,xSemaphoreHandle nvram_mutex);


static void nvram_mutex_init()
{
	memset(&nvram_the_list, 0x0, sizeof(nvram_handle_t));
	nvram_the_list.nvram_mutex = xSemaphoreCreateMutex();
}

static void nvram_mutex_uinit()
{
   vSemaphoreDelete(nvram_the_list.nvram_mutex);
}


void nvram_list_init()
{
    nvram_mutex_init();
}


void nvram_list_uinit()
{  
	del_all_nvramnode(&nvram_the_list.nvram_list,nvram_the_list.nvram_mutex);
	nvram_mutex_uinit();
}

/**
* @brief    clean all node in list 
* @param list: double pointer to list
* @return return 0 if success
*/
static int del_all_nvramnode(nvram_info_t **list,xSemaphoreHandle nvram_mutex)
{
	nvram_info_t *l;
	xSemaphoreTake(nvram_mutex, portMAX_DELAY);
	struct sglib_nvram_info_t_iterator  it;
	if(*list!=NULL)
	{
		for(l=sglib_nvram_info_t_it_init(&it,*list); l!=NULL; l=sglib_nvram_info_t_it_next(&it)) 
		{	
			sglib_nvram_info_t_delete(list,l);
			vPortFree(l->nvram_pkg_name);
			vPortFree(l->nvram_ckg_name);
			vPortFree(l->data);
			vPortFree(l->data);
			vPortFree(l);
		}
		vPortFree(*list);
	} 
	xSemaphoreGive(nvram_mutex);
	*list = NULL;
	list = NULL;
	return 0;
}


static int add_nvram_node(nvram_info_t **nvram_list,char *pkg_name, char *cfg_name, void* data, nvram_data_type_t data_type,xSemaphoreHandle nvram_mutex)   
{	
	int rc = 0;
	nvram_info_t *datainfo;
	void* datacopy;
	if(!(datainfo = (nvram_info_t*)pvPortMalloc(sizeof(nvram_info_t), GFP_KERNEL, MODULE_APP)))
	{		
		CSTREAMER_DEBUGF(CS_MAIN_DEBUG|CSTREAMER_DBG_TRACE,("malloc failed!\n"));		
		return -1;		
	}
	if(data_type==DT_INT)
	{
		if(!(datacopy = (int*)pvPortMalloc(sizeof(int), GFP_KERNEL, MODULE_APP)))
		{		
			CSTREAMER_DEBUGF(CS_MAIN_DEBUG|CSTREAMER_DBG_TRACE,("data copy malloc failed!\n")); 	
			return -1;		
		}
		memset(datacopy, 0x0, sizeof(int));
        (*(int*)datacopy) =(*(int*)(data));
		
	}else if(data_type==DT_UINT)
	{
		if(!(datacopy = (unsigned int*)pvPortMalloc(sizeof(unsigned int), GFP_KERNEL, MODULE_APP)))
		{		
			CSTREAMER_DEBUGF(CS_MAIN_DEBUG|CSTREAMER_DBG_TRACE,("data copy malloc failed!\n")); 	
			return -1;		
		}
		memset(datacopy, 0x0, sizeof(unsigned int));
        (*(unsigned int*)datacopy) =(*(unsigned int*)(data));
	}else if(data_type==DT_STRING)
	{
		if(!(datacopy = (char*)pvPortMalloc(strlen(((char*)data)+1), GFP_KERNEL, MODULE_APP)))
		{		
			CSTREAMER_DEBUGF(CS_MAIN_DEBUG|CSTREAMER_DBG_TRACE,("data copy malloc failed!\n")); 	
			return -1;		
		}
        strcpy((char*)datacopy,(char*)data);
		//CSTREAMER_DEBUGF(CS_MAIN_DEBUG|CSTREAMER_DBG_TRACE,("datacopy==%s\n",datacopy)); 
	}
	memset(datainfo, 0x0, sizeof(nvram_info_t));
	datainfo->nvram_pkg_name = strdup(pkg_name);
	datainfo->nvram_ckg_name = strdup(cfg_name);
	datainfo->data_type=data_type;
	datainfo->data=(void*)datacopy;
	//CSTREAMER_DEBUGF(CS_MAIN_DEBUG|CSTREAMER_DBG_TRACE,("datainfo->nvram_pkg_name==%s\n",datainfo->nvram_pkg_name));
	//CSTREAMER_DEBUGF(CS_MAIN_DEBUG|CSTREAMER_DBG_TRACE,("datainfo->nvram_ckg_name==%s\n",datainfo->nvram_ckg_name));
	//CSTREAMER_DEBUGF(CS_MAIN_DEBUG|CSTREAMER_DBG_TRACE,("datainfo->data_type==%d\n",datainfo->data_type));
	xSemaphoreTake(nvram_mutex, portMAX_DELAY);
	sglib_nvram_info_t_add(nvram_list,datainfo);
	xSemaphoreGive(nvram_mutex);
	//print_nvram_node();
}

void print_nvram_node()
{
	nvram_info_t *l;
	xSemaphoreTake(nvram_the_list.nvram_mutex, portMAX_DELAY);
	if(nvram_the_list.nvram_list!=NULL)
	{
		for(l=sglib_nvram_info_t_get_first(nvram_the_list.nvram_list);l!=NULL; l=l->ptr_to_next)
		{
			CSTREAMER_DEBUGF(CS_MAIN_DEBUG|CSTREAMER_DBG_TRACE,("print data value=%d\n",(*(int*)(l->data))));
		}
	}
	xSemaphoreGive(nvram_the_list.nvram_mutex);
}


int snx_nvram_integer_set_to_ddr(char *pkg_name, char *cfg_name, int data)
{
	nvram_info_t *l;
    int rc=-1;
	if ((rc=snx_nvram_integer_set_to_ddr(pkg_name, cfg_name, data)) == NVRAM_SUCCESS) 
	{
		xSemaphoreTake(nvram_the_list.nvram_mutex, portMAX_DELAY);
		for(l=sglib_nvram_info_t_get_first(nvram_the_list.nvram_list);l!=NULL; l=l->ptr_to_next)
		{
		  	if((strncmp(l->nvram_pkg_name, pkg_name, strlen(pkg_name))== 0)&&(strncmp(l->nvram_ckg_name, cfg_name, strlen(cfg_name))== 0))
		  	{
                (*(int*)(l->data))=data;
				xSemaphoreGive(nvram_the_list.nvram_mutex);
				return rc;
		  	}
		}
		xSemaphoreGive(nvram_the_list.nvram_mutex);
        add_nvram_node(&nvram_the_list.nvram_list,pkg_name,cfg_name,(void*)(&data),DT_INT,nvram_the_list.nvram_mutex);
	}	
    return rc;	
}




int snx_nvram_integer_get_from_ddr(char *pkg_name, char *cfg_name, int *data)
{
	nvram_info_t *l;
	int rc=-1;
	if(nvram_the_list.nvram_list==NULL)
    {	
		if ((rc=snx_nvram_integer_get(pkg_name, cfg_name, data)) == NVRAM_SUCCESS)
		{
			add_nvram_node(&nvram_the_list.nvram_list,pkg_name,cfg_name,(void*)(data),DT_INT,nvram_the_list.nvram_mutex);
		}
		else
		{
			CSTREAMER_DEBUGF(CS_MAIN_DEBUG|CSTREAMER_DBG_TRACE,("get nvram fail pkg_name=%s,cfg_name=%s,rc=%d\n",pkg_name,cfg_name,rc));	
		}
		return rc;
    }
	else
	{
		xSemaphoreTake(nvram_the_list.nvram_mutex, portMAX_DELAY);
		for(l=sglib_nvram_info_t_get_first(nvram_the_list.nvram_list);l!=NULL; l=l->ptr_to_next)
		{
			  if((strncmp(l->nvram_pkg_name, pkg_name, strlen(pkg_name))== 0)&&(strncmp(l->nvram_ckg_name, cfg_name, strlen(cfg_name))== 0))
              {
				    //CSTREAMER_DEBUGF(CS_MAIN_DEBUG|CSTREAMER_DBG_TRACE,("hit\n"));  
					*data=(*(int*)(l->data));
					xSemaphoreGive(nvram_the_list.nvram_mutex);
              		return NVRAM_SUCCESS;
              }
		}
		xSemaphoreGive(nvram_the_list.nvram_mutex);
        //not found pkgname and cfg_name
        if ((rc=snx_nvram_integer_get(pkg_name, cfg_name, data)) == NVRAM_SUCCESS)
		{
			add_nvram_node(&nvram_the_list.nvram_list,pkg_name,cfg_name,(void*)(data),DT_INT,nvram_the_list.nvram_mutex);
		}
		else
		{
			CSTREAMER_DEBUGF(CS_MAIN_DEBUG|CSTREAMER_DBG_TRACE,("get nvram fail pkg_name=%s,cfg_name=%s,rc=%d\n",pkg_name,cfg_name,rc));	
		}
		return rc;
	}
}



int snx_nvram_unsign_integer_set_to_ddr(char *pkg_name, char *cfg_name, unsigned int data)
{
	nvram_info_t *l;
    int rc=-1;
	if ((rc=snx_nvram_unsign_integer_set(pkg_name, cfg_name, data)) == NVRAM_SUCCESS) 
	{
		xSemaphoreTake(nvram_the_list.nvram_mutex, portMAX_DELAY);
		for(l=sglib_nvram_info_t_get_first(nvram_the_list.nvram_list);l!=NULL; l=l->ptr_to_next)
		{
		  	if((strncmp(l->nvram_pkg_name, pkg_name, strlen(pkg_name))== 0)&&(strncmp(l->nvram_ckg_name, cfg_name, strlen(cfg_name))== 0))
		  	{
                (*(unsigned int*)(l->data))=data;
				xSemaphoreGive(nvram_the_list.nvram_mutex);
				return rc;
		  	}
		}
		xSemaphoreGive(nvram_the_list.nvram_mutex);
        add_nvram_node(&nvram_the_list.nvram_list,pkg_name,cfg_name,(void*)(&data),DT_INT,nvram_the_list.nvram_mutex);
	}	
    return rc;	
}


int snx_nvram_unsign_integer_get_from_ddr(char *pkg_name, char *cfg_name, unsigned int *data)
{
	nvram_info_t *l;
	int rc=-1;
	if(nvram_the_list.nvram_list==NULL)
    {	
		if ((rc=snx_nvram_unsign_integer_get(pkg_name, cfg_name, data)) == NVRAM_SUCCESS)
		{
			add_nvram_node(&nvram_the_list.nvram_list,pkg_name,cfg_name,(void*)(data),DT_UINT,nvram_the_list.nvram_mutex);
		}
		else
		{
			CSTREAMER_DEBUGF(CS_MAIN_DEBUG|CSTREAMER_DBG_TRACE,("get nvram fail pkg_name=%s,cfg_name=%s,rc=%d\n",pkg_name,cfg_name,rc));	
		}
		return rc;
    }
	else
	{
		xSemaphoreTake(nvram_the_list.nvram_mutex, portMAX_DELAY);
		for(l=sglib_nvram_info_t_get_first(nvram_the_list.nvram_list);l!=NULL; l=l->ptr_to_next)
		{
			  if((strncmp(l->nvram_pkg_name, pkg_name, strlen(pkg_name))== 0)&&(strncmp(l->nvram_ckg_name, cfg_name, strlen(cfg_name))== 0))
              {
				    CSTREAMER_DEBUGF(CS_MAIN_DEBUG|CSTREAMER_DBG_TRACE,("hit\n"));  
					*data=(*(unsigned int*)(l->data));
					xSemaphoreGive(nvram_the_list.nvram_mutex);
              		return NVRAM_SUCCESS;
              }
		}
		xSemaphoreGive(nvram_the_list.nvram_mutex);
        //not found pkgname and cfg_name
        if ((rc=snx_nvram_unsign_integer_get(pkg_name, cfg_name, data)) == NVRAM_SUCCESS)
		{
			add_nvram_node(&nvram_the_list.nvram_list,pkg_name,cfg_name,(void*)(data),DT_UINT,nvram_the_list.nvram_mutex);
		}
		else
		{
			CSTREAMER_DEBUGF(CS_MAIN_DEBUG|CSTREAMER_DBG_TRACE,("get nvram fail pkg_name=%s,cfg_name=%s,rc=%d\n",pkg_name,cfg_name));	
		}
		return rc;
	}
}




int snx_nvram_string_set_to_ddr(char *pkg_name, char *cfg_name,char *data)
{
	nvram_info_t *l;
    int rc=-1;
	if ((rc=snx_nvram_string_set(pkg_name, cfg_name, data)) == NVRAM_SUCCESS) 
	{
		xSemaphoreTake(nvram_the_list.nvram_mutex, portMAX_DELAY);
		for(l=sglib_nvram_info_t_get_first(nvram_the_list.nvram_list);l!=NULL; l=l->ptr_to_next)
		{
		  	if((strncmp(l->nvram_pkg_name, pkg_name, strlen(pkg_name))== 0)&&(strncmp(l->nvram_ckg_name, cfg_name, strlen(cfg_name))== 0))
		  	{
                //(*(unsigned int*)(l->data))=data;
                //strcpy(data,(char*)(l->data));
                strcpy((char*)(l->data),data);
				xSemaphoreGive(nvram_the_list.nvram_mutex);
				return rc;
		  	}
		}
		xSemaphoreGive(nvram_the_list.nvram_mutex);
        add_nvram_node(&nvram_the_list.nvram_list,pkg_name,cfg_name,(void*)(&data),DT_INT,nvram_the_list.nvram_mutex);
	}	
    return rc;	
}



int snx_nvram_string_get_from_ddr(char *pkg_name, char *cfg_name, char *data)
{
	nvram_info_t *l;
	int rc=-1;
	if(nvram_the_list.nvram_list==NULL)
    {	
		if ((rc=snx_nvram_string_get(pkg_name, cfg_name, data)) == NVRAM_SUCCESS)
		{
			add_nvram_node(&nvram_the_list.nvram_list,pkg_name,cfg_name,(void*)(data),DT_STRING,nvram_the_list.nvram_mutex);
		}
		else
		{
			CSTREAMER_DEBUGF(CS_MAIN_DEBUG|CSTREAMER_DBG_TRACE,("get nvram fail pkg_name=%s,cfg_name=%s,rc=%d\n",pkg_name,cfg_name,rc));	
		}
		return rc;
    }
	else
	{
		xSemaphoreTake(nvram_the_list.nvram_mutex, portMAX_DELAY);
		for(l=sglib_nvram_info_t_get_first(nvram_the_list.nvram_list);l!=NULL; l=l->ptr_to_next)
		{
			  if((strncmp(l->nvram_pkg_name, pkg_name, strlen(pkg_name))== 0)&&(strncmp(l->nvram_ckg_name, cfg_name, strlen(cfg_name))== 0))
              {
				    CSTREAMER_DEBUGF(CS_MAIN_DEBUG|CSTREAMER_DBG_TRACE,("hit\n"));  
					strcpy(data,(char*)(l->data));
					CSTREAMER_DEBUGF(CS_MAIN_DEBUG|CSTREAMER_DBG_TRACE,("data==%s\n",data));
					xSemaphoreGive(nvram_the_list.nvram_mutex);
              		return NVRAM_SUCCESS;
              }
		}
		xSemaphoreGive(nvram_the_list.nvram_mutex);
        //not found pkgname and cfg_name
        if ((rc=snx_nvram_unsign_integer_get(pkg_name, cfg_name, data)) == NVRAM_SUCCESS)
		{
			add_nvram_node(&nvram_the_list.nvram_list,pkg_name,cfg_name,(void*)(data),DT_STRING,nvram_the_list.nvram_mutex);
		}
		else
		{
			CSTREAMER_DEBUGF(CS_MAIN_DEBUG|CSTREAMER_DBG_TRACE,("get nvram fail pkg_name=%s,cfg_name=%s,rc=%d\n",pkg_name,cfg_name));	
		}
		return rc;
	}
}





#if 0


void test_nvram_int()
{
    int width;
	int height;
	snx_nvram_integer_set_to_ddr(NVRAM_PKG_VIDEO_ISP, NVRAM_CFG_PREVIEWVIDEO_WIDTH, 1280);
	snx_nvram_integer_set_to_ddr(NVRAM_PKG_VIDEO_ISP, NVRAM_CFG_PREVIEWVIDEO_HEIGHT, 720);
	if (snx_nvram_integer_get_from_ddr(NVRAM_PKG_VIDEO_ISP, NVRAM_CFG_PREVIEWVIDEO_WIDTH, &width) != NVRAM_SUCCESS) 
	{

		CSTREAMER_DEBUGF(CS_MAIN_DEBUG|CSTREAMER_DBG_TRACE,("Get Preview Video WIDTH from NVRAM failed!"));

	}
	if (snx_nvram_integer_get_from_ddr(NVRAM_PKG_VIDEO_ISP, NVRAM_CFG_PREVIEWVIDEO_HEIGHT, &height) != NVRAM_SUCCESS) 
	{
       
       CSTREAMER_DEBUGF(CS_MAIN_DEBUG|CSTREAMER_DBG_TRACE,("Get Preview Video Height from NVRAM failed!"));
	}
	CSTREAMER_DEBUGF(CS_MAIN_DEBUG|CSTREAMER_DBG_TRACE,("first width==%d\n",width));
	CSTREAMER_DEBUGF(CS_MAIN_DEBUG|CSTREAMER_DBG_TRACE,("first height==%d\n",height));

	if (snx_nvram_integer_get_from_ddr(NVRAM_PKG_VIDEO_ISP, NVRAM_CFG_PREVIEWVIDEO_WIDTH, &width) != NVRAM_SUCCESS) 
	{

		CSTREAMER_DEBUGF(CS_MAIN_DEBUG|CSTREAMER_DBG_TRACE,("Get Preview Video WIDTH from NVRAM failed!"));

	}
	if (snx_nvram_integer_get_from_ddr(NVRAM_PKG_VIDEO_ISP, NVRAM_CFG_PREVIEWVIDEO_HEIGHT, &height) != NVRAM_SUCCESS) 
	{
       
       CSTREAMER_DEBUGF(CS_MAIN_DEBUG|CSTREAMER_DBG_TRACE,("4Get Preview Video Height from NVRAM failed!"));
	}
	CSTREAMER_DEBUGF(CS_MAIN_DEBUG|CSTREAMER_DBG_TRACE,("second width==%d\n",width));
	CSTREAMER_DEBUGF(CS_MAIN_DEBUG|CSTREAMER_DBG_TRACE,("second height==%d\n",height));

	
	snx_nvram_integer_set_to_ddr(NVRAM_PKG_VIDEO_ISP, NVRAM_CFG_PREVIEWVIDEO_WIDTH, 640);
	snx_nvram_integer_set_to_ddr(NVRAM_PKG_VIDEO_ISP, NVRAM_CFG_PREVIEWVIDEO_HEIGHT, 360);
	if (snx_nvram_integer_get_from_ddr(NVRAM_PKG_VIDEO_ISP, NVRAM_CFG_PREVIEWVIDEO_WIDTH, &width) != NVRAM_SUCCESS) 
	{  
       CSTREAMER_DEBUGF(CS_MAIN_DEBUG|CSTREAMER_DBG_TRACE,("Get Preview Video Height from NVRAM failed!"));
	}
	if (snx_nvram_integer_get_from_ddr(NVRAM_PKG_VIDEO_ISP, NVRAM_CFG_PREVIEWVIDEO_HEIGHT, &height) != NVRAM_SUCCESS) 
	{  
       CSTREAMER_DEBUGF(CS_MAIN_DEBUG|CSTREAMER_DBG_TRACE,("Get Preview Video Height from NVRAM failed!"));
	}
	CSTREAMER_DEBUGF(CS_MAIN_DEBUG|CSTREAMER_DBG_TRACE,("third width==%d\n",width));
	CSTREAMER_DEBUGF(CS_MAIN_DEBUG|CSTREAMER_DBG_TRACE,("third height==%d\n",height));
	vTaskDelete(NULL);
}


void test_nvram_unsign_int()
{
	unsigned int birate;
	snx_nvram_unsign_integer_set_to_ddr(NVRAM_PKG_VIDEO_ISP, NVRAM_CFG_RECORD_BPS, 10*1024*1024);
	if (snx_nvram_unsign_integer_get_from_ddr(NVRAM_PKG_VIDEO_ISP, NVRAM_CFG_RECORD_BPS, &birate) != NVRAM_SUCCESS) 
	{
	
		CSTREAMER_DEBUGF(CS_MAIN_DEBUG|CSTREAMER_DBG_TRACE,("Get Birate failed!"));
	
    }
	CSTREAMER_DEBUGF(CS_MAIN_DEBUG|CSTREAMER_DBG_TRACE,("first birate==%d\n",birate));
	
	if (snx_nvram_unsign_integer_get_from_ddr(NVRAM_PKG_VIDEO_ISP, NVRAM_CFG_RECORD_BPS, &birate) != NVRAM_SUCCESS) 
	{
	
		CSTREAMER_DEBUGF(CS_MAIN_DEBUG|CSTREAMER_DBG_TRACE,("Get Birate failed!"));
	
    }
	CSTREAMER_DEBUGF(CS_MAIN_DEBUG|CSTREAMER_DBG_TRACE,("second  birate==%d\n",birate));
	snx_nvram_unsign_integer_set_to_ddr(NVRAM_PKG_VIDEO_ISP, NVRAM_CFG_RECORD_BPS,5*1024*1024);
	if (snx_nvram_unsign_integer_get_from_ddr(NVRAM_PKG_VIDEO_ISP, NVRAM_CFG_RECORD_BPS, &birate) != NVRAM_SUCCESS) 
	{
	
		CSTREAMER_DEBUGF(CS_MAIN_DEBUG|CSTREAMER_DBG_TRACE,("Get Birate failed!"));
	
    }	
	CSTREAMER_DEBUGF(CS_MAIN_DEBUG|CSTREAMER_DBG_TRACE,("third  birate==%d\n",birate));
	vTaskDelete(NULL);
}




void test_nvram_string()
{
	char ssid[32] = {'\0'};
	if (snx_nvram_string_get_from_ddr("WIFI_DEV", "SSID_INFO", ssid) != NVRAM_SUCCESS) 
	{
		
			CSTREAMER_DEBUGF(CS_MAIN_DEBUG|CSTREAMER_DBG_TRACE,("Get Preview Video WIDTH from NVRAM failed!"));
		
	}	
	CSTREAMER_DEBUGF(CS_MAIN_DEBUG|CSTREAMER_DBG_TRACE,("first ssid==%s\n",ssid));
	if (snx_nvram_string_get_from_ddr("WIFI_DEV", "SSID_INFO", ssid) != NVRAM_SUCCESS) 
	{
		
			CSTREAMER_DEBUGF(CS_MAIN_DEBUG|CSTREAMER_DBG_TRACE,("Get Preview Video WIDTH from NVRAM failed!"));
		
	}	
	CSTREAMER_DEBUGF(CS_MAIN_DEBUG|CSTREAMER_DBG_TRACE,("second ssid==%s\n",ssid));

	snx_nvram_string_set_to_ddr("WIFI_DEV", "SSID_INFO","ALLENTEST_SSID");
	if (snx_nvram_string_get_from_ddr("WIFI_DEV", "SSID_INFO", ssid) != NVRAM_SUCCESS) 
	{
		
			CSTREAMER_DEBUGF(CS_MAIN_DEBUG|CSTREAMER_DBG_TRACE,("Get Preview Video WIDTH from NVRAM failed!"));
		
	}	
	CSTREAMER_DEBUGF(CS_MAIN_DEBUG|CSTREAMER_DBG_TRACE,("third ssid==%s\n",ssid));
	vTaskDelete(NULL);
}




void chk_nvram_task()
{

	nvram_list_init();
	if (pdPASS != xTaskCreate(test_nvram_int, "test_nvram_int", STACK_SIZE_6K, NULL,PRIORITY_TASK_APP_REC_FLOW, NULL))
	{
		CSTREAMER_DEBUGF(CS_RECORD_DEBUG | CSTREAMER_DBG_TRACE, ("chk_nvram_task task create fail\n"));
	}
	if (pdPASS != xTaskCreate(test_nvram_unsign_int, "test_nvram_unsign_int", STACK_SIZE_6K, NULL,PRIORITY_TASK_APP_REC_FLOW, NULL))
	{
		CSTREAMER_DEBUGF(CS_RECORD_DEBUG | CSTREAMER_DBG_TRACE, ("chk_nvram_task task create fail\n"));
	}
	if (pdPASS != xTaskCreate(test_nvram_string, "test_nvram_string", STACK_SIZE_6K, NULL,PRIORITY_TASK_APP_REC_FLOW, NULL))
	{
		CSTREAMER_DEBUGF(CS_RECORD_DEBUG | CSTREAMER_DBG_TRACE, ("chk_nvram_task task create fail\n"));
	} 
}



#endif
