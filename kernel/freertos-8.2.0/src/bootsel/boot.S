/*
Copyright 2013, Jernej Kovacic

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

.section .init
.code 32                                   @ 32-bit ARM instruction set

@ This symbol must be visible to the linker
.global vectors_start
.global do_crc16_on_mem
.global sf_end_read_data
.global sf_read_data32
.global sf_start_read_data
.global sf_load_image
.global execute_rtos
.global sf_read_flashlayout

.global		__common_mmu_cache_on
.global 	__setup_mmu
.global 	__armv4_mmu_cache_on



vectors_start:
	@ Exception vectors, relative to the base address, see page 2-26 of DDI0222
	LDR pc, reset_handler_addr             @ Reset (and startup) vector
	LDR pc, undef_handler_addr             @ Undefined (unknown) instruction
	LDR pc, swi_handler_addr               @ Software interrupt
	LDR pc, prefetch_abort_handler_addr    @ Prefetch abort
	LDR pc, data_abort_handler_addr        @ Data abort (system bus cannot access a peripheral)
	LDR pc, invalid_addr_handler           @ Reserved (early ARM only supported 26-bit addresses)
	LDR pc, irq_handler_addr               @ IRQ handler
	LDR pc, fiq_handler_addr               @ FIQ handler

@ Labels with addresses to exception handler routines, referenced above:
reset_handler_addr:
	.word reset_handler
undef_handler_addr:
	.word undef_handler
swi_handler_addr:
	.word undef_handler
prefetch_abort_handler_addr:
	.word undef_handler
data_abort_handler_addr:
	.word undef_handler
invalid_addr_handler:
	.word undef_handler
irq_handler_addr:
	.word undef_handler
fiq_handler_addr:
	.word unhandled

vectors_end:

/*.text*/
.code 32
/*
 * Implementation of the reset handler, executed also at startup.
 * It sets stack pointers for all supported operating modes (Supervisor,
 * IRQ and System), disables IRQ iand FIQ nterrupts for all modes and finally
 * it jumps into the startup function.
 *
 * Note: 'stack_top', 'irq_stack_top' and 'svc_stack_top' are allocated in qemu.ld
 */
reset_handler:
	@ The handler is always entered in Supervisor mode
@	LDR sp, =svc_stack_top                 @ stack for the supervisor mode
@	LDR sp, =0x0FFFFFC
	LDR sp, =0xEFFFFC

	BL	detect_update_mode		@ detect update mode	
	CMP	r0, #1
	BLNE	rescue

	LDR r2, = 0xFFFF4004
	LDR	r2, [r2]
	LDR	r1, = 0x66666666
	CMP	r1, r2
	BLEQ	rescue


	LDR r2, = NOW_RTOS_STATUS
	LDR	r2, [r2]
	LDR	r1, = NONZIPIMG
	CMP	r1, r2
	BNE	zipim

	LDR	r11, =BASE_MS1

	LDR	r11, =BASE_MS1
	BL	sf_read_flashlayout

	LDR r0, = NOW_RTOS_STATUS 	@ normal 
	LDR r1, = 0x6e6f726d
	STR	r1, [r0]

	LDR	r0, =LOAD_IMAGE_ADDR_NL
	LDR	r1, =SF_UIMAGE_ADDR_NL			@ flash zip image addr
	LDR	r2, =LOAD_IMAGE_SIZE_NL 		@ zip image size
	BL	sf_load_image
	BL	execute_rtos

	B fai

zipim:
	@ Clear the whole BSS section to 0:
	LDR		r0, __bss_begin_addr
	LDR		r1, __bss_end_addr
	MOV		r2, #0
