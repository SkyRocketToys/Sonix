#include <FreeRTOS.h>
#include <bsp.h>
#include <task.h>
#include <cmd_verify.h>
#include <nonstdlib.h>
#include <libmid_sf/mid_sf.h>
/** \defgroup cmd_verify_sf SF verify commands
 *  \ingroup cmd_verify
 * @{
 */
/**
* @brief Serial Flash read/write test
* @param None
* @details Example:
*/
int cmd_verify_sf_rwtest(int argc, char* argv[])
{
	unsigned int cmp_size, cmp_start = 0;
	uint8_t *sf_buf1=NULL, *sf_backup=NULL;
	int i, ret;
	mid_sf_capacity_t sf_cap;

	print_msg_queue("sf rwtest\n");

	//get capacity
	if(MID_SF_QUEUE_FINISH == mid_sf_capacity(&sf_cap))
	{
		print_msg_queue("sf(%s) chip size = %d, sector size = %d \n", sf_cap.name,
				sf_cap.chip_size, sf_cap.sector_size);
	}
	else
	{
		print_msg_queue("read sf capacity fail\n");
		goto fail;
	}

	//parameter checking
	cmp_size = sf_cap.sector_size;
	if(cmp_start>=sf_cap.chip_size || (cmp_start&sf_cap.sector_size)!=0 || (cmp_start+cmp_size)>=sf_cap.chip_size)
	{
		print_msg_queue("parameter error\n");
		goto fail;
	}

	cmp_size = sf_cap.sector_size;

	sf_backup = pvPortMalloc(cmp_size, GFP_DMA, MODULE_CLI);
	if(!sf_backup)
	{
		print_msg_queue("sf_backup allocate error\n");
		goto fail;
	}
	//read sf  original data
	if(MID_SF_QUEUE_FINISH != mid_sf_read(sf_backup, cmp_start,cmp_size,NULL))
	{
		print_msg_queue("read sf data fail\n");
		goto fail;
	}

	//sector erase
	if(MID_SF_QUEUE_FINISH == mid_sf_sector_erase(cmp_start, cmp_size, NULL))
		print_msg_queue("sector erase success\n");
	else
		print_msg_queue("sf erase fail\n");


	//buffer allocate
	sf_buf1 = pvPortMalloc(cmp_size, GFP_DMA, MODULE_CLI);
	if(!sf_buf1)
	{
		print_msg_queue("buffer1 allocate error\n");
		goto fail;
	}

	//sector erase check
	memset(sf_buf1, 0, cmp_size);
	if(MID_SF_QUEUE_FINISH == mid_sf_read(sf_buf1, cmp_start,cmp_size,NULL))
	{
		for(i=0;i<sf_cap.sector_size;i++)
		{
			if(sf_buf1[i]!=0xff)
				break;
		}
		if(i!=sf_cap.sector_size)
			print_msg_queue("sector erase check fail\n");
		else
			print_msg_queue("sector erase check ok\n");
	}
	else
		print_msg_queue("sf read fail\n");



	ret = mid_sf_write(cmp_start,sf_backup,cmp_size,NULL);
	print_msg_queue("write backup %s\n", (ret==MID_SF_QUEUE_FINISH)?"success":"fail");

	ret = mid_sf_read(sf_buf1, cmp_start,cmp_size,NULL);
	print_msg_queue("read %s\n", (ret==MID_SF_QUEUE_FINISH)?"success":"fail");

	for(i=0; i<cmp_size; i++ )
	{
		if(sf_buf1[i]!=sf_backup[i])
		{
			print_msg_queue("data compare error (%x != %x) from %d\n", sf_buf1[i], sf_backup[i], i);
			break;
		}
	}

	if(i==cmp_size)
		print_msg_queue("data compare success\n");

	if(sf_backup)
		vPortFree(sf_backup);
	if(sf_buf1)
		vPortFree(sf_buf1);
	return 0;


fail:
	if(sf_backup)
		vPortFree(sf_backup);
	if(sf_buf1)
		vPortFree(sf_buf1);
	print_msg_queue("sf rwtest fail\n");

	return 0;
}
/** @} */
