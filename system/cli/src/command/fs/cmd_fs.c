//#include <stdio.h>
#include <FreeRTOS.h>
#include <task.h>
//#include <interrupt.h>
#include <bsp.h>
#include <nonstdlib.h>
#include <libmid_sd/mid_sd.h>
#include <libmid_fatfs/ff.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "cmd_fs.h"
#include "printlog.h"

/** \defgroup cmd_fs File system commands
 *  \ingroup system_cli
 * @{
 */


#define PRIORITY_TASK_CMD_FS 2

enum{
	CMD_FS_LS=0,
	CMD_FS_RM,
	CMD_FS_DU,
};

typedef struct cmd_fs_ls{
	uint8_t cmd;
	FILLIST **ppfinfo; 
	int option;
	int *pret;

}cmd_fs_ls_t;

typedef struct cmd_fs_path{
	uint8_t cmd;
	char *path; 
	unsigned long *psize;
	int *pret;
}cmd_fs_path_t;

uint8_t task_work = 0;
void task_fs_cmd(void *pvParameters)
{
	
	uint8_t *cmd =(uint8_t *) pvParameters;	
	switch(*cmd)
	{
		case CMD_FS_LS:
		{
			cmd_fs_ls_t *ls_parm = (cmd_fs_ls_t *)pvParameters;
			*ls_parm->pret = fs_cmd_ls(ls_parm->ppfinfo, ls_parm->option);
			break;
		}	
		case CMD_FS_RM:	
		{
			cmd_fs_path_t *rm_parm = (cmd_fs_path_t *)pvParameters;
			*rm_parm->pret= fs_cmd_rm(rm_parm->path);
			break;
		}	
		case CMD_FS_DU:
		{
			cmd_fs_path_t *du_parm = (cmd_fs_path_t *)pvParameters;
			*du_parm->pret= fs_cmd_du(du_parm->path, du_parm->psize);
			break;
		}	
	}
	task_work = 0;
	vTaskDelete(NULL);
	//set flag
}


int cmd_fs_mount(int argc, char* argv[])
{
	int ret;
	if(argc==2)
	{
		ret = fs_cmd_mount(atoi(argv[1]));
		if(ret != FR_OK)
			print_msg_queue("%s fail(%d)\n", __func__, ret);
	}	
	return 0;
}

int cmd_fs_umount(int argc, char* argv[])
{
	int ret;
	if(argc==2)
	{
		ret = fs_cmd_umount(atoi(argv[1]));
		if(ret != FR_OK)
			print_msg_queue("%s fail(%d)\n", __func__, ret);
	}
	return 0;
}

cmd_fs_ls_t ls_parm;
int cmd_fs_ls(int argc, char* argv[])
{
	int ret;
	FILLIST *pfitem;
	int option = FF_SORT_BY_NAME;
	if(argc==2 && strcmp(argv[1],"-rt")==0)
	{
		option = FF_SORT_BY_RTIME;
	}	
	ls_parm.cmd = CMD_FS_LS;
	ls_parm.option = option;
	ls_parm.pret= &ret;
	if((ls_parm.ppfinfo=(FILLIST**)pvPortMalloc(sizeof(FILLIST*), GFP_KERNEL, MODULE_MID_FATFS))==NULL) //this **ppfinfo address is =0 ,so add malloc  to avoid 
	 return 0;
	task_work = 1;
	if (pdPASS != xTaskCreate(task_fs_cmd, "test_fs_cmd", STACK_SIZE_2K, &ls_parm, 
				PRIORITY_TASK_CMD_FS, NULL))
	{
		print_msg("task create fail\n");
		return 0;
	}
	//wait for task finish
	while(task_work) 
		vTaskDelay( 10 / portTICK_RATE_MS );

	if(*ls_parm.pret == FR_OK && (*ls_parm.ppfinfo)!=NULL)
	{
		pfitem = *ls_parm.ppfinfo;
		while(pfitem)	
		{
			FILINFO *pfno = &pfitem->finfo;
			print_msg_queue("%c %10d %4d-%02d-%02d %2d:%02d %s\n", (pfno->fattrib&AM_DIR)?'d':'-', pfno->fsize, 
					FF_YEAR(pfno->fdate), FF_MONTH(pfno->fdate), FF_DATE(pfno->fdate),
					FF_HOUR(pfno->ftime), FF_MINUTE(pfno->ftime), GET_FN(pfitem->finfo));	
			pfitem = pfitem->next;
		}	
	}
	fs_cmd_ls_clear(ls_parm.ppfinfo);
	ls_parm.ppfinfo=NULL;
	if(ret != FR_OK)
		print_msg_queue("%s fail(%d)\n", __func__, ret);
	
	return 0;
}

