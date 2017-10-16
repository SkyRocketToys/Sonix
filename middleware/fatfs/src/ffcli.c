/**
 * @file
 * this is fatfs command line interface file ffcli.c  
 * @author Algorithm Dept Sonix. (yiling porting to RTOS)
 */


#include <FreeRTOS.h>
#include <bsp.h>
#include <nonstdlib.h>
#include <string.h>
#include <stdint.h>
#include <sglib.h>
#include "ff.h"		

#include <generated/snx_sdk_conf.h>

FATFS *pFatWorkSpace[_VOLUMES]={0};	

FATFS *pFatWorkSpace_t[0]={0};

extern FRESULT sync_fs (FATFS* fs);
extern volatile int fatfs_res;

extern volatile uint32_t disable_diskio_t;

#define NAME_COMPARATOR(e1, e2) fs_name_cmp(e1->finfo, e2->finfo)
#define TIME_COMPARATOR(e1, e2) fs_rtime_cmp(e1->finfo, e2->finfo)

void fs_add_path(char *path, char *item)
{
	int pathlen = strlen(path);
	if((pathlen && item[0] == '/') || (pathlen>=2 && item[1] == ':'))	//absolute path
		strcpy(path, item);
	else if(strcmp(item, ".")!=0)
	{
		strcat(path, "/");
		strcat(path, item);
	}
}

void fs_drive_num_to_path(char *path, uint8_t drive_num)
{
	if(drive_num>10)
	{
		print_msg_queue("%s: drive number(%d) overflow\n",__func__, drive_num);
	}	
	path[0] = (drive_num%10) + '0';
	strcpy(path+1, ":");
}


/**
* @brief interface function - mount a filesystem and create a working area
* @param drive_num 0:SD card
* @return return FR_OK if success or other fail number in struct FRESULT
*/
int fs_cmd_mount(BYTE drive_num)
{
	FRESULT ret;
	char path[5]={0};

	disable_diskio_t = 0;

	if(drive_num>_VOLUMES)
	{	
		print_msg_queue("drive number(%d) must less than %d\n",drive_num,_VOLUMES);
		return FR_INVALID_DRIVE;
	}

	if(pFatWorkSpace[drive_num])
	{	
		//vPortFree(pFatWorkSpace[drive_num]);
		print_msg_queue("drive number(%d) has already mounted\n",drive_num);
		return FR_OK;
	}	
	
	if(PhyDrvNum==REC_DEV_USBH){
		pFatWorkSpace_t[0] = pvPortMalloc((sizeof(FATFS)+512), GFP_KERNEL, MODULE_MID_FATFS);
		pFatWorkSpace[drive_num] = ( (((unsigned int) pFatWorkSpace_t[0]) + 512) &(0xFFFFFE00));
	}
	else
		pFatWorkSpace[drive_num] = pvPortMalloc(sizeof(FATFS), GFP_KERNEL, MODULE_MID_FATFS);



	if(!pFatWorkSpace[drive_num])
		return FR_INT_ERR;
	memset(pFatWorkSpace[drive_num], 0, sizeof(FATFS) );

	
	//mount workspace
	fs_drive_num_to_path(path, drive_num);
	ret = f_mount(pFatWorkSpace[drive_num],path ,1);
	
	if(ret == FR_OK)
	{
		fatfs_res = 0;
		print_msg_queue("mount %d success\n",drive_num);
	}
	else
	{
		/*if sd fast remove error, these free action will be done by fs_cmd_umount*/
		// vPortFree(pFatWorkSpace[drive_num]);
		// pFatWorkSpace[drive_num] = NULL;
	}	
	return ret;

}


/**
* @brief interface function - umount a filesystem and release a working area
* @param drive_num 0:SD card
* @return return FR_OK if success or other fail number in struct FRESULT
*/
int fs_cmd_umount(BYTE drive_num)
{
	char path[5]={0};
	int ret;
	if(drive_num>_VOLUMES)
	{	
		print_msg_queue("drive number(%d) must less than %d\n",drive_num,_VOLUMES);
		return FR_INVALID_DRIVE;
	}

	if(!pFatWorkSpace[drive_num])
	{	
		print_msg_queue("drive number(%d) has umount %d\n",drive_num);
		return FR_OK;
	}
	sync_fs(pFatWorkSpace[drive_num]);
	
	fs_drive_num_to_path(path, drive_num);
	ret = f_mount(NULL,path,0);
	print_msg_queue("unmount %d success\n",drive_num);
	
	if(pFatWorkSpace[drive_num])
	{		
		if(PhyDrvNum==REC_DEV_USBH){
			vPortFree(pFatWorkSpace_t[0]);
			pFatWorkSpace[drive_num] = NULL;		
		}
		else{
			vPortFree(pFatWorkSpace[drive_num]);
			pFatWorkSpace[drive_num] = NULL;
		}
	}	
	pFatWorkSpace[drive_num] = 0;
	return ret;
}

