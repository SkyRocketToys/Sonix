/*
 * ahb_dma.h
 *
 *  Created on: May 4, 2015
 *      Author: spark_lee
 */

#ifndef DRIVER_SN986XX_AHB_DMA_SRC_AHB_DMA_H_
#define DRIVER_SN986XX_AHB_DMA_SRC_AHB_DMA_H_


#define DMA_MAX_CHANNEL		4

#define AHB_FUNC	0
#define APB_FUNC	1


//=========================================================================
// DMA Channel
//=========================================================================
typedef enum
{
	DMA_CHAN_0 = 0,
	DMA_CHAN_1,
	DMA_CHAN_2,
	DMA_CHAN_3
} dma_chan_t;

//=========================================================================
// DMA Master
//=========================================================================
typedef enum
{
	DMA_MASTER_0 = 0,	// connect to common bus
	DMA_MASTER_1		// dedicated bus to ddr
} dma_master_t;

//=========================================================================
// DMA Switch
//=========================================================================
typedef enum
{
	DMA_ENABLE = 0,
	DMA_MASK
} dma_switch_t;

//=========================================================================
// DMA Endian Configuration
//=========================================================================
typedef enum
{
	DMA_LITTLE_ENDIAN = 0,
	DMA_BIG_ENDIAN
} dma_endian_t;

//=========================================================================
// DMA Source/Destination Address Control
//=========================================================================
typedef enum
{
	DMA_INC_ADDR = 0,
	DMA_DEC_ADDR,
	DMA_FIX_ADDR
} dma_addr_ctrl_t;

//=========================================================================
// DMA Mode
//=========================================================================
typedef enum
{
	DMA_NORMAL_MODE = 0,
	DMA_HANDSHAKE_MODE
} dma_mode_t;

//=========================================================================
// DMA Transfer Mode
//=========================================================================
typedef enum
{
	DMA_NO_CHAIN_MODE = 0,
	DMA_CHAIN_MODE
} dma_trans_mode_t;

//=========================================================================
// DMA Transfer Width
//=========================================================================
typedef enum
{
	DMA_WIDTH_8BIT = 0,
	DMA_WIDTH_16BIT,
	DMA_WIDTH_32BIT
} dma_trans_width_t;

static const char *DMA_Trans_Width_str[] = {
	"DMA_WIDTH_8BIT",
	"DMA_WIDTH_16BIT",
	"DMA_WIDTH_32BIT"
};

//=========================================================================
// DMA Source Burst Size
//=========================================================================
typedef enum
{
	DMA_BURST_1 = 0,
	DMA_BURST_4,
	DMA_BURST_8,
	DMA_BURST_16,
	DMA_BURST_32,
	DMA_BURST_64,
	DMA_BURST_128,
	DMA_BURST_256
} dma_burst_t;

static const char *DMA_Burst_str[] = {
	"DMA_BURST_1",
	"DMA_BURST_4",
	"DMA_BURST_8",
	"DMA_BURST_16",
	"DMA_BURST_32",
	"DMA_BURST_64",
	"DMA_BURST_128",
	"DMA_BURST_256"
};

//=========================================================================
// DMA Channel Priority Level
//=========================================================================
typedef enum
{
	DMA_LOWEST_PRI = 0,
	DMA_3RD_HIGH_PRI,
	DMA_2ND_HIGH_PRI,
	DMA_HIGHEST_PRI
} dma_pri_t;

//=========================================================================
// Linked List Pointer(LLP)
//=========================================================================
typedef struct
{
	uint32_t master_id:1;
	uint32_t reserved:1;
	uint32_t link_list_addr:30;
} dma_llp_t;

//=========================================================================
// Control in Linked List Descriptor
//=========================================================================
typedef struct
{
	uint32_t size:12;
	uint32_t reserved:4;
	uint32_t dst_sel:1;
	uint32_t src_sel:1;
	uint32_t dst_ctrl:2;
	uint32_t src_ctrl:2;
	uint32_t dst_width:3;
	uint32_t src_width:3;
	uint32_t tc_msk:1;
	uint32_t reserved1:3;
} dma_ctrl_t;

//=========================================================================
// Linked List Descriptor
//=========================================================================
typedef struct
{
	uint32_t src_addr;
	uint32_t dst_addr;
	dma_llp_t llp;
	dma_ctrl_t ctrl;
	uint32_t size;
} dma_lld_t;