bss_clear_loop:
	CMP		r0, r1                     @ if (r0<r1) ....
	STRLTB	r2, [r0], #1            	@ ...store a byte of r2 (i.r. 0) to location pointed by r0++
	BLT		bss_clear_loop             @ ...and continue the loop


	LDR r0, = 0x98A00010
	LDR r1, ='b'
	STR	r1, [r0]
	LDR r1, ='o'
	STR	r1, [r0]
	LDR r1, ='o'
	STR	r1, [r0]
	LDR r1, ='t'
	STR	r1, [r0]
	LDR r1, ='s'
	STR	r1, [r0]
	LDR r1, ='e'
	STR	r1, [r0]
	LDR r1, ='l'
	STR	r1, [r0]
	LDR r1, ='.'
	STR	r1, [r0]
	LDR r1, ='.'
	STR	r1, [r0]
	LDR r1, ='.'
	STR	r1, [r0]


	LDR r2, = NOW_RTOS_STATUS
	LDR	r2, [r2]
	LDR	r1, = BYPASSTAG
	CMP	r1, r2
	BNE	normal

	LDR r1, ='b'
	STR	r1, [r0]

	LDR r1, ='y'
	STR	r1, [r0]

	LDR r1, ='p'
	STR	r1, [r0]

	LDR r1, ='s'
	STR	r1, [r0]

	LDR r1, ='s'
	STR	r1, [r0]

	B	bypass


normal:
	LDR r0, = 0x98700004		@ 2s watchdog timer 
	LDR r1, = 0x1312D00
	STR	r1, [r0]

	LDR r0, = 0x98700008
	LDR r1, = 0x5AB9
	STR	r1, [r0]

	LDR r0, = 0x9870000C
	LDR r1, = 0x13
	STR	r1, [r0]

	LDR r0, = 0xFFFF4004
	LDR r1, = 0x66666666
	STR	r1, [r0]

	@ Enable I-cache
	mrc		p15, 0, r0, c1, c0, 0
	orr		r0, r0, #0x1000
	mcr		p15, 0, r0, c1, c0, 0

	LDR	r11, =BASE_MS1

	LDR	r11, =BASE_MS1
	BL	sf_read_flashlayout

	LDR r0, = NOW_RTOS_STATUS 	@ normal 
	LDR r1, = 0x6e6f726d
	STR	r1, [r0]

	LDR	r0, =LOAD_ZIMAGE_ADDR_NL 		@ ddr zip image addr (df 0x1800000)
	LDR	r1, =SF_UIMAGE_ADDR_NL			@ flash zip image addr
	LDR	r2, =LOAD_IMAGE_SIZE_NL 		@ zip image size
	BL	sf_load_image
	CMP	r0, #0
	BNE rescue

bypass:
	@ Enable I-cache
	mrc		p15, 0, r0, c1, c0, 0
	orr		r0, r0, #0x1000
	mcr		p15, 0, r0, c1, c0, 0

	@ Enable D-cache & MMU
	BL		__armv4_mmu_cache_on

	@ Clear the whole BSS section to 0:
	LDR		r0, __bss_begin_addr
	LDR		r1, __bss_end_addr
	MOV		r2, #0
1:
	CMP		r0, r1         	@ if (r0<r1) ....
	STRLTB	r2, [r0], #1  	@ ...store a byte of r2 (i.r. 0) to location pointed by r0++
	BLT		1b             	@ ...and continue the loop


2:      
	mrc     p15, 0, r15, c7, c14, 3         @ test,clean,invalidate
    bne     2b

    mov     r0, #0
	mcr     p15, 0, r0, c7, c10, 4	@ drain write buffer
     


	LDR	r3, =LOAD_ZIMAGE_ADDR_NL 		@ ddr zip image addr (df 0x1800000)
	LDR	r0, [r3]
	LDR	r2, [r0], #4
	ADD	r2, #0x80000

	LDR	r3, =JUMP_RTOS_ADDR  		@ ddr d_zip image addr        
	LDR	r1, [r3]
	BL 	gzip
	CMP	r0, #1
	BLNE	execute_rtos

rescue:
	LDR sp, =0xEFFFFC

	LDR	r11, =BASE_MS1
	BL	sf_read_flashlayout

	@ Enable I-cache
	mrc		p15, 0, r0, c1, c0, 0
	orr		r0, r0, #0x1000
	mcr		p15, 0, r0, c1, c0, 0

	@ Enable D-cache & MMU
	BL		__armv4_mmu_cache_on


	@ Clear the whole BSS section to 0:
	LDR		r0, __bss_begin_addr
	LDR		r1, __bss_end_addr
	MOV		r2, #0
