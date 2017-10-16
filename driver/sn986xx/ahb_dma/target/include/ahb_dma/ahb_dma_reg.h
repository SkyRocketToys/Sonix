/*
 * ahb_dma.h
 *
 *  Created on: May 4, 2015
 *      Author: spark_lee
 */

#ifndef DRIVER_SN986XX_AHB_DMA_SRC_AHB_DMA_REG_H_
#define DRIVER_SN986XX_AHB_DMA_SRC_AHB_DMA_REG_H_

#define BSP_GPIO_BASE_ADDRESS		(0x90400000)
#define BASE_DMA 					BSP_GPIO_BASE_ADDRESS


#define BIT0			0x01
#define BIT1			0x02
#define BIT2			0x04
#define BIT3			0x08
#define BIT4			0x10
#define BIT5			0x20
#define BIT6			0x40
#define BIT7			0x80
#define BIT8			0x0100
#define BIT9			0x0200
#define BIT10			0x0400
#define BIT11			0x0800
#define BIT12			0x1000
#define BIT13			0x2000
#define BIT14			0x4000
#define BIT15			0x8000
#define BIT16			0x010000
#define BIT17			0x020000
#define BIT18			0x040000
#define BIT19			0x080000
#define BIT20			0x100000
#define BIT21			0x200000
#define BIT22			0x400000
#define BIT23			0x800000
#define BIT24			0x01000000
#define BIT25			0x02000000
#define BIT26			0x04000000
#define BIT27			0x08000000
#define BIT28			0x10000000
#define BIT29			0x20000000
#define BIT30			0x40000000
#define BIT31			0x80000000


#define DMA_CHAN_BASE			(BASE_DMA + 0x100)
#define DMA_CHAN_OFFSET_SHIFT	0x5

#define BASE_DMA_CH(ch)			(DMA_CHAN_BASE + (ch << DMA_CHAN_OFFSET_SHIFT))

//-----------------------------------------------------------------------------
#define DMA_INTR				(BASE_DMA)

#define DMA_INTR_MASK			(BIT0 | BIT1 | BIT2 | BIT3 \
								| BIT4 | BIT5 | BIT6 | BIT7)

//-----------------------------------------------------------------------------
#define DMA_INTR_TC				(BASE_DMA + 0x4)

#define DMA_INTR_TC_MASK		(BIT0 | BIT1 | BIT2 | BIT3 \
								| BIT4 | BIT5 | BIT6 | BIT7)

//-----------------------------------------------------------------------------
#define DMA_INTR_TC_CLR			(BASE_DMA + 0x8)

#define CLR_DMA_INTR_TC_MASK	(BIT0 | BIT1 | BIT2 | BIT3 \
								| BIT4 | BIT5 | BIT6 | BIT7)

//-----------------------------------------------------------------------------
#define DMA_INTR_ERRABT			(BASE_DMA + 0xC)
// bit definition
#define DMA_INTR_ERR_BIT		0
#define DMA_INTR_ABT_BIT		16

#define DMA_INTR_ERR_MASK		(BIT0 | BIT1 | BIT2 | BIT3 \
								| BIT4 | BIT5 | BIT6 | BIT7)
#define DMA_INTR_ABT_MASK		(BIT16 | BIT17 | BIT18 | BIT19 \
								| BIT20 | BIT21 | BIT22 | BIT23)

//-----------------------------------------------------------------------------
#define DMA_INTR_ERRABT_CLR		(BASE_DMA + 0x10)
// bit definition
#define CLR_DMA_INTR_ERR_BIT	0
#define CLR_DMA_INTR_ABT_BIT	16

#define CLR_DMA_INTR_ERR_MASK	(BIT0 | BIT1 | BIT2 | BIT3 \
								| BIT4 | BIT5 | BIT6 | BIT7)
#define CLR_DMA_INTR_ABT_MASK	(BIT16 | BIT17 | BIT18 | BIT19 \
								| BIT20 | BIT21 | BIT22 | BIT23)

//-----------------------------------------------------------------------------
#define DMA_TC					(BASE_DMA + 0x14)