int cmd_fs_pwd(int argc, char* argv[])
{
	int ret;
	char path[100] = {0};
	ret = fs_cmd_pwd(path, 100);
	if(ret ==FR_OK)
		print_msg_queue("%s\n", path);
	else
		print_msg_queue("%s fail(%d)\n", __func__, ret);
	return 0;
}

int cmd_fs_cd(int argc, char* argv[])
{
	int ret;
	if(argc==2)
	{
		ret = fs_cmd_cd(argv[1]);
		if(ret != FR_OK)
			print_msg_queue("%s fail(%d)\n", __func__, ret);
		
	}	
	return 0;
}

int cmd_fs_mkdir(int argc, char* argv[])
{
	int ret;
	if(argc==2)
	{
		ret = fs_cmd_mkdir(argv[1]);
		if(ret != FR_OK)
			print_msg_queue("make directory %s fail \n", argv[1]);
		
	}	
	return 0;
}

cmd_fs_path_t rm_parm;
int cmd_fs_rm(int argc, char* argv[])
{
	int ret;

	if(argc==2)
	{
		//ret = fs_cmd_rm(argv[1]);
		
		//***creat task to avoid statck overflow***
		rm_parm.cmd = CMD_FS_RM;
		rm_parm.path = argv[1];
		rm_parm.pret= &ret;
		while(task_work) 
			vTaskDelay( 10 / portTICK_RATE_MS );
		task_work = 1;
		if (pdPASS != xTaskCreate(task_fs_cmd, "tast_fs_cmd", STACK_SIZE_6K, &rm_parm, 
						PRIORITY_TASK_CMD_FS, NULL))
		{
			print_msg("task create fail\n");
			return 0;
		}
		while(task_work) 
			vTaskDelay( 10 / portTICK_RATE_MS );
		//***end of creat task ***
		
		
		if(ret != FR_OK)
		{
			print_msg_queue("remove directory %s fail(%d) \n", argv[1], ret);
		}
	}	
	return 0;
}

cmd_fs_path_t du_parm;
int cmd_fs_du(int argc, char* argv[])
{
	int ret;
	unsigned long csize;

	//if(argc==2)
	//	ret = fs_cmd_du(argv[1]);
	//else
	//	ret = fs_cmd_du(".");
	
	//***creat task to avoid statck overflow***
	du_parm.cmd = CMD_FS_DU;
	du_parm.path = (argc==2)?argv[1]:".";
	du_parm.psize = &csize;
	du_parm.pret= &ret;
	while(task_work) 
		vTaskDelay( 10 / portTICK_RATE_MS );
	task_work = 1;
	if (pdPASS != xTaskCreate(task_fs_cmd, "tast_fs_cmd", STACK_SIZE_6K, &du_parm, 
			PRIORITY_TASK_CMD_FS, NULL))
	{
		print_msg("task create fail\n");
		return 0;
	}
	while(task_work) 
		vTaskDelay( 10 / portTICK_RATE_MS );
	//***end of creat task ***	
		
	if(ret == FR_OK)
	{
		print_msg_queue("space usage is %d KB\n", csize);
	}
	else
	{
		print_msg_queue("remove directory fail(%d) \n", ret);
	}
		
	return 0;
}

int cmd_fs_write_file(int argc, char* argv[])
{
	FIL file;
	void *pbuf;
	char * filename, *endptr;
	uint32_t bufsize = 0, uiWritedSize;
	int ret;
	filename = argv[1];
	pbuf = (void *)simple_strtol(argv[2], &endptr, 16);
	bufsize = simple_strtol(argv[3], &endptr, 16);
	print_msg_queue("write file = %s , buf = %x, size = %x\n", filename, pbuf, bufsize);
	if((ret=f_open(&file, filename ,  FA_WRITE|FA_OPEN_ALWAYS))!=FR_OK)
	{
		print_msg_queue("file %s open fail(ret = %d)\n", filename, ret);
		return 0;
	}
	if((int)pbuf&0x3)
		print_msg_queue("pbuf(%x) is not 4 byte align\n", pbuf);
	
	//rec_cmd.pFrame must be allocated use GFP_DMA parameter
	if((ret = f_write(&file, pbuf, bufsize,&uiWritedSize))!=FR_OK || (bufsize!=uiWritedSize))
	{
		print_msg_queue("write data fail(ret = %d), %x, %x\n",ret, bufsize,uiWritedSize);

	}

	f_close(&file);
	return 0;
}

int cmd_fs_format(int argc, char* argv[])
{
	int ret;
	char path[10];
	if(argc!=2)
	{
		print_msg_queue("format [drive_num=0]\n");
		return 0;
	}	
	sprintf(path, "%s:", argv[1]);
	print_msg_queue("path = %s\n", path);	
	if((ret = f_mkfs(path, 1, 0))!=FR_OK)
		print_msg_queue("format fail(ret = %d)\n", ret);
	else
		print_msg_queue("format success\n");
	return 0;

}
/** @} */
