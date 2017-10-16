#define SPEED_100_bit		(1UL<<18)	//Speed mode. 1:100 Mbps; 0:10 Mbps
#define RX_BROADPKT_bit		(1UL<<17)	// Receiving broadcast packet
#define RX_MULTIPKT_bit		(1UL<<16)	// receiving multicast packet
#define FULLDUP_bit		(1UL<<15)	// full duplex
#define CRC_APD_bit		(1UL<<14)	// append crc to transmit packet
#define MDC_SEL_bit		(1UL<<13)	// set MDC as TX_CK/10
#define RCV_ALL_bit		(1UL<<12)	// not check incoming packet's destination address
#define RX_FTL_bit		(1UL<<11)	// Store incoming packet even its length is great than 1518 byte
#define RX_RUNT_bit		(1UL<<10)	// Store incoming packet even its length is les than 64 byte
#define HT_MULTI_EN_bit		(1UL<<9)
#define RCV_EN_bit		(1UL<<8)	// receiver enable
#define ENRX_IN_HALFTX_bit	(1UL<<6)	//Enables packet reception when transmitting packet in half duplex mode.
#define XMT_EN_bit		(1UL<<5)	// transmitter enable
#define CRC_DIS_bit		(1UL<<4)
#define LOOP_EN_bit		(1UL<<3)	// Internal loop-back
#define SW_RST_bit		(1UL<<2)	// software reset/
#define RDMA_EN_bit		(1UL<<1)	// enable DMA receiving channel
#define XDMA_EN_bit		(1UL<<0)	// enable DMA transmitting channel


// --------------------------------------------------------------------
//		FTMAC110 hardware related definition
// --------------------------------------------------------------------

#define ISR_REG			0x00		// interrups status register
#define IMR_REG			0x04		// interrupt maks register
#define MAC_MADR_REG		0x08		// MAC address (Most significant)
#define MAC_LADR_REG		0x0c		// MAC address (Least significant)

#define MAHT0_REG		0x10		// Multicast Address Hash Table 0 register
#define MAHT1_REG		0x14		// Multicast Address Hash Table 1 register
#define TXPD_REG		0x18		// Transmit Poll Demand register
#define RXPD_REG		0x1c		// Receive Poll Demand register
#define TXR_BADR_REG		0x20		// Transmit Ring Base Address register
#define RXR_BADR_REG		0x24		// Receive Ring Base Address register
#define ITC_REG			0x28		// interrupt timer control register
#define APTC_REG		0x2c		// Automatic Polling Timer control register
#define DBLAC_REG		0x30		// DMA Burst Length and Arbitration control register

#define MACCR_REG		0x88		// MAC control register
#define MACSR_REG		0x8c		// MAC status register
#define PHYCR_REG		0x90		// PHY control register
#define PHYWDATA_REG		0x94		// PHY Write Data register
#define FCR_REG			0x98		// Flow Control register
#define BPR_REG			0x9c		// back pressure register
#define WOLCR_REG		0xa0		// Wake-On-Lan control register
#define WOLSR_REG		0xa4		// Wake-On-Lan status register
#define WFCRC_REG		0xa8		// Wake-up Frame CRC register
#define WFBM1_REG		0xb0		// wake-up frame byte mask 1st double word register
#define WFBM2_REG		0xb4		// wake-up frame byte mask 2nd double word register
#define WFBM3_REG		0xb8		// wake-up frame byte mask 3rd double word register
#define WFBM4_REG		0xbc		// wake-up frame byte mask 4th double word register
#define TM_REG			0xcc		// test mode register



/* Generic MII registers. */

