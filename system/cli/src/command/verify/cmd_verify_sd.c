#include <FreeRTOS.h>
#include <bsp.h>
#include <task.h>
#include <cmd_verify.h>
#include <nonstdlib.h>
#include <libmid_sd/mid_sd.h>

/** \defgroup cmd_verify_sd SD verify commands
 *  \ingroup cmd_verify
 * @{
 */

/**
* @brief SD read/write test
* @param None
* @details Example:
*/
int cmd_verify_sd_rwtest(int argc, char* argv[])
{
	uint64_t sd_size;
	unsigned int cmp_size = 2048, cmp_start = 512;
	uint8_t *sd_buf1 = NULL, *sd_buf2 = NULL, *sd_backup = NULL;
	int i, ret;
	print_msg_queue("sd rwtest\n");

	ret = mid_sd_identify(MID_SD_BLOCK, NULL);
	if(ret != MID_SD_QUEUE_FINISH)
	{
		print_msg_queue("identify fail\n");
		goto fail;
	}

	ret = mid_sd_get_capacity(&sd_size, MID_SD_BLOCK, NULL);
	if(ret == MID_SD_QUEUE_FINISH)
	{
		print_msg_queue("sd_size = 0x%x%08x bytes\n",(uint32_t)(sd_size>>32),
			(uint32_t)sd_size);
	}


	//get original data in sd
	sd_backup = pvPortMalloc(cmp_size, GFP_DMA, MODULE_CLI);
	if(!sd_backup)
	{
		print_msg_queue("sd_backup allocate error\n");
		goto fail;
	}

	ret = mid_sd_read(sd_backup, cmp_start>>9,cmp_size>>9, MID_SD_BLOCK, NULL);
	print_msg_queue("read sd data %s\n", (ret==MID_SD_QUEUE_FINISH)?"success":"fail");
	if(ret != MID_SD_QUEUE_FINISH)
		goto fail;

	sd_buf1 = pvPortMalloc(cmp_size, GFP_DMA, MODULE_CLI);
	if(!sd_buf1)
	{
		print_msg_queue("buffer1 allocate error\n");
		goto fail;
	}

	sd_buf2 = pvPortMalloc(cmp_size, GFP_DMA, MODULE_CLI);
	if(!sd_buf2)
	{
		print_msg_queue("buffer2 allocate error\n");
		goto fail;
	}
	memset(sd_buf2, 0xff, cmp_size);

	ret = mid_sd_write(cmp_start>>9,sd_buf1,cmp_size>>9, MID_SD_BLOCK, NULL);
	print_msg_queue("write %s\n", (ret==MID_SD_QUEUE_FINISH)?"success":"fail");

	ret = mid_sd_read(sd_buf2, cmp_start>>9,cmp_size>>9, MID_SD_BLOCK, NULL);
	print_msg_queue("read %s\n", (ret==MID_SD_QUEUE_FINISH)?"success":"fail");

	for(i=0; i<cmp_size; i++ )
	{
		if(sd_buf1[i]!=sd_buf2[i])
		{
			print_msg_queue("data compare error (%x != %x) from %d\n", sd_buf1[i], sd_buf2[i], i);
			break;
		}
	}

	if(i==cmp_size)
		print_msg_queue("data compare success\n");



	ret = mid_sd_write(cmp_start>>9,sd_backup,cmp_size>>9, MID_SD_BLOCK, NULL);
	print_msg_queue("write back original sd data %s\n", (ret==MID_SD_QUEUE_FINISH)?"success":"fail");


	if(sd_backup)
		vPortFree(sd_backup);
	if(sd_buf1)
		vPortFree(sd_buf1);
	if(sd_buf2)
		vPortFree(sd_buf2);

	return 0;
fail:
	if(sd_backup)
		vPortFree(sd_backup);
	if(sd_buf1)
		vPortFree(sd_buf1);
	if(sd_buf2)
		vPortFree(sd_buf2);
	print_msg_queue("sd rwtest fail\n");
	return 0;
}
/** @} */
