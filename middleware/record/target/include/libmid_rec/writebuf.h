/**
 * @file
 * this is middleware write buffer header file, include this file before use
 * @author Algorithm Dept Sonix. (yiling porting to RTOS)
 */
#ifndef __WRITEBUF_LIB_H__
#define __WRITEBUF_LIB_H__

#define WRITE_CMD_WRITE_NUM	 100	
#define WRITE_CMD_FINISH_NUM	100
#define FILE_ATTR_UPDATE_COUNT	130

typedef enum{
	WB_IS_WRITE_BUF = 0,	/**<  buffer address belongs to one of pre allocated write buffer*/
	WB_IS_DATA_BUF,			/**<  buffer address is address of coming data*/
}BufType_t;

typedef enum{
	WB_CMD_FOPEN = 0,	
	WB_CMD_WR_FRAME,
	WB_CMD_FILL_DATA,
	WB_CMD_CLEAN_ALL,
	WB_CMD_FCLOSE
}CMDType_t;

typedef enum{
	WB_CMD_INIT = -1,
	WB_CMD_FAIL = 0,
	WB_CMD_SUCCESS=1,
}CMDStatus_t;

typedef enum{
	WB_STATUS_NORMAL = 0,
	WB_STATUS_WR_FAIL = 0x01,
	WB_STATUS_FULL=0x02,
}BufStatus_t;

typedef struct _WriteCmd{
	CMDType_t cmd_type;
	FIL	*pf;
	uint32_t arg1;
	uint32_t arg2;
	BufType_t ucWbFlag;	//type of pAddr 
	xSemaphoreHandle *pSema;
}WriteCmd_t;


typedef struct _ReturnCmd{
	CMDType_t cmd_type;
	FIL	*pf;
	uint32_t arg1;
	uint32_t arg2;
	CMDStatus_t status ;
}ReturnCmd_t;

typedef struct _frame_info{
	unsigned char *pAddr;
	unsigned int uiSize;
}frame_info;

typedef struct _BufInitInfo{
  uint32_t write_buf_size;
  int write_unit_to_file;
}BufInitInfo_t;

typedef struct _WriteBufInfo{
	uint8_t *WriteBuf;		//buffer to saving data 
	int BufStart;	//point to addr in buffer can be writed
	//*pRdBufStart;					
	int BufEnd;
	int iRemainSize;
	//int iUsedSize; 		//iRemainSize+iUsedSize = WRITE_BUF_SIZE
	int iWriteToFileUnit;
	FIL *pfile;							//file pointer
	unsigned long ulFilePos;
	BufInitInfo_t writebufferuserparam;
	uint8_t ucBufStatus;
	xQueueHandle queue_write;	
	xQueueHandle queue_finish;			
	xTaskHandle	task_write;				//task to write full buffer topfile
	xSemaphoreHandle Sema_block;		//semaphore to wait buffer write to file finish
	xSemaphoreHandle mutex;				//avoid  double entry
	uint8_t *OriginAddr;
	uint32_t dev;	/**<  current storage, 1:mmc, 2:usbh*/			
}WriteBufInfo_t;

int writebuf_init(WriteBufInfo_t *pWBInfo,BufInitInfo_t *pWBInitInfoParam);
int writebuf_uninit(WriteBufInfo_t *pWBInfo);
xQueueHandle* writebuf_get_finish_queue(WriteBufInfo_t *pWBInfo);
int writebuf_reset(WriteBufInfo_t *pWBInfo);
int writebuf_open_file(WriteBufInfo_t *pWBInfo, FIL *pf, char* fname);
int writebuf_close_file(WriteBufInfo_t *pWBInfo);
int writebuf_write_frame(WriteBufInfo_t *pWBInfo, frame_info *pframe_info, int len);
int writebuf_fill_data(WriteBufInfo_t *pWBInfo, unsigned long pos, unsigned long data);
char writebuf_get_wb_status(WriteBufInfo_t *pWBInfo);
int writebuf_wb_create(WriteBufInfo_t *pWBInfo,BufInitInfo_t *pWBInitInfoParam);
int writebuf_wb_release(WriteBufInfo_t *pWBInfo);

#endif	//__WRITEBUF_LIB_H__
