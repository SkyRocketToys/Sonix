#include "common.h"
#include "generated/snx_sdk_conf.h"

u32 FLASH_INFO_START;	
u32 HW_SETTING_START;   
u32 U_BOOT_START;       
u32 U_ENV_START;        
u32 FLASH_LAYOUT_START; 
u32 FACTORY_START;      
u32 U_LOGO_START;       
u32 RESCUE_START;
u32 AHB2_CLOCK;

u32 IMAGE_TABLE_START;
u32 IMAGE_TABLE_SIZE;

u32 ONLY_BRINGUP_RTOS = 0;


#define ITCM_ZI 0xFFFF6000
#define SB_FWIMG_LOAD_ADDR 	(ITCM_ZI + 0x1c)
#define SB_FWIMG_SIZE 	0x00010000

#define SNX_SYS_BASE		0x98000000
#define WD_BASE 			0x98700000


// For MCU
#define UART2_TIME_OUT				2000000

#define BSP_TIMER_BASE_ADDRESS          (0x98600000)

#define BSP_UART2_BASE_ADDRESS          (0x98b00000)
#define	TX_MODE_UART				1
#define	TX_STOP_BIT_2				1<<1
#define	TX_RDY						1<<2
#define	RX_MODE_UART				1<<4
#define	RX_RDY						1<<5
#define	RS485_EN					1<<7
#define	RS232_DMA_TX_EN				1<<8
#define	RS232_DMA_RX_EN				1<<9
#define	DMA_TX_BURST_MODE			1<<10
#define	DMA_RX_BURST_MODE			1<<11
#define	RS232_TX_INT_EN_BIT			1<<16
#define	RS232_RX_INT_EN_BIT			1<<17
#define	RS232_TX_INT_CLR_BIT		1<<18
#define	RS232_RX_INT_CLR_BIT		1<<19

#define	UART_CONFIG					0x0
#define	UART_CLOCK					0x0C
#define	RS_DATA						0x10
#define	FIFO_THD					0x18

#define outl(addr, value)       (*((volatile unsigned int *)(addr)) = value)
#define inl(addr)                (*((volatile unsigned int *)(addr)))

typedef unsigned char  uint8_t;



int mcu_uart2_set_command(unsigned char data[12])
{
	uint8_t i;
	uint32_t cnt = 0;

	while((inl((unsigned int*)(BSP_UART2_BASE_ADDRESS + UART_CONFIG))&TX_RDY) == 0){
		cnt++;
		
		if(cnt > UART2_TIME_OUT){
			serial_puts("MCU set command timeout\n");
			return 0;
		}	
	}	
	
	for(i=0; i<12; i++){
		outl((unsigned int*)((BSP_UART2_BASE_ADDRESS + RS_DATA)), data[i]);
	}
	
	return 1;
}

int mcu_uart2_get_command(unsigned char data[12])
{
	uint8_t i;
	uint32_t cnt = 0;

	/* Get the received mcu command from the UART */
	while(inl((unsigned int*)(BSP_UART2_BASE_ADDRESS + UART_CONFIG))&RX_RDY){
		for(i=0; i<12; i++){
			data[i] = (unsigned char)inl((unsigned int*)(BSP_UART2_BASE_ADDRESS + RS_DATA));
		}
		
		if((inl((unsigned int*)(BSP_UART2_BASE_ADDRESS + UART_CONFIG))&RX_RDY) == 0){
			return 1;
		}
		
		cnt++;
		
		if(cnt > UART2_TIME_OUT){
			serial_puts("MCU get command timeout....\n");
			return 1;
		}
	}

	return 0;
}


int mcu_set_sync_status(uint8_t data)
{
	uint8_t rx[12],tx[12];
	uint32_t cnt = 0;
	uint32_t val;
	unsigned short sum = 0;
	
	val = inl((unsigned int*)(BSP_UART2_BASE_ADDRESS + FIFO_THD));
	val = (val >> 24) & 0x3F;
	if(val > 30){
		for(cnt=0; cnt<32; cnt++){
			val = inl((unsigned int*)(BSP_UART2_BASE_ADDRESS + RS_DATA));
		}
	}
	
	cnt = 0;
	while(mcu_uart2_get_command(rx) == 0){
		cnt++;
		if(cnt > UART2_TIME_OUT){
			serial_puts("MCU get command timeout\n");
			return 0;
		}
	};
	
	//serial_printf("MCU get data 0x%x, 0x%x, 0x%x, 0x%x, 0x%x  \n", rx[0], rx[1], rx[2], rx[3], rx[4]);

	if(rx[0] == 0x01){
		tx[0] = 0x01;
		tx[1] = rx[1];
		tx[2] = data;
		tx[3] = 0x00;
		tx[4] = 0x00;
		tx[5] = 0x00;
		tx[6] = 0x00;
		tx[7] = 0x00;
		tx[8] = 0x00;
		tx[9] = 0x00;
		tx[10] = 0x00;
		sum = tx[0] + tx[1] + tx[2];
		if (sum > 255)
			sum++;

		tx[11] = sum & 0xff;

		//tx[0] = 0x80;
		//tx[2] = data;
		//tx[3] = rx[2];
		//tx[4] = 0x00;
		//tx[1] = tx[0] + tx[2] + tx[3] + tx[4];

		//set timer2 to delay 1ms
		outl((unsigned int*)((BSP_TIMER_BASE_ADDRESS + 0x10)), 10000);
		outl((unsigned int*)((BSP_TIMER_BASE_ADDRESS + 0x18)), 0);
		val = inl((unsigned int*)(BSP_TIMER_BASE_ADDRESS + 0x30));
		val |= 0x08;
		outl((unsigned int*)((BSP_TIMER_BASE_ADDRESS + 0x30)), val);
		cnt = 0;
		while((inl((unsigned int*)(BSP_TIMER_BASE_ADDRESS + 0x34))&0x08) == 0)
		{
			cnt++;
			if(cnt > UART2_TIME_OUT){
				serial_puts("MCU timer2 timeout\n");
				//return 0;
				break;
			}		
		}

		val &= ~0x08;
		outl((unsigned int*)((BSP_TIMER_BASE_ADDRESS + 0x30)), val);
		cnt = 0;		
		outl((unsigned int*)((BSP_TIMER_BASE_ADDRESS + 0x34)), (1 << 12));
		
		if(mcu_uart2_set_command(tx))
			return 1;
		else
			return 0;
	}
	
	return 0;
}