1:
	CMP		r0, r1         	@ if (r0<r1) ....
	STRLTB	r2, [r0], #1  	@ ...store a byte of r2 (i.r. 0) to location pointed by r0++
	BLT		1b             	@ ...and continue the loop


2:      
	mrc     p15, 0, r15, c7, c14, 3         @ test,clean,invalidate
    bne     2b

    mov     r0, #0
	mcr     p15, 0, r0, c7, c10, 4	@ drain write buffer
     

	LDR r0, = 0x98A00010
	LDR r1, ='r'
	STR	r1, [r0]
	LDR r1, ='e'
	STR	r1, [r0]
	LDR r1, ='s'
	STR	r1, [r0]
	LDR r1, ='c'
	STR	r1, [r0]
	LDR r1, ='u'
	STR	r1, [r0]
	LDR r1, ='e'
	STR	r1, [r0]
	LDR r1, ='.'
	STR	r1, [r0]
	LDR r1, ='.'
	STR	r1, [r0]
	LDR r1, ='.'
	STR	r1, [r0]

	LDR r0, = NOW_RTOS_STATUS 	@ normal 
	LDR r1, = 0x72657363
	STR	r1, [r0]

	LDR	r11, =BASE_MS1

	LDR	r0, =LOAD_ZIMAGE_ADDR_NL
	LDR	r1, =SF_UIMAGE_ADDR_RS
	LDR	r2, =LOAD_IMAGE_SIZE_RS	
	BL	sf_load_image
	CMP	r0, #0
	BNE fai

	LDR	r3, =LOAD_ZIMAGE_ADDR_NL 		@ ddr zip image addr (df 0x1800000)
	LDR	r0, [r3]
	LDR	r2, [r0], #4
	ADD	r2, #0x80000

	LDR	r3, =JUMP_RTOS_ADDR  		@ ddr d_zip image addr        
	LDR	r1, [r3]
	BL 	gzip
	CMP	r0, #1
	BLNE	execute_rtos

fai:
	LDR r0, = 0x98A00010
	LDR r1, ='c'
	STR	r1, [r0]
	LDR r1, ='r'
	STR	r1, [r0]
	LDR r1, ='a'
	STR	r1, [r0]
	LDR r1, ='s'
	STR	r1, [r0]
	LDR r1, ='h'
	STR	r1, [r0]
	LDR r1, ='.'
	STR	r1, [r0]
	LDR r1, ='.'
	STR	r1, [r0]
	LDR r1, ='.'
	STR	r1, [r0]

bootfail:
	B	bootfail



unhandled:
	B .					@ infinite loop for unsupported exceptions

undef_handler:
	B .										@ infinite loop for unsupported exceptions

prefetch_abort_handler:
	B .

data_abort_handler:
	B .

invalid_addr:
	B .

/*-------------------------------------------------------------------------------
* void execute_rtos(void);
* copy partial self from flash to RAM and remap address space
*-------------------------------------------------------------------------------*/
execute_rtos:
	LDR	r1, =JUMP_RTOS_ADDR
	LDR	r0, [r1]

@	LDR r0, = 0x00008000


	CMP	r0, #0
	MOVEQ	r0, #DEFAULT_JUMP_ADDR

	LDR	sp, =svc_stack_top			@ Setup the stack
	MOV	pc, r0



/*-------------------------------------------------------------------------------
* UINT32 sf_load_image (void);
* load u-boot from serial flash
* - return value (r0)	- 0: Succeed, -1: Fail
*-------------------------------------------------------------------------------*/
.equ  BASE_MS1,						0x90900000
.equ  MS_CTL,						0x00
.equ  MS_RDY_MASK,					8
.equ  MS_CACHE_CMD,					0x24
.equ  MS_DMA_ADDR,					0x5C
.equ  SPI_CMD,						0x3C
.equ  MS_CACHE_R_CMD_MASK,			0x0000FF00
.equ  SF_READ_CMD,					0x03
.equ  MS_CACHE_R_CMD_BIT,			8
.equ  MS_DMA_SIZE,					0x04
.equ  MS_DMA_BLOCK,					0x48
.equ  MS_MDMA_ECC,					0x50
.equ  MS_M_DMA_EN_MASK,				(1 << 0)
.equ  MS_DMA_RW_MASK,				(1 << 5)
.equ  MS_DMA_EN_MASK,				(1 << 4)
.equ  MS_M_DMA_OK_MASK,				(1 << 1)
.equ  MS_M_DMA_TIME_OUT_MASK,		(1 << 2)
.equ  SZ_128K,						(1 << 17)		@ 0x00020000

