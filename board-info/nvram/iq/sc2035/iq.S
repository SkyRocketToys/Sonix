
	.code 32
	.text

	.align 5

	.long  0x20160530	@date
	.long  0x19000000	@time
	.long  1		@modes or res[0]
	.long  0x0		@res[1]
	.long  0x00058660	@uwICHeader
	.long  0x0		@uwChipId

	.short 1920		@width
	.short 1080		@height
	.short 30		@rate
	.short 0
	.long __fhd30_start
	.long __fhd30_end

	.align 5
__fhd30_start:
	.incbin "FHD30.bin"
__fhd30_end:

	.end

