#include <FreeRTOS.h>
#include <bsp.h>
#include <stdint.h>
#include <nonstdlib.h>
#include <string.h>
#include <libmid_sd/mid_sd.h>
#include <libmid_fatfs/ff.h>
#include <sys/time.h>
#include <task.h>
#include "cmd_sd.h"

int strtoint(char* str)
{
	int base = 10;
	int result = 0;
	char *endptr;
	if(str[0] == '0' && str[1] == 'x')
	{	
		base = 16;
		str +=2;
	}	
	endptr = str;
	while(*endptr)
	{
		if(*endptr>='0' && *endptr <='9')
			result += *endptr - '0';
		else if(base == 16 && *endptr >='a' && *endptr<='f')
			result += *endptr - 'a' + 10;
		result *= base; 
		endptr++;
	}	
	result /= base;
	return result;
	//return (int)strtol(str,&endptr,base);


}	

int cmd_sd_read(int argc, char* argv[])
{
	mid_sd_queue_status_t q_status=0;
	uint8_t *buf;
	int i, addr;
	buf = pvPortMalloc(512, GFP_DMA, MODULE_CLI);
	if(!buf || argc !=2)
		return 0;
	
	q_status = 0;
	addr = strtoint(argv[1]);
	print_msg_queue("addr = %d\n", addr);
	if(addr>0x1b0b800)
	{
		print_msg_queue("fail:addr = 0x%lx\n", addr);
		return 0;
	}	
	mid_sd_read(buf, addr,1,MID_SD_BLOCK,&q_status);
	if(q_status == MID_SD_QUEUE_FINISH)
	{
		print_msg_queue("    ");
		for(i=0;i<16;i++)
			print_msg_queue("%02x ", i);
		print_msg_queue("\n");
		for(i=0;i<512;i++)
		{
			if((i&0xf)==0)
				print_msg_queue("\n%02x  ", i>>4);		
			print_msg_queue("%02x ", buf[i]);	

		}
		print_msg_queue("\n");
	}
	else
		print_msg_queue("fail:status = %d\n", q_status);



	vPortFree(buf);
	return 0;
}

void show_rwtest_result(unsigned long long speed, unsigned long long time, char *title)
{
	unsigned long deci;
	char *unit;

	if (speed > (1024*1024)) {
		speed *= 100;
		speed /= (1024*1024);
		deci = speed % 100;
		speed /= 100;
		unit ="MB/s";
		print_msg_queue("%sspeed: %lu.%lu %s, cost_time=%lu us\n",title, (unsigned long)speed,deci,unit,(unsigned long)time);
	}
	else if (speed > 1024) {
		speed /= 1024;	unit ="KB/s";
		print_msg_queue("%sspeed: %lu% %s, cost_time=%lu us\n",title, (unsigned long)speed,unit,(unsigned long)time);
	}
	else {
		unit ="B/s";
		print_msg_queue("%swrite speed: %lu% %s, cost_time=%lu us\n",title, (unsigned long)speed,unit,(unsigned long)time);
	}
}

/**
* @brief SD read/write test
* @param None
* @details Example:
*/