.equ  SF_CS_DISABLE,				1
.equ  MS_IO_3_BIT,					3 				@ CS

.equ  BIT3,							(1 << 3)
.equ  MS_IO_O,						0x34
.equ  MS_IO_3_MASK,					BIT3
.equ  MS_REG_RW_MASK,				BIT3
.equ  MS_REG_DATA,					0x74

.equ  SZ_4K,						(1 << 12)	 			@ 0x00001000
.equ  SZ_8K,						(1 << 13)	 			@ 0x00002000
.equ  SZ_256K,						(1 << 18)	 			@ 0x00040000
.equ  ITCM_ZI_OFFSET,				SZ_8K					@ 0x2000

.equ  ITCM_START,					0xffff4000						@ [13:12] needs to be 0
.equ  ITCM_ZI_OFFSET,				SZ_8K							@ 0x2000
.equ  ITCM_ZI,						(ITCM_START + ITCM_ZI_OFFSET)

.equ  LOAD_IMAGE_ADDR_NL,			(ITCM_ZI + 0x128)
.equ  SF_UIMAGE_ADDR_NL,			(ITCM_ZI + 0x12c)
.equ  LOAD_IMAGE_SIZE_NL,			(ITCM_ZI + 0x130)


.equ  LOAD_IMAGE_ADDR_RS,			(ITCM_ZI + 0x134)
.equ  SF_UIMAGE_ADDR_RS,			(ITCM_ZI + 0x138)
.equ  LOAD_IMAGE_SIZE_RS,			(ITCM_ZI + 0x13c)

.equ  JUMP_RTOS_ADDR,				(ITCM_ZI + 0x140)
.equ  NOW_RTOS_STATUS,				(ITCM_ZI + 0x144) 			@rescue= 0x72657363 , normal=0x6e6f726d

.equ  LOAD_ZIMAGE_ADDR_NL,			(ITCM_ZI + 0x148)
.equ  LOAD_DZIMAGE_SIZE_NL,			(ITCM_ZI + 0x14c)
.equ  LOAD_DZIMAGE_SIZE_RS,			(ITCM_ZI + 0x150)

.equ  FLASH_LAYOUT_STR,				(ITCM_ZI + 0x154)

.equ  DEFAULT_JUMP_ADDR,			0x00008000	@ after remap
.equ  DEFAULT_LOAD_ADDR,			0x00007ffc	@ after remap
.equ  DEFAULT_SF_IMAGE_ADDR,		SZ_4K		@ 4K
.equ  DEFAULT_SF_IMAGE_SZIE,		SZ_256K		@ 256K

.equ  BYPASSTAG,					(0x73737062)
.equ  NONZIPIMG,					(0x70697a6e)

