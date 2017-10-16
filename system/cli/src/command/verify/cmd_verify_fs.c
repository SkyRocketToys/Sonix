#include <FreeRTOS.h>
#include <bsp.h>
#include <task.h>
#include <cmd_verify.h>
#include <nonstdlib.h>
#include <libmid_fatfs/ff.h>

int cmd_verify_fs_rwtest(int argc, char* argv[])
{
	FIL file;
	uint8_t *buf1 = NULL, *buf2 = NULL;
	int cmp_size = 1024, ret, i;
	unsigned int wbytes;
	char filename[15] = "fs_rw_test";
	print_msg_queue("fs rwtest\n");
	if(FR_OK != fs_cmd_mount(0)
)
	{
		print_msg_queue("fs mount fail\n");
		goto fail1;
	}

	buf1 = pvPortMalloc(cmp_size, GFP_DMA, MODULE_CLI);
	if(!buf1)
	{
		print_msg_queue("buffer1 allocate error\n");
		goto fail1;
	}

	buf2 = pvPortMalloc(cmp_size, GFP_DMA, MODULE_CLI);
	if(!buf2)
	{
		print_msg_queue("buffer2 allocate error\n");
		goto fail1;
	}


	if((ret = f_open(&file, filename, FA_WRITE|FA_OPEN_ALWAYS)) != FR_OK)
	{
		print_msg_queue("file open fail(ret = %d)\n", ret);
		goto fail1;
	}

	if((ret = f_write(&file,buf1,cmp_size, &wbytes))!=FR_OK || cmp_size!=wbytes)
	{
		print_msg_queue("file write fail(ret = %d)\n", ret);
		goto fail2;
	}

	if(f_close(&file)!=FR_OK)
	{
		print_msg_queue("file close fail\n");
		goto fail1;
	}

	if((ret = f_open(&file, filename, FA_READ)) != FR_OK)
	{
		print_msg_queue("file open fail(ret = %d)\n", ret);
		goto fail1;
	}

	if((ret = f_read(&file,buf2,cmp_size, &wbytes))!=FR_OK || cmp_size!=wbytes)
	{
		print_msg_queue("file read fail(ret = %d)\n", ret);
		goto fail2;
	}
	if(f_close(&file)!=FR_OK)
	{
		print_msg_queue("file close fail\n");
		goto fail1;
	}


	for(i=0; i<cmp_size; i++ )
	{
		if(buf1[i]!=buf2[i])
		{
			print_msg_queue("data compare error (%x != %x) from %d\n", buf1[i], buf2[i], i);
			break;
		}
	}

	if(i==cmp_size)
		print_msg_queue("data compare success\n");




	//remove test file
	if(f_unlink(filename)!=FR_OK)
	{
		print_msg_queue("remove file fail\n");
		goto fail1;
	}
	if(buf1)
		vPortFree(buf1);
	if(buf2)
		vPortFree(buf2);
	return 0;

fail2:
	f_close(&file);
fail1:

	if(buf1)
		vPortFree(buf1);
	if(buf2)
		vPortFree(buf2);
	print_msg_queue("fs rwtest fail\n");
	return 0;

}
