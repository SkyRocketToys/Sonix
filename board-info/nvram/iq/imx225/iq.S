
	.code 32
	.text

	.align 5

	.long  0x20151228	@date
	.long  0x19163900	@time
	.long  1		@modes or res[0]
	.long  0x0		@res[1]
	.long  0x00058660	@uwICHeader
	.long  0x0		@uwChipId

	.short 1280		@width
	.short 724		@height
	.short 30		@rate
	.short 0
	.long __hd30_start
	.long __hd30_end


	.align 5
__hd30_start:
	.incbin "HD30.bin"
__hd30_end:

	.end