#define MII_BMCR            0x00        /* Basic mode control register */
#define MII_BMSR            0x01        /* Basic mode status register  */
#define MII_PHYSID1         0x02        /* PHYS ID 1                   */
#define MII_PHYSID2         0x03        /* PHYS ID 2                   */
#define MII_ADVERTISE       0x04        /* Advertisement control reg   */
#define MII_LPA             0x05        /* Link partner ability reg    */
#define MII_EXPANSION       0x06        /* Expansion register          */
#define MII_CTRL1000        0x09        /* 1000BASE-T control          */
#define MII_STAT1000        0x0a        /* 1000BASE-T status           */
#define MII_ESTATUS    		0x0f	    /* Extended Status */
#define MII_DCOUNTER        0x12        /* Disconnect counter          */
#define MII_FCSCOUNTER      0x13        /* False carrier counter       */
#define MII_NWAYTEST        0x14        /* N-way auto-neg test reg     */
#define MII_RERRCOUNTER     0x15        /* Receive error counter       */
#define MII_SREVISION       0x16        /* Silicon revision            */
#define MII_RESV1           0x17        /* Reserved...                 */
#define MII_LBRERROR        0x18        /* Lpback, rx, bypass error    */
#define MII_PHYADDR         0x19        /* PHY address                 */
#define MII_RESV2           0x1a        /* Reserved...                 */
#define MII_TPISTATUS       0x1b        /* TPI status for 10mbps       */
#define MII_NCONFIG         0x1c        /* Network interface config    */

/* Basic mode control register. */

#define BMCR_RESV               0x003f  /* Unused...                   */
#define BMCR_SPEED1000          0x0040  /* MSB of Speed (1000)         */
#define BMCR_CTST               0x0080  /* Collision test              */
#define BMCR_FULLDPLX           0x0100  /* Full duplex                 */
#define BMCR_ANRESTART          0x0200  /* Auto negotiation restart    */
#define BMCR_ISOLATE            0x0400  /* Disconnect DP83840 from MII */
#define BMCR_PDOWN              0x0800  /* Powerdown the DP83840       */
#define BMCR_ANENABLE           0x1000  /* Enable auto negotiation     */
#define BMCR_SPEED100           0x2000  /* Select 100Mbps              */
#define BMCR_LOOPBACK           0x4000  /* TXD loopback bits           */
#define BMCR_RESET              0x8000  /* Reset the DP83840	       */ 


/*
 * MAC control register bits
 */
#define MACCR_SPD               (0x1<<18)       /* Speed mode: 1:100 Mbps; 0:10 Mbps */
#define MACCR_RX_BRO            (0x1<<17)       /* Receive broadcast packet */
#define MACCR_RX_MUL            (0x1<<16)       /* Receive all multicast packet */
#define MACCR_FD                (0x1<<15)       /* Full duplex mode */
#define MACCR_CRC_APD           (0x1<<14)       /* Append CRC to packet to be transmitted */
#define MACCR_RX_ALL            (0x1<<12)       /* Not check destination address of incoming packet */
#define MACCR_RX_FTL            (0x1<<11)       /* Store incoming packet great than 1518 bytes */
#define MACCR_RX_RUNT           (0x1<<10)       /* Store incoming packet less than 64 bytes */
#define MACCR_MUL_HT            (0x1<<9)        /* Store incoming multcast packet */
#define MACCR_RX_EN             (0x1<<8)        /* Enable Receiver */
#define MACCR_ENRX_IN_HALF      (0x1<<6)        /* Enale reception when transmitting in half duplex mode*/
#define MACCR_TX_EN             (0x1<<5)        /* Enable Transmitter */
#define MACCR_CRC_DIS           (0x1<<4)        /* Disable CRC check when receiving packet */
#define MACCR_LOOP_EN           (0x1<<3)        /* Internal loop back mode */
#define MACCR_RST               (0x1<<2)        /* Reset Ethernet MAC controller */
#define MACCR_RXDMA_EN          (0x1<<1)        /* Enable receive DMA channel */
#define MACCR_TXDMA_EN          (0x1)           /* Enable transmit DMA channel */

/*
 * MAC interrupt bits
 */
#define PHYSTS_CHG			(0x1<<9)     	/* PHY link status change */
#define AHB_ERR				(0x1<<8)     	/* AHB error */
#define RPKT_LOST			(0x1<<7)     	/* Received packet lost due to RX FIFO full */
#define RPKT_OK				(0x1<<6)     	/* Packet received into RX FIFO successfully */
#define XPKT_LOST			(0x1<<5)     	/* Packet transmitted to Ethernet lost due to collision */
#define XPKT_OK				(0x1<<4)     	/* Packet transmitted to Ethernet successfully */
#define NOTXBUF				(0x1<<3)     	/* Transmit buffer not available */
#define XPKT_FINISH			(0x1<<2)     	/* TXDMA has moved data into TX FIFO */
#define NORXBUF				(0x1<<1)     	/* Receive buffer not available */
#define RPKT_FINISH			(0x1)     		/* RXDMA has received packets to RX buffer successfully */

