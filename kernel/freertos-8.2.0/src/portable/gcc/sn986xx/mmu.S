#include "mmu.h"

	.global	__armv4_mmu_cache_on
	.global flush_dcache_area

__common_mmu_cache_on:
		stmfd	sp!, {lr}
#ifndef CONFIG_THUMB2_KERNEL
#ifndef DEBUG
		orr	r0, r0, #0x000d		@ Write buffer, mmu
#endif
		mov	r1, #-1
		mcr	p15, 0, r3, c2, c0, 0	@ load page table pointer
		mcr	p15, 0, r1, c3, c0, 0	@ load domain access control
		b	1f
		.align	5			@ cache line aligned
1:		mcr	p15, 0, r0, c1, c0, 0	@ load control register
		mrc	p15, 0, r0, c1, c0, 0	@ and read it back to
#		sub	pc, lr, r0, lsr #32	@ properly flush pipeline
#endif
		ldmfd	sp!, {pc}

__setup_mmu:
		stmfd	sp!, {lr}
/*
 * Initialise the page tables, turning on the cacheable and bufferable
 * bits for the RAM area only.
 */
		mov	r0, r3
		ldr	r2, =((configHEAP_CB_SIZE + configHEAP_PreCB_SIZE) >> 20)
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
		mov	r2, #configHEAP_CB_SIZE
		mov	r2, r2, lsr #20
		rsb	r2, r2, #0x1000
		
#		rsb	r2, r2, #configDDR_SIZE
#		mov	r2, r2, lsr #20
#		rsb	r2, r2, #(4 << 10)
##		ldr	r2, =((4 << 10) - (configHEAP_NCNB_SIZE >> 20))	@ Total 4GB = 4K * 1MB
		cmp	r2, #0
2:
		strgt	r1, [r0], #4		@ 1:1 mapping
		addgt	r1, r1, #1048576
		subs	r2, #1
		bgt	2b

		ldmfd	sp!, {pc}


__armv4_mmu_cache_on:
		stmfd	sp!, {lr}
#ifdef CONFIG_MMU
		ldr	r3, =__pagetable_begin	@ r0: image end
		bl	__setup_mmu
		mov	r0, #0
		mcr	p15, 0, r0, c7, c10, 4	@ drain write buffer
		mcr	p15, 0, r0, c8, c7, 0	@ flush I,D TLBs
		mrc	p15, 0, r0, c1, c0, 0	@ read control reg
		orr	r0, r0, #0x5000		@ I-cache enable, RR cache replacement
		orr	r0, r0, #0x0030
#ifdef CONFIG_CPU_ENDIAN_BE8
		orr	r0, r0, #1 << 25	@ big-endian page tables
#endif
		bl	__common_mmu_cache_on
		mov	r0, #0
		mcr	p15, 0, r0, c8, c7, 0	@ flush I,D TLBs
#endif
		ldmfd	sp!, {pc}

/*
 * flush_dcache_area(start_addr, size)
 */
flush_dcache_area:
	stmfd	sp!, {r4,lr}
#	ldr		r2, =ucHeap_ncnb 								@ ncnb start
#	ldr		r3, =(ucHeap_ncnb+configHEAP_NCNB_SIZE) 		@ ncnb size
	ldr		r2, =ucHeap_pre_ncnb 											@ ncnb start
	ldr		r3, =(ucHeap_pre_ncnb+configHEAP_NCNB_SIZE+configHEAP_PreNCNB_SIZE) @ ncnb size

	cmp 	r0, r3
	blt 	1f
	add		r4, r0, r1
	cmp 	r4, r3
	bge 	1f
	ldmfd	sp!, {r4,pc}
1:
	add		r1, r0, r1
	and		r0, r0, #0xffffffe0
2:
	mcr     p15, 0, r0, c7, c14, 1	@ Clean DCache single entry
	add     r0, r0, #32
	cmp     r0, r1
	blo     2b
	mov     r0, #0
	mcr     p15, 0, r0, c7, c10, 4	@ drain write buffer
	ldmfd	sp!, {r4,pc}