sf_load_image:
	PUSH	{lr}

	LDR	r0, [r0]			@ r0 = dst addr on ddr
	LDR	r1, [r1]			@ r1 = src addr on sf
	LDR	r2, [r2]			@ r2 = size in byte

	LDR	r3, = DEFAULT_LOAD_ADDR
	CMP	r0, #0

	MOVEQ	r0, r3

	CMP	r1, #0
	MOVEQ	r1, #DEFAULT_SF_IMAGE_ADDR	@ default sf addr = 4K
	CMP	r2, #0
	MOVEQ	r2, #DEFAULT_SF_IMAGE_SZIE	@ default size = 256K


	@ Set src and dst addr
	LDR	r3, [r11, #MS_CACHE_CMD]
	STR	r0, [r11, #MS_DMA_ADDR]		@ dst addr
	STR	r1, [r11, #SPI_CMD]		    @ src addr

	MOV r6, r0
	MOV r7, r1


	@ Set read cmd
	BIC	r3, #MS_CACHE_R_CMD_MASK
	ORR	r3, r3, #(SF_READ_CMD << MS_CACHE_R_CMD_BIT)
	STR	r3, [r11, #MS_CACHE_CMD]

	LDR	r1, =0x1FFFF
	STR	r1, [r11, #MS_DMA_SIZE]

	LSR	r5, r2, #17
	CMP	r5, #0
	SUBNE	r5, #1

	STR	r5, [r11, #MS_DMA_BLOCK]
	ADD r5, #1
	LSL r5, r5, #17
	ADD r6, r6, r5
	ADD r7, r7, r5
	SUB r5, r2, r5


	@ Enable Multi-DMA
	LDR	r1, [r11, #MS_MDMA_ECC]
	LDR	r2, [r11, #MS_CTL]
	ORR	r1, #MS_M_DMA_EN_MASK
	STR	r1, [r11, #MS_MDMA_ECC]		@ MS_M_DMA_EN = 1
	ORR	r2, #MS_DMA_RW_MASK			@ DMA read mode
	ORR	r2, #MS_DMA_EN_MASK
	STR	r2, [r11, #MS_CTL]			@ MS_DMA_EN = 1

	/*---------------------------------------
	 * Wait for DMA finish
	 *--------------------------------------- */
	@ check MS1 ready
2:	LDR	r3, [r11, #MS_CTL]		@ ms1_ready
	TST	r3, #MS_RDY_MASK
	BEQ	2b


3:	LDR	r1, [r11, #MS_MDMA_ECC]
	TST	r1, #MS_M_DMA_OK_MASK
	TSTEQ	r1, #MS_M_DMA_TIME_OUT_MASK		@ check time out flag
	BEQ	3b

	@ Disable Multi-DMA
	LDR	r2, [r11, #MS_CTL]
	BIC	r1, #MS_M_DMA_EN_MASK
	BIC	r2, #MS_DMA_EN_MASK
	STR	r2, [r11, #MS_CTL]			@ #MS_DMA_EN = 0
	STR	r1, [r11, #MS_MDMA_ECC]		@ #MS_M_DMA_EN = 0

	/*******************************************/
	STR	r5, [r11, #MS_DMA_SIZE]
	MOV r5, #0
	STR	r5, [r11, #MS_DMA_BLOCK]
	STR	r6, [r11, #MS_DMA_ADDR]		@ dst addr
	STR	r7, [r11, #SPI_CMD]		    @ src addr

	@ Enable Multi-DMA
	LDR	r1, [r11, #MS_MDMA_ECC]
	LDR	r2, [r11, #MS_CTL]
	ORR	r1, #MS_M_DMA_EN_MASK
	STR	r1, [r11, #MS_MDMA_ECC]		@ MS_M_DMA_EN = 1
	ORR	r2, #MS_DMA_RW_MASK			@ DMA read mode
	ORR	r2, #MS_DMA_EN_MASK
	STR	r2, [r11, #MS_CTL]			@ MS_DMA_EN = 1

	@ check MS1 ready
5:	LDR	r3, [r11, #MS_CTL]		@ ms1_ready
	TST	r3, #MS_RDY_MASK
	BEQ	5b

6:	LDR	r1, [r11, #MS_MDMA_ECC]
	TST	r1, #MS_M_DMA_OK_MASK
	TSTEQ	r1, #MS_M_DMA_TIME_OUT_MASK		@ check time out flag
	BEQ	6b


	@ Disable Multi-DMA
	LDR	r2, [r11, #MS_CTL]
	BIC	r1, #MS_M_DMA_EN_MASK
	BIC	r2, #MS_DMA_EN_MASK
	STR	r2, [r11, #MS_CTL]			@ #MS_DMA_EN = 0
	STR	r1, [r11, #MS_MDMA_ECC]		@ #MS_M_DMA_EN = 0


	@ Issues of register map
	MOV	r2, #0
	STR	r2, [r11, #MS_DMA_SIZE]

	@ check time out fail
	TST	r1, #MS_M_DMA_TIME_OUT_MASK
	BNE	sf_load_fail


	MOV	r0, #0				@ return 0
	POP	{pc}

	/*----------------------------------
	 * Check CRC16
	 *----------------------------------*/
	LDR	r1, [r0], #4			@ r0 = addr, r1 = size

	@ Check size
	TST	r1, #0xFE000000			@ check < 32Mb
	TSTEQ	r1, #0x3 			@ check align 4 bytes
	BNE	sf_load_fail

	@ Get org CRC16 value
	SUB	r1, #4					@ subtract 1 word crc16 value
	ADD	r2, r0, r1
	LDR	r2, [r2]				@ r2 = crc16 value

	@ Do CRC16, r0 = addr, r1 = size, r2 = crc16 value
	BL	do_crc16_on_mem
	CMP	r0, #0

sf_load_fail:
	MVNNE	r0, #0				@ return -1
	POP	{pc}



/*----------------------------------
* sf_read_mdma
*	r0 = dst addr on ddr
*	r1 = src addr on sf
*	r2 = size in 4 byte
*----------------------------------*/
sf_read_mdma:
	PUSH	{lr}

	@ Set src and dst addr
	LDR	r3, [r11, #MS_CACHE_CMD]
	STR	r0, [r11, #MS_DMA_ADDR]		@ dst addr
	STR	r1, [r11, #SPI_CMD]		    @ src addr

	MOV r6, r0
	MOV r7, r1


	@ Set read cmd
	BIC	r3, #MS_CACHE_R_CMD_MASK
	ORR	r3, r3, #(SF_READ_CMD << MS_CACHE_R_CMD_BIT)
	STR	r3, [r11, #MS_CACHE_CMD]

	LDR	r1, =0x3
	STR	r1, [r11, #MS_DMA_SIZE]

	LSR	r5, r2, #2
	CMP	r5, #0
	SUBNE	r5, #1

	STR	r5, [r11, #MS_DMA_BLOCK]
	ADD r5, #1
	LSL r5, r5, #2
	ADD r6, r6, r5
	ADD r7, r7, r5
	SUB r5, r2, r5


	@ Enable Multi-DMA
	LDR	r1, [r11, #MS_MDMA_ECC]
	LDR	r2, [r11, #MS_CTL]
	ORR	r1, #MS_M_DMA_EN_MASK
	STR	r1, [r11, #MS_MDMA_ECC]		@ MS_M_DMA_EN = 1
	ORR	r2, #MS_DMA_RW_MASK			@ DMA read mode
	ORR	r2, #MS_DMA_EN_MASK
	STR	r2, [r11, #MS_CTL]			@ MS_DMA_EN = 1

	/*---------------------------------------
	 * Wait for DMA finish
	 *--------------------------------------- */
	@ check MS1 ready
2:	LDR	r3, [r11, #MS_CTL]		@ ms1_ready
	TST	r3, #MS_RDY_MASK
	BEQ	2b


3:	LDR	r1, [r11, #MS_MDMA_ECC]
	TST	r1, #MS_M_DMA_OK_MASK
	TSTEQ	r1, #MS_M_DMA_TIME_OUT_MASK		@ check time out flag
	BEQ	3b

	@ Disable Multi-DMA
	LDR	r2, [r11, #MS_CTL]
	BIC	r1, #MS_M_DMA_EN_MASK
	BIC	r2, #MS_DMA_EN_MASK
	STR	r2, [r11, #MS_CTL]			@ #MS_DMA_EN = 0
	STR	r1, [r11, #MS_MDMA_ECC]		@ #MS_M_DMA_EN = 0

	MOV	r0, #0					@ return 0
	POP	{pc}




sf_read_flashlayout:
	PUSH	{lr}

	LDR	r0, = 0x00900000
	LDR	r1, = FLASH_LAYOUT_STR
	LDR	r1, [r1]
	LDR	r2, = 0x40

	BL	sf_read_mdma

	LDR	r0, = 0x00900000
	LDR	r1, [r0, #0x18]
	LDR	r2, = SF_UIMAGE_ADDR_NL
	STR	r1, [r2]

	LDR	r0, = 0x00900000
	LDR	r1, [r0, #0x18]
	LDR	r2, = 4

	BL	sf_read_mdma

	LDR	r0, = 0x00900000
	LDR	r0, [r0]
	ADD	r0, r0, #4

	LDR	r2, = LOAD_IMAGE_SIZE_NL
	STR	r0, [r2]

	MOV	r0, #0					@ return 0
	POP	{pc}


/*-----------------------------------------------------------------------------------
 * UINT32 do_crc16_on_mem(UINT32 start_addr, UINT32 size, UINT32 crc_val);
 * CRC16 check for the data on the sequence memory
 * - start_addr (r0)	- start address of data [4 words aligned]
 * - size (r1)		- the size of data [4 words aligned]
 * - crc_val (r2)	- CRC16 calculate value
 * - return value (r0)	- crc16 check result (0:OK, -1:Fail)
 *----------------------------------------------------------------------------------- */
.equ  BASE_CRC16,				0x90e00000
.equ  CRC16_START_ADDR,			0x10
.equ  CRC16_SIZE,				0x18
.equ  CRC16_CTRL,				0x00
.equ  FUNCTION_MODE_MASK,		0x000000F0
.equ  CRC16_DRAM_MODE,			0x2
.equ  FUNCTION_MODE_BIT,		4
.equ  ENABLE_MASK,				0x00000001
.equ  CRC16_READ_DATA_MASK,		0x00FFFF00
.equ  CRC16_READ_DATA_BIT,		8


do_crc16_on_mem:
	LDR	r3, =BASE_CRC16

	@ set start address of data
	STR	r0, [r3, #CRC16_START_ADDR]

	@ set size of data
	SUB	r1, r1, #0x10
	STR	r1, [r3, #CRC16_SIZE]

	@ set crc16 ctrl
	LDR	r0, [r3, #CRC16_CTRL]
	BIC	r0, r0, #FUNCTION_MODE_MASK
	ORR	r0, r0, #(CRC16_DRAM_MODE << FUNCTION_MODE_BIT)		@ set crc16 dram mode
	ORR	r0, r0, #ENABLE_MASK								@ enable crc16
	STR	r0, [r3, #CRC16_CTRL]

	@ check whether crc calculate finish
1:	LDR	r0, [r3, #CRC16_CTRL]
	ANDS	r1, r0, #ENABLE_MASK
	BNE	1b

	@ get crc calculate result
	LDR	r1, =CRC16_READ_DATA_MASK
	AND	r0, r0, r1

	CMP	r0, r2, LSL #CRC16_READ_DATA_BIT
	MOVEQ	r0, #0									@ crc16_check_ok
	MOVNE	r0, #-1									@ crc16_check_fail
	MOV	pc, lr


@ -------------------------------------------------------------------------------
@ 	void detect_update_mode (void)
@ 	detect update mode by gpio2
@ 	return GPIO status, 1: High, 0: Low
@ -------------------------------------------------------------------------------
.equ  BASE_GPIO,			0x98100000
.equ  GPIO_O,				0x0
.equ  GPIO_I,				0x4
.equ  GPIO_OE,				0x8
.equ  GPIO_O_SET,			0x10
.equ  GPIO_O_CLR,			0x14

.equ  BUTTON_THRESHOLD,				0x3			@ 3 times for non-active
.equ  BUTTON_LOOP,					0x10		@ 10 loop
.equ  BUTTON_INTERVAL,				0x18400		@ 0x18400 = 99328 := 100000 = (1ms * 100MHz)
.equ  BUTTON_TWICE_THRESHOLD,		0x100		@ 256

.equ  UPDATE_DETECT_GPIO,		2		@ BIT2


detect_update_mode:
	PUSH	{r4-r5, lr}
	LDR	r1, =BASE_GPIO
@	LDR	r2, =ITCM_GPIO2_DETECT
	MOV	r3, #BUTTON_THRESHOLD
	MOV	r4, #BUTTON_LOOP

	@ Set GPIO2 input mode
	LDR	r0, [r1, #GPIO_OE]
	BIC r0, r0, #4
	STR	r0, [r1, #GPIO_OE]


	@ Read update detect GPIO, low active
1:	LDR	r0, [r1, #GPIO_I]
	LSR	r0, r0, #UPDATE_DETECT_GPIO
	ANDS	r0, r0, #1
	SUBNES	r3, r3, #1				@ count down for high
	BEQ	detect_end
	MOV	r5, #BUTTON_INTERVAL		@ almost 1ms
@	CPUDLY	r5
2:	SUBS    r5, r5, #4
	BHI     2b

	SUBS	r4, r4, #1
	BNE	1b
detect_end:
@	STR	r0, [r2]			@ write status to TCM
	POP	{r4-r5, pc}			@ return status


/*-------------------------------------------------------------------------------
 * MMU and d cache
 *-------------------------------------------------------------------------------*/
#.equ  BTSEL_HEAP_CB_SIZE,					0x01800000
#.equ  BTSEL_DDR_SIZE,					0x02000000
.equ  BTSEL_HEAP_CB_SIZE,					0xF00000
.equ  BTSEL_DDR_SIZE,					0x01000000

__common_mmu_cache_on:
		stmfd	sp!, {lr}


		orr	r0, r0, #0x000d		@ Write buffer, mmu

		mov	r1, #-1
		mcr	p15, 0, r3, c2, c0, 0	@ load page table pointer
		mcr	p15, 0, r1, c3, c0, 0	@ load domain access control
		b	1f
		.align	5			@ cache line aligned
1:		mcr	p15, 0, r0, c1, c0, 0	@ load control register
		mrc	p15, 0, r0, c1, c0, 0	@ and read it back to
#		sub	pc, lr, r0, lsr #32	@ properly flush pipeline

		ldmfd	sp!, {pc}

__setup_mmu:
		stmfd	sp!, {lr}
/*
 * Initialise the page tables, turning on the cacheable and bufferable
 * bits for the RAM area only.
 */
		mov	r0, r3
		ldr	r2, =(BTSEL_HEAP_CB_SIZE >> 20)
		mov	r1, #0x12
		orr	r1, r1, #3 << 10	@ AP: Manager mode
		orr	r1, r1, #0x0c		@ set cacheable, bufferable

		cmp	r2, #0
1:
		strgt	r1, [r0], #4		@ 1:1 mapping
		addgt	r1, r1, #1048576
		subs	r2, #1
		bgt	1b

		bic	r1, r1, #0x0c		@ clear cacheable, bufferabl
		mov	r2, #BTSEL_HEAP_CB_SIZE
		mov	r2, r2, lsr #20
		rsb	r2, r2, #0x1000

#		rsb	r2, r2, #BTSEL_DDR_SIZE
#		mov	r2, r2, lsr #20
#		rsb	r2, r2, #(4 << 10)
#		ldr	r2, =((4 << 10) - (configHEAP_NCNB_SIZE >> 20))	@ Total 4GB = 4K * 1MB
		cmp	r2, #0
2:
		strgt	r1, [r0], #4		@ 1:1 mapping
		addgt	r1, r1, #1048576
		subs	r2, #1
		bgt	2b

		ldmfd	sp!, {pc}

__armv4_mmu_cache_on:
		stmfd	sp!, {lr}

		ldr	r3, =__pagetable_begin	@ r0: image end
		bl	__setup_mmu
		mov	r0, #0
		mcr	p15, 0, r0, c7, c10, 4	@ drain write buffer
		mcr	p15, 0, r0, c8, c7, 0	@ flush I,D TLBs
		mrc	p15, 0, r0, c1, c0, 0	@ read control reg
		orr	r0, r0, #0x5000		@ I-cache enable, RR cache replacement
		orr	r0, r0, #0x0030
#ifdef CONFIG_CPU_ENDIAN_BE8
#		orr	r0, r0, #1 << 25	@ big-endian page tables
#endif
		bl	__common_mmu_cache_on

		@ flush v4 I/D caches
		MOV	r0, #0
		MCR	p15, 0, r0, c8, c7, 0		@ flush v4 TLB

		ldmfd	sp!, {pc}


@ Addresses of BSS begin and end.
@ Note that both symbols have been defined in the linker script
__bss_begin_addr:
	.word __bss_begin
__bss_end_addr:
	.word __bss_end


.end