#define MAC_ALLINTRS		(0x3ff)     	/* All interrupts of MAC */


/*
 * PHY control register bits
 */
#define PHYCR_MIIWR     (0x1<<27)   /* Write PHY operation */
#define PHYCR_MIIRD     (0x1<<26)   /* Read PHY operation */

/* Davicom 9161 PHY */
#define PHYID_LAN8720	0x0007c0f1
#define PHYID_IP101G	0x02430c54


#define OWNBY_SOFTWARE		0
#define OWNBY_FTMAC110		1
#define OWNBY_SW		0
#define OWNBY_MAC		1

#define MAC_FAIL		-1
#define MAC_OK        0                                   


typedef struct
{
	// RXDES0
	unsigned int ReceiveFrameLength:11;//0~10
	unsigned int Reserved1:5;          //11~15
	unsigned int MULTICAST:1;          //16
	unsigned int BROARDCAST:1;         //17
	unsigned int RX_ERR:1;             //18
	unsigned int CRC_ERR:1;            //19
	unsigned int FTL:1;
	unsigned int RUNT:1;
	unsigned int RX_ODD_NB:1;
	unsigned int Reserved2:5;
	unsigned int LRS:1;
	unsigned int FRS:1;
	unsigned int Reserved3:1;
	unsigned int RXDMA_OWN:1;			// 1 ==> owned by FTMAC110, 0 ==> owned by software

	// RXDES1
	unsigned int RXBUF_Size:11;
	unsigned int Reserved:20;
	unsigned int EDOTR:1;

	// RXDES2
	unsigned int RXBUF_BADR;

	unsigned int VIR_RXBUF_BADR;			// not defined, Save receive buffer of virtual address
}RX_DESC;

typedef struct
{
	// TXDES0
	unsigned int TXPKT_LATECOL:1;
	unsigned int TXPKT_EXSCOL:1;
	unsigned int Reserved1:29;
	unsigned int TXDMA_OWN:1;

	// TXDES1
	unsigned int TXBUF_Size:11;
	unsigned int Reserved2:16;
	unsigned int LTS:1;
	unsigned int FTS:1;
	unsigned int TX2FIC:1;
	unsigned int TXIC:1;
	unsigned int EDOTR:1;

	// RXDES2
	unsigned int TXBUF_BADR;
	unsigned int VIR_TXBUF_BADR;			// Reserve, Save receive buffer of virtual address
}TX_DESC;

struct ftmac110_local {
 	// these are things that the kernel wants me to keep, so users
	// can find out semi-useless statistics of how well the card is
	// performing
	//struct net_device_stats stats;
	// Set to true during the auto-negotiation sequence
	int autoneg_active;
	// Address of our PHY port
	unsigned int phyaddr;
	// Type of PHY
	unsigned int phytype;
	// Last contents of PHY Register 18
	unsigned int lastPhy18;
	volatile RX_DESC *rx_descs;			// receive ring base address
	unsigned int rx_descs_dma;				// receive ring physical base address
	char *rx_buf;					// receive buffer cpu address
	int rx_buf_dma;					// receive buffer physical address
	int rx_idx;						// Current receive descriptor
	volatile TX_DESC *tx_descs;
	unsigned int tx_descs_dma;
	char *tx_buf;
	int tx_buf_dma;
	int tx_idx;
	unsigned int maccr_val;
	unsigned int speed_mode;         /* speed mode: 1:100 Mbps; 0:10 Mbps */
	unsigned int full_duplex;   /* is full duplex? */
	unsigned int force_media;   /* is autoneg. disabled? */
};

typedef struct packet_buf {
	int size;
	char buf[1536];
}packet_buf_t;

int eth_init(void);
int mac_pkt_send(void *packet, int length);
int mac_pkt_send_to_queue(void *packet, int length);
void ftmac110_get_mac_addr(unsigned char* addr);
