
	.code 32
	.text

	.align 5

	.long  0x20160825	@date
	.long  0x16000000	@time
	.long  1		@modes or res[0]
	.long  0x0		@res[1]
	.long  0x00058660	@uwICHeader
	.long  0x0		@uwChipId

	.short 648		@width
	.short 488		@height
	.short 30		@rate
	.short 0		@id
	.long __vga30_start
	.long __vga30_end

	.align 5
__vga30_start:
	.incbin "VGA30.bin"
__vga30_end:

	.end