void sd_rwtest(void *pvParameters)
{
	uint64_t sd_size;
	unsigned int cmp_size = 1024*1024, cmp_start = 512, block_cnt;
	uint8_t *sd_buf1 = NULL, *sd_buf2 = NULL, *sd_backup = NULL;
	int i, ret, ret1;
	char *unit;
	struct timeval start_tt, end_tt;
	unsigned long long speed, cost_time;
	int test_cnt = 1, cnt, bs;
	unsigned long long total_speed, total_time_read=0, total_time_write=0;
	unsigned long long max_write_speed=0, min_write_speed=-1, max_write_time=0, min_write_time=-1;
	unsigned long long max_read_speed=0, min_read_speed=-1, max_read_time=0, min_read_time=-1;
	uint32_t wbytes=0,rbytes=0;
	FRESULT res;
	FIL rFile,wFile;

	char **arg = (char**)pvParameters;
	block_cnt = strtoint(arg[1]);
	test_cnt = strtoint(arg[2]);

	ret = mid_sd_identify(MID_SD_BLOCK, NULL);
	if(ret != MID_SD_QUEUE_FINISH)
	{
		print_msg_queue("identify fail\n");
		ret = pdFAIL;
		goto fail;
	}

	ret = mid_sd_get_capacity(&sd_size , MID_SD_BLOCK, NULL);
	if(ret == MID_SD_QUEUE_FINISH)
	{
		print_msg_queue("sd_size = %d MBytes\n", sd_size / 2048);
	}

	print_msg_queue("rwtest size = 0x%x bytes\n",	(uint32_t)cmp_size);

	ret = fs_cmd_mount(0);
	if(ret != FR_OK)
	{
		print_msg_queue("mount sd fail(%d)\n", ret);
		ret = pdFAIL;
		goto fail;
	}

	sd_buf1 = pvPortMalloc(cmp_size, GFP_DMA, MODULE_CLI);
	if(!sd_buf1)
	{
		print_msg_queue("buffer1 allocate error\n");
		ret = pdFAIL;
		goto fail;
	}

	sd_buf2 = pvPortMalloc(cmp_size, GFP_DMA, MODULE_CLI);
	if(!sd_buf2)
	{
		print_msg_queue("buffer2 allocate error\n");
		ret = pdFAIL;
		goto fail;
	}
	memset(sd_buf2, 0xff, cmp_size);

	for (cnt=0; cnt < test_cnt; cnt++)
	{
		f_open(&wFile, ".rwtestfile", FA_CREATE_ALWAYS | FA_WRITE);
		gettimeofday(&start_tt, NULL);
		for (bs=0;bs<block_cnt;bs++)
			res = f_write(&wFile, sd_buf1, cmp_size, (void *)&wbytes);
		f_sync(&wFile);
		gettimeofday(&end_tt, NULL);
		if (end_tt.tv_usec < start_tt.tv_usec)
			cost_time = (end_tt.tv_sec - 1 - start_tt.tv_sec)*1000000 + (end_tt.tv_usec +1000000 - start_tt.tv_usec);
		else
			cost_time = (end_tt.tv_sec - start_tt.tv_sec)*1000000 + (end_tt.tv_usec - start_tt.tv_usec);
		total_time_write += cost_time;
		speed = cmp_size * block_cnt; //cast first
		speed = speed * 1000000 / cost_time;
		if (speed > max_write_speed) max_write_speed = speed;
		if (speed < min_write_speed) min_write_speed = speed;
		if (cost_time > max_write_time) max_write_time = cost_time;
		if (cost_time < min_write_time) min_write_time = cost_time;
		print_msg_queue("Write %s", (res==FR_OK)?"success. ":"fail\n");
		if (res == FR_OK)
			show_rwtest_result(speed,cost_time,"");
		f_close(&wFile);

		f_open(&rFile, ".rwtestfile", FA_OPEN_EXISTING | FA_READ);
		gettimeofday(&start_tt, NULL);
		for (bs=0;bs<block_cnt;bs++)
			f_read(&rFile, sd_buf2, cmp_size, &rbytes);
		gettimeofday(&end_tt, NULL);
		if (end_tt.tv_usec < start_tt.tv_usec)
			cost_time = (end_tt.tv_sec - 1 - start_tt.tv_sec)*1000000 + (end_tt.tv_usec +1000000 - start_tt.tv_usec);
		else
			cost_time = (end_tt.tv_sec - start_tt.tv_sec)*1000000 + (end_tt.tv_usec - start_tt.tv_usec);
		total_time_read += cost_time;
		speed = cmp_size * block_cnt; //cast first
		speed = speed * 1000000 / cost_time;
		if (speed > max_read_speed) max_read_speed = speed;
		if (speed < min_read_speed) min_read_speed = speed;
		if (cost_time > max_read_time) max_read_time = cost_time;
		if (cost_time < min_read_time) min_read_time = cost_time;
		print_msg_queue("Read %s", (res==FR_OK)?"success. ":"fail\n");
		if (res == FR_OK)
			show_rwtest_result(speed,cost_time,"");
		f_close(&rFile);

		f_open(&rFile, ".rwtestfile", FA_OPEN_EXISTING | FA_READ);
		for (bs=0;bs<block_cnt;bs++)
		{
			f_read(&rFile, sd_buf2, cmp_size, &rbytes);
			for(i=0; i<cmp_size; i++ )
			{
				if(sd_buf1[i]!=sd_buf2[i])
				{
					print_msg_queue("data compare error (%x != %x) from %d\n", sd_buf1[i], sd_buf2[i], i);
					ret = pdFAIL;
					goto fail;
				}
			}
		}
		f_close(&rFile);

		if(i==cmp_size)
			print_msg_queue("data compare success\n");
		f_unlink(".rwtestfile");
	}

	if (test_cnt > 1)
	{
		print_msg_queue("\n");
		//Avg write
		total_speed = test_cnt * cmp_size * block_cnt;
		total_speed = total_speed * 1000000 / total_time_write;
		total_time_write /= test_cnt;
		show_rwtest_result(max_write_speed,min_write_time,"Maximum write ");
		show_rwtest_result(min_write_speed,max_write_time,"Minimum write ");
		show_rwtest_result(total_speed,total_time_write,"Average write ");

		//Avg read
		total_speed = test_cnt * cmp_size * block_cnt;
		total_speed = total_speed * 1000000 / total_time_read;
		total_time_read /= test_cnt;

		show_rwtest_result(max_read_speed,min_read_time,"Maximum read ");
		show_rwtest_result(min_read_speed,max_read_time,"Minimum read ");
		show_rwtest_result(total_speed,total_time_read,"Average read ");
	}
/*
	f_close(&wFile);
	f_close(&rFile);
	f_unlink(".rwtestfile");
	if(sd_buf1) { vPortFree(sd_buf1); sd_buf1=NULL; }
	if(sd_buf2) { vPortFree(sd_buf2); sd_buf2=NULL; }
	ret = fs_cmd_umount(0);
	if(ret != FR_OK)
		print_msg_queue("umount fail(%d)\n", ret);
*/	
//	vTaskDelete(NULL);
	ret = pdPASS;
fail:
	f_close(&wFile);
	f_close(&rFile);
	f_unlink(".rwtestfile");
	if(sd_buf1) { vPortFree(sd_buf1); sd_buf1=NULL; }
	if(sd_buf2) { vPortFree(sd_buf2); sd_buf2=NULL; }
	ret1 = fs_cmd_umount(0);
	if(ret1 != FR_OK)
		print_msg_queue("umount fail(%d)\n", ret1);

	if( ret == pdFAIL) {
		print_msg_queue("sd rwtest fail\n");
	}
	else {
		print_msg_queue("sd rwtest pass\n");
	}


//	vTaskDelete(NULL);
}	

int cmd_sd_rwtest(int argc, char* argv[])
{
	if (argc != 3)
	{
		print_msg_queue("Usage:\n rwtest bs_count loops\n\n");
		return 0;
	}
	else
	{
		print_msg_queue("cmd >> rwtest %s %s\n",argv[1],argv[2]);
		
		sd_rwtest((void*)argv);
/*
	    if (pdPASS != xTaskCreate(sd_rwtest, "sd rwtest task", STACK_SIZE_4K  , (void*)argv,  80, NULL))
	    {
	        print_msg_queue("Could not create sd_rwtest task\r\n");
	        return pdFAIL;
	    }
	    else
	        return pdPASS;
*/
	}
}