void fs_strlwr(char *str, char *str_lw)
{
	char w, index = 0;
	while((w = *(str+index)))
	{
		if(w>=0x41 && w<=0x5a)	//A-Z	
			w+=0x20;
		*(str_lw+index) = w;
		index ++;
	}
	*(str_lw+index) = *(str+index);

}
int fs_cmp_name(char* fn1, char* fn2)
{
	char *fn_lw1, *fn_lw2;	/* This function assumes non-Unicode configuration */
	int rst=0;
	fn_lw1 = pvPortMalloc(strlen(fn1)+1, GFP_KERNEL, MODULE_MID_FATFS);
	if(!fn_lw1)
	{	
		print_msg_queue("alloc fn_lw1 fail\n");
		return 0;
	}
	fn_lw2 = pvPortMalloc(strlen(fn2)+1, GFP_KERNEL, MODULE_MID_FATFS);
	if(!fn_lw2)
	{	
		print_msg_queue("alloc fn_lw2 fail\n");
		vPortFree(fn_lw1);
		return 0;
	}
	fs_strlwr(fn1, fn_lw1);
	fs_strlwr(fn2, fn_lw2);
	rst = strcmp(fn_lw1,fn_lw2);
	vPortFree(fn_lw2);
	vPortFree(fn_lw1);
	return rst;
}

int fs_name_cmp(FILINFO finfo1, FILINFO finfo2)
{
	return fs_cmp_name(GET_FN(finfo1), GET_FN(finfo2));
}

int fs_rtime_cmp(FILINFO finfo1, FILINFO finfo2)
{
	int result;
	//compare last modified date
	result = finfo1.fdate-finfo2.fdate;
	if(result)
		return result;
	//compare last modified time	
	result = finfo1.ftime-finfo2.ftime;
	if(result)
	{
		return result;
	}	
	else
	{
		//compare file name
		return fs_cmp_name(GET_FN(finfo1), GET_FN(finfo2));
	}	

}

static FILLIST* create_flist_item(void)
{
	FILLIST *pfitem = NULL;  
	if((pfitem = pvPortMalloc(sizeof(FILLIST), GFP_KERNEL, MODULE_MID_FATFS)) == NULL)
		return pfitem;
	memset(pfitem, 0, sizeof(FILLIST));
	pfitem->finfo.lfname = pfitem->lfname;
	pfitem->finfo.lfsize = sizeof(pfitem->lfname);
	return pfitem;
}

int fs_cmd_ls(FILLIST **finfo, int option)
{	

	FRESULT ret;
	char cur_path[100] = {0};
	
	*finfo = NULL;
	if((ret = f_getcwd(cur_path,100))!=FR_OK)
	{
		goto fail0;
	}

	return fs_cmd_ls_by_path(cur_path, finfo, option);
fail0:
	return ret;
	
}



int fs_cmd_ls_by_path(char* path, FILLIST **finfo, int option)
{	

	FRESULT ret;
	DIR dir;
	FILLIST* pitem;
	FILINFO *pfno;
	//int len=0;

	*finfo = NULL;

	if((ret = f_opendir(&dir, path))!=FR_OK)					   /* Open the directory */
	{
		goto fail0;
	}

	if((pitem = create_flist_item())==NULL)
	{
		ret = FR_NOT_ENOUGH_BUF;
		goto fail0;
	}

	while((ret = f_readdir(&dir, &pitem->finfo))==FR_OK)
	{
		pfno = &pitem->finfo;
		if (ret != FR_OK || pfno->fname[0] == 0) break;  /* Break on error or end of dir */
		if (pfno->fname[0] == '.') continue;			   /* Ignore dot entry */
#if 0		
		print_msg_queue("%c %10d %4d-%2d-%2d %2d:%2d %s\n", (pfno->fattrib&AM_DIR)?'d':'-', pfno->fsize, 
						FF_YEAR(pfno->fdate), FF_MONTH(pfno->fdate), FF_DATE(pfno->fdate),
						FF_HOUR(pfno->ftime), FF_MINUTE(pfno->ftime), GET_FN(pitem->finfo));
#endif		
		SGLIB_LIST_ADD(FILLIST, *finfo, pitem, next);

		//create next item
		if((pitem = create_flist_item())==NULL)
		{
			ret = FR_NOT_ENOUGH_BUF;
			goto fail1;
		}
	}
	//vPortFree(pitem);	//free space for fail item	


	//SGLIB_LIST_LEN(FILLIST, *finfo, next, len);
	//print_msg_queue("%s:total_item = %d\n",__func__,  len);
	
	fs_cmd_flist_sort(finfo, option);


fail1:		
	f_closedir(&dir);
fail0:
	return ret;
	
}