#define DMA_TC_MASK				(BIT0 | BIT1 | BIT2 | BIT3 \
								| BIT4 | BIT5 | BIT6 | BIT7)

//-----------------------------------------------------------------------------
#define DMA_ERRABT				(BASE_DMA + 0x18)

#define DMA_ERRABT_MASK			(BIT0 | BIT1 | BIT2 | BIT3 \
								| BIT4 | BIT5 | BIT6 | BIT7)
//-----------------------------------------------------------------------------
#define DMA_CH_EN				(BASE_DMA + 0x1C)

#define DMA_CH_EN_MASK			(BIT0 | BIT1 | BIT2 | BIT3 \
								| BIT4 | BIT5 | BIT6 | BIT7)
//-----------------------------------------------------------------------------
#define DMA_CH_BUSY				(BASE_DMA + 0x20)

#define DMA_CH_BUSY_MASK		(BIT0 | BIT1 | BIT2 | BIT3 \
								| BIT4 | BIT5 | BIT6 | BIT7)

//-----------------------------------------------------------------------------
#define DMA_CSR					(BASE_DMA + 0x24)
// bit definition
#define DMA_EN_BIT				0
#define M0ENDIAN_BIT			1
#define M1ENDIAN_BIT			2

//-----------------------------------------------------------------------------
#define DMA_SYNC				(BASE_DMA + 0x28)

#define DMA_SYNC_MASK			(BIT0 | BIT1 | BIT2 | BIT3 \
								| BIT4 | BIT5 | BIT6 | BIT7)

//-----------------------------------------------------------------------------
#define DMAC_REVISION			(BASE_DMA + 0x30)
//bit definition
#define DMA_REL_REV_BIT			0
#define DMA_MINOR_REV_BIT		8
#define DMA_MAJOR_REV_BIT		16

#define DMA_REL_REV_MASK		(BIT0 | BIT1 | BIT2 | BIT3 \
								| BIT4 | BIT5 | BIT6 | BIT7)

#define DMA_MINOR_REV_MASK		(BIT8 | BIT9 | BIT10 | BIT11 \
								| BIT12 | BIT13 | BIT14 | BIT15)

#define DMA_MAJOR_REV_MASK		(BIT16 | BIT17 | BIT18 | BIT19 \
								| BIT20 | BIT21 | BIT22 | BIT23)

//-----------------------------------------------------------------------------
#define DMAC_FEATURE			(BASE_DMA + 0x34)
//bit definition
#define DMA_ADDR_WIDTH_BIT		0
#define DMA_HAVE_LINKLIST_BIT	8
#define DMA_HAVE_AHB1_BIT		9
#define DMA_HAVE_BRIDGE_BIT		10
#define DMA_MAX_CHNO_BIT		12

#define DMA_ADDR_WIDTH_MASK		(BIT0 | BIT1 | BIT2 | BIT3)

#define DMA_MAX_CHNO_MASK		(BIT12 | BIT13 | BIT14 | BIT15)

//-----------------------------------------------------------------------------
#define DMA_CHn_CSR(ch)			(BASE_DMA_CH(ch))
// bit definition
#define CHn_EN_BIT				0
#define CHn_DST_SEL_BIT			1
#define CHn_SRC_SEL_BIT			2
#define CHn_DST_INC_BIT			3
#define CHn_SRC_INC_BIT			5
#define CHn_MODE_BIT			7
#define CHn_DST_WIDTH_BIT		8
#define CHn_SRC_WIDTH_BIT		11
#define CHn_ABT_BIT				15
#define CHn_SRC_SIZE_BIT		16
#define CHn_PROT_BIT			19
#define CHn_PRI_BIT				22
#define CHn_FF_TH_BIT			24
#define CHn_TC_MSK_BIT			31

#define CHn_DST_INC_MASK		(BIT3 | BIT4)
#define CHn_SRC_INC_MASK		(BIT5 | BIT6)
#define CHn_DST_WIDTH_MASK		(BIT8 | BIT9 | BIT10)
#define CHn_SRC_WIDTH_MASK		(BIT11 | BIT12 | BIT13)
#define CHn_SRC_SIZE_MASK		(BIT16 | BIT17 | BIT18)
#define CHn_PROT_MASK			(BIT19 | BIT20 | BIT21)
#define CHn_PRI_MASK			(BIT22 | BIT23)
#define CHn_FF_TH_MASK			(BIT24 | BIT25 | BIT26)