int mcu_set_stop_reset(void)
{
	return mcu_set_sync_status(0x10);
}

int mcu_set_all_reset(void)
{
	return mcu_set_sync_status(0x01);
}






static void mcu_uart2_init(void)
{
	unsigned int tmp = 0;

	//config UART2 clock/baudrate 115200/12=9600
	tmp = 0x58C;
	outl((unsigned int*)(BSP_UART2_BASE_ADDRESS + UART_CLOCK), tmp);
	
	//set FIFO threshold
	tmp = 0x0C0C;
	outl((unsigned int*)(BSP_UART2_BASE_ADDRESS + FIFO_THD), tmp);

	//config UART2 mode
	tmp = TX_MODE_UART|RX_MODE_UART|RS232_RX_INT_EN_BIT;

	//set UART2 config
	outl((unsigned int*)(BSP_UART2_BASE_ADDRESS + UART_CONFIG), tmp);
}

// For WDT


static int get_clock ()
{
	u32 pll800=0, ahb2_rate=0;

	pll800 = ((* (volatile unsigned int *) SNX_SYS_BASE) & 0x3f8000) >> 15;
	ahb2_rate = ((* (volatile unsigned int *) (SNX_SYS_BASE + 4)) & 0xf8000) >> 15;
		
	AHB2_CLOCK = (pll800 * 12 * 1000000) / ahb2_rate;
	
	return (0);
}

static int wd_reset ()
{
	u32 pll800=0, ahb2_rate=0;

	(* (volatile unsigned int *) (WD_BASE + 0x4)) = 1;
	(* (volatile unsigned int *) (WD_BASE + 0xc)) = 0x3;
			
	return (0);
}


int main(void)
{
/* check id */
	u32 platform_id;

	platform_id = (* (volatile unsigned int *) (SNX_SYS_BASE + 0x10)) & 0xfffff;
#if defined(CONFIG_SYSTEM_PLATFORM_ST58660FPGA) || defined(CONFIG_SYSTEM_PLATFORM_SN98660) || defined(CONFIG_SYSTEM_PLATFORM_SN98661) || defined (CONFIG_SYSTEM_PLATFORM_SN98670) || defined (CONFIG_SYSTEM_PLATFORM_SN98671) || defined (CONFIG_SYSTEM_PLATFORM_SN98672) || defined (CONFIG_SYSTEM_PLATFORM_SN98293)
	if (platform_id && (platform_id != 0x58660)) {
		serial_printf ("ERROR: id is not match, hardware platform id = %x, but image id is 58660\n", platform_id);
		return -1;
	}
#else
	if (platform_id && (platform_id != 0x58600)) {
		serial_printf ("ERROR: id is not match, hardware platform id = %x, but image id is 58600\n", platform_id);
		return -1;
	}
#endif

	IMAGE_TABLE_START = readl(SB_FWIMG_LOAD_ADDR) + SB_FWIMG_SIZE;
	IMAGE_TABLE_SIZE = readl(IMAGE_TABLE_START);

	serial_puts("----UPDATE FIRMWARE----\n");
	serial_printf("IMAGE_TABLE_START = 0x%x\n", IMAGE_TABLE_START);
	serial_printf("IMAGE_TABLE_SIZE = 0x%x\n", IMAGE_TABLE_SIZE);

	get_clock();
	serial_init();


#if FW_F 
serial_puts("FIRMWARE_F BURNNING START\n");
mcu_uart2_init();
if (mcu_set_stop_reset() == 0) {
	serial_puts("STOP MCU RESET ... FAILED!!\n");
}
#endif

	if(ms_serial_flash_init())
		return -1;
		
	if(ms_serial_flash_update(0,0)) {
		serial_puts("serial flash update failed\n");
		return -1;
	}

	serial_puts("serial flash update success\n");

#if FW_F 
if (mcu_set_all_reset() == 0) {
	serial_puts("MCU ALL RESET ... FAILED!!\n");
}
#endif

	wd_reset ();
	return 0;
}