//=========================================================================
// AHB channel interrupt mask
//=========================================================================
typedef struct {
	dma_switch_t tc_intr_mask;     /* tc intr flag enable=0 mask=1 */
	dma_switch_t error_intr_mask;  /* error intr flag enable=0 mask=1 */
	dma_switch_t abt_intr_mask;    /* abt intr flag enable=0 mask=1 */
} dma_intr_ctl_t;

//=========================================================================
// lld channel control
//=========================================================================
typedef struct {
	uint32_t lld_count;    /* linked list dest count */
	uint32_t lld_size;     /* linked list dest size */
	dma_lld_t *start_addr;	/* start address of lld buffer*/
} lld_chan_ctl_t;

//=========================================================================
// AHB Master Endian Setting
//=========================================================================
typedef struct {
	dma_endian_t master_0;
	dma_endian_t master_1;
} endian_chan_ctl_t;

//=========================================================================
// AHB DMA Channel Descriptor
//=========================================================================
typedef struct {
	dma_master_t sel;	// DMA MASTER 0: use common bus; DMA MASTER 1: use proprietary bus to DDR2
	dma_addr_ctrl_t ctl;
	dma_trans_width_t data_width;
	uint32_t addr;
} ahbdma_chan_ctl_t;

typedef struct
{
	int8_t				*name;		/* channel name */
	uint8_t				which;		/* select AHB or APB DMA function */
	uint8_t				polling;	/* channel polling ckeck */
	dma_chan_t			chan_num;	/* channel number */
	dma_burst_t			burst;		/* transfer burst size */
	dma_intr_ctl_t		dma_intr;	/* channel intr flags */
	endian_chan_ctl_t	endian;		/* endian setting */
	dma_pri_t			pri;		/* priority */
	dma_mode_t			handshake;	/* protocol */
	dma_trans_mode_t	chain;		/* transfer mode */
	uint32_t				tr_size;	/* transfer size */
	lld_chan_ctl_t		lld_chan;	/* channel lld ctl */
	ahbdma_chan_ctl_t	src;		/* src addr ctl */
	ahbdma_chan_ctl_t	dst;		/* dst addr ctl */
} ahbdma_chan_desc_t;

//=========================================================================
// Function API
//=========================================================================
void dma_reset_chan (dma_chan_t chan);
void dma_enable_chan_sync (dma_chan_t chan);
void dma_disable_chan_sync (dma_chan_t chan);
void dma_enable_chan (dma_chan_t chan);
void dma_disable_chan (dma_chan_t chan);
void dma_set_chan_intr (dma_chan_t chan, dma_switch_t tc_int_msk,
						dma_switch_t err_int_msk, dma_switch_t abt_int_msk);
void dma_set_chan_trans_info (dma_chan_t chan,
						uint32_t src_addr, uint32_t dst_addr, uint32_t size);
void dma_normal_mode (ahbdma_chan_desc_t *pchan);
void dma_link_mode (ahbdma_chan_desc_t *pchian, dma_lld_t* lld);



uint32_t DMA_VERSION (void);
void DMA_FEATURE (uint8_t *p_width, uint8_t *p_max_ch,
						uint8_t *p_list, uint8_t *p_ahb1, uint8_t *p_bridge);
void DMA_Enable (dma_endian_t master0_endian,
							dma_endian_t master1_endian);

uint8_t DMA_INTR_Status (void);
uint8_t DMA_INTR_TC_Status (void);
uint32_t DMA_INTR_ERRABT_Status (uint8_t *pERR, uint8_t *pABT);
uint8_t DMA_TC_Status (void);
uint32_t DMA_ERRABT_Status (uint8_t *pERR, uint8_t *pABT);
void DMA_INTR_TC_Clear (uint8_t ch_bit_map);
void DMA_INTR_ERRABT_Clear (uint8_t err_ch_bit_map, uint8_t abt_ch_bit_map);
uint8_t DMA_CH_EN_Status (void);
uint8_t DMA_CH_BUSY_Status (void);







//=========================================================================
// DMA API
//=========================================================================
uint8_t dma_chan_init (void *);
void dma_chan_run (void *);
void dma_chan_is_finish (void *);



#endif /* DRIVER_SN986XX_AHB_DMA_SRC_AHB_DMA_H_ */