void fs_cmd_flist_sort(FILLIST **pflist, int option)
{
	if(option == FF_SORT_BY_NAME)
	{
		SGLIB_LIST_SORT(FILLIST, *pflist, NAME_COMPARATOR, next); 
	}	
	else
	{
		SGLIB_LIST_SORT(FILLIST, *pflist, TIME_COMPARATOR, next); 
	}
}

void fs_cmd_ls_clear(FILLIST **pflist)
{
	FILLIST *pItem, *pNextItem;
	//int len = 0;
	if(*pflist==NULL)
		return;
	
	pItem = *pflist;
	while(pItem)
	{
		pNextItem = pItem->next;
		vPortFree(pItem);
		//len ++;
		pItem = pNextItem;
	}
	vPortFree(pflist);
	pflist = NULL;
	//print_msg_queue("%s:total_item = %d\n",__func__,  len);
	
}


/**
* @brief interface function - retrieves name of current working directory(similar to f_getcwd())
* @param path Pointer to the buffer to receive directory string
* @param len The length of the buffer 
* @return return FR_OK if success or other fail number in struct FRESULT
*/
int fs_cmd_pwd(char *path, int len)
{
	return f_getcwd(path,len);
}


/**
* @brief interface function - Change current directory(similar to f_chdir())
* @param path Pointer to string that specifies a directory to go
* @return return FR_OK if success or other fail number in struct FRESULT
*/
int fs_cmd_cd(char* path)
{
	FRESULT ret;
	char cur_path[100] = {0};
	print_msg_queue("path =  %s\n", path);
	ret = f_chdir(path);
	if(ret==FR_OK)
	{	
		f_getcwd(cur_path,100);
		print_msg_queue("move to %s\n", cur_path);
	}
	return ret;	
}

/**
* @brief interface function - creates a new directory(similar to f_mkdir())
* @param path Pointer to string that specifies the directory name to create 
* @return return FR_OK if success or other fail number in struct FRESULT
*/
int fs_cmd_mkdir(char* path)
{
	return f_mkdir(path);
}	

int fs_rm(char* rm_path)
{
	FRESULT ret;
	FILINFO fno;
	char *fn;
	char lfn[_MAX_LFN + 1];
	fno.lfname = lfn;
	fno.lfsize = sizeof(lfn);
	
	ret = f_stat(rm_path, &fno);
	if(ret == FR_OK)
	{
		//print_msg_queue("%s is %s\n", rm_path, fno.fattrib&AM_DIR?"dir":"file");
		if(fno.fattrib&AM_DIR)
		{
			DIR dir;
			ret = f_opendir(&dir, rm_path);
			//remove all item in this directory
			while(FR_OK == ret)
			{
				ret = f_readdir(&dir, &fno);                   /* Read a directory item */
				if (ret != FR_OK || fno.fname[0] == 0) break;  /* Break on error or end of dir */
				if (fno.fname[0] == '.') continue;             /* Ignore dot entry */
				if(ret == FR_OK)
				{
					char next_path[200]={0};				
					fn = *fno.lfname ? fno.lfname : fno.fname;
					strcpy(next_path, rm_path);
					fs_add_path(next_path, fn);
					ret = fs_rm(next_path);	
					
				}
				else
				{
					print_msg_queue("read dir %s fail\n", fn);
				}
			}
			f_closedir(&dir);
			//remove current directory
			ret = f_unlink(rm_path);
			//print_msg_queue("remove %s dir finish\n", rm_path);
		}
		else
		{
			ret = f_unlink(rm_path);
			//print_msg_queue("remove %s file finish\n", rm_path);
		}
	}
	
	
	if(ret != FR_OK)
	{
		char fail_path[200]={0};	
		f_getcwd(fail_path,200);
		fs_add_path(fail_path,rm_path);
		print_msg_queue("remove %s fail(ret = %d)\n", fail_path, ret);
			
		
	}
	return ret;	

}


/**
* @brief interface function - remove file or directory
* @param rm_path Pointer to string that specifies the directory name to create 
* @return return FR_OK if success or other fail number in struct FRESULT
*/
int fs_cmd_rm(char* path)
{
	return fs_rm(path);
}