//-----------------------------------------------------------------------------
#define DMA_CHn_CFG(ch)			(BASE_DMA_CH(ch) + 0x4)
// bit definition
#define CHn_INTR_TC_MSK_BIT		0
#define CHn_INTR_ERR_MSK_BIT	1
#define CHn_INTR_ABT_MSK_BIT	2
#define CHn_CH0_SRC_REQ_BIT		3
#define CHn_SRC_HE_BIT			7
#define CHn_BUSY_BIT			8
#define CHn_DST_REQ_BIT			9
#define CHn_DST_HE_BIT			13
#define CHn_LLP_CNT_BIT			16

#define CHn_CH0_SRC_REQ_MASK	(BIT3 | BIT4 | BIT5 | BIT6)
#define CHn_DST_REQ_MASK		(BIT9 | BIT10 | BIT11 | BIT12)
#define CHn_LLP_CNT_MASK		(BIT16 | BIT17 | BIT18 | BIT19)

//-----------------------------------------------------------------------------
#define DMA_CHn_SRC_A(ch)		(BASE_DMA_CH(ch) + 0x8)

#define DMA_CHn_SRC_A_MASK		(BIT0 | BIT1 | BIT2 | BIT3 \
								| BIT4 | BIT5 | BIT6 | BIT7 \
								| BIT8 | BIT9 | BIT10 | BIT11 \
								| BIT12 | BIT13 | BIT14 | BIT15 \
								| BIT16 | BIT17 | BIT18 | BIT19 \
								| BIT20 | BIT21 | BIT22 | BIT23 \
								| BIT24 | BIT25 | BIT26 | BIT27 \
								| BIT28 | BIT29 | BIT30 | BIT31)

//-----------------------------------------------------------------------------
#define DMA_CHn_DST_A(ch)	 	(BASE_DMA_CH(ch) + 0xC)

#define DMA_CHn_DST_A_MASK		(BIT0 | BIT1 | BIT2 | BIT3 \
								| BIT4 | BIT5 | BIT6 | BIT7 \
								| BIT8 | BIT9 | BIT10 | BIT11 \
								| BIT12 | BIT13 | BIT14 | BIT15 \
								| BIT16 | BIT17 | BIT18 | BIT19 \
								| BIT20 | BIT21 | BIT22 | BIT23 \
								| BIT24 | BIT25 | BIT26 | BIT27 \
								| BIT28 | BIT29 | BIT30 | BIT31)

//-----------------------------------------------------------------------------
#define DMA_CHn_LLP(ch)			(BASE_DMA_CH(ch) + 0x10)
// bit definition
#define CHn_LLP_SRC_BIT			0
#define CHn_LLP_ADDR_BIT		2

#define CHn_LLP_ADDR_MASK		(BIT2 | BIT3 | BIT4 | BIT5 | BIT6 | BIT7 \
								| BIT8 | BIT9 | BIT10 | BIT11 \
								| BIT12 | BIT13 | BIT14 | BIT15 \
								| BIT16 | BIT17 | BIT18 | BIT19 \
								| BIT20 | BIT21 | BIT22 | BIT23 \
								| BIT24 | BIT25 | BIT26 | BIT27 \
								| BIT28 | BIT29 | BIT30 | BIT31)

//-----------------------------------------------------------------------------
#define DMA_CHn_SIZE(ch)		(BASE_DMA_CH(ch) + 0x14)

#define DMA_CHn_SIZE_MASK		(BIT0 | BIT1 | BIT2 | BIT3 \
								| BIT4 | BIT5 | BIT6 | BIT7 \
								| BIT8 | BIT9 | BIT10 | BIT11 \
								| BIT12 | BIT13 | BIT14 | BIT15 \
								| BIT16 | BIT17 | BIT18 | BIT19 \
								| BIT20 | BIT21)




#endif /* DRIVER_SN986XX_AHB_DMA_SRC_AHB_DMA_REG_H_ */