int fs_du(char* path, DWORD *total_size)	//csize is kbyte unit
{
	FRESULT ret;
	FILINFO fno;
	char *fn;
	char lfn[_MAX_LFN + 1];	 /* Buffer to store the LFN */
	DWORD item_clst,csize;
	
	*total_size = 0;
	
	fno.lfname = lfn;
	fno.lfsize = sizeof(lfn);
	

	ret = f_stat(path, &fno);
	
	
	if(ret == FR_IS_ROOT_DIR && strcmp(path, ".")==0)
	{
		//is root path
		ret = FR_OK;
		fno.fattrib |= AM_DIR;	
	}
	
	if(ret==FR_OK)
	{
		//print_msg_queue("%s is %s\n", path, fno.fattrib&AM_DIR?"dir":"file");
		if(fno.fattrib&AM_DIR)
		{
			DIR dir;
			
			ret = f_opendir(&dir, path);
			//remove all item in this directory
			while(FR_OK == ret)
			{
				DWORD item_size = 0;
				ret = f_readdir(&dir, &fno);                   /* Read a directory item */
				//print_msg_queue("ret2 = %d\n", ret);
				if (ret != FR_OK || fno.fname[0] == 0) break;  /* Break on error or end of dir */
				if (fno.fname[0] == '.') continue;             /* Ignore dot entry */
				if(ret == FR_OK)
				{
					char next_path[200]={0};	
					fn = *fno.lfname ? fno.lfname : fno.fname;
					strcpy(next_path, path);
					fs_add_path(next_path, fn);
					ret = fs_du(next_path, &item_size);
					if(ret == FR_OK)
					{
						*total_size += item_size; 
						//print_msg_queue("%s size = %d(%d)\n", fn, item_clst*(size>>10), *csize);
					}
					//else
					//	print_msg_queue("err = %d\n", ret);
				}
				else
				{
					print_msg_queue("read dir fail\n");
				}
			}
			f_closedir(&dir);
			//get directory clst size
			ret = f_getcusage(path, &item_clst, &csize);
			if(ret == FR_OK)
				*total_size += item_clst*(csize>>10); 
			print_msg_queue("%-10d %s\n",*total_size, path);
		}
		else
		{
			ret = f_getcusage(path,&item_clst, &csize);
			if(ret == FR_OK)
				*total_size = item_clst*(csize>>10);
			//print_msg_queue("%-10d %s(file)\n",*total_size, path);
		}
	}
	
	if(ret != FR_OK)
	{
		char fail_path[200]={0};
		f_getcwd(fail_path,200);
		fs_add_path(fail_path,path);
		print_msg_queue("%s fail(ret = %d)\n", fail_path, ret);
		*total_size = 0;
	}	
	return ret;	

}

/**
* @brief interface function - estimate file space usage
* @param path Pointer to string that specifies the directory name to estimate
* @param psize Pointer to return total space usage(kbyte unit)
* @return return FR_OK if success or other fail number in struct FRESULT
*/
int fs_cmd_du(char* path, unsigned long *psize)
{
	return fs_du(path, psize);
}



unsigned long long fs_cmd_ffdu (char* path)
{
    FRESULT res;
    FILINFO fno;
    DIR dir;
    unsigned long long totfiles_size = 0;

    static char lfn[255 + 1];   /* Buffer to store the LFN */
    fno.lfname = lfn;
    fno.lfsize = sizeof(lfn);


    res = f_opendir(&dir, path);

    if (res == FR_OK) {
        for (;;) {              
            res = f_readdir(&dir, &fno);

            if ((res != FR_OK)||(fno.fname[0] == 0)){
                break;
            }
            
            if (fno.fname[0] == '.') {
                continue;
            }

            totfiles_size += (fno.fsize);
            //print_msg("%s,  %10lu\n",fno.lfname,fno.fsize);
        } 
        f_closedir(&dir);
    }

    return  (totfiles_size >> 20);  
}


void fs_test(void)
{

#if 0		
	FATFS *pFatFs = pFatWorkSpace[0];	
	FIL fil;	   /* File object */
	//char line[82]; /* Line buffer */
	FRESULT fr;    /* FatFs return code */
	char *buffer;
	int i, w_size;
	
	print_msg_queue("***start fatfs copy***\n");
	
	/* Register work area to the default drive */
	if(fs_cmd_mount(0)!=FR_OK)
		return;

	fr = f_mkdir("test");
	/* Open a text file */

	fr = f_open(&fil, "0:test/msg.txt", FA_WRITE|FA_OPEN_ALWAYS);

	if (fr) 
	{
		print_msg_queue("file open error, fr = %d\n", fr);
		return;
	}
	buffer = pvPortMalloc(1024, GFP_KERNEL, MODULE_MID_FATFS);

	for(i=0;i<1024;i++)
		buffer[i] = '0'+ (i&0x7);

	fr = f_write(&fil, buffer, 1024, (UINT*)&w_size);			 /* Write it to the destination file */
	if (fr || w_size < 1024)
		print_msg_queue("write to file error\n");
	/* Close the file */
	f_close(&fil);
	fs_cmd_umount(0);

#endif


}	





