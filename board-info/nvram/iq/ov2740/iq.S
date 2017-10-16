
	.code 32
	.text

	.align 5

	.long  0x20160801	@date
	.long  0x16000000	@time
	.long  1		@modes or res[0]
	.long  0x0		@res[1]
	.long  0x00058660	@uwICHeader
	.long  0x0		@uwChipId

	.short 1928		@width
	.short 1088		@height
	.short 30		@rate
	.short 0		@id
	.long __fhd30_start
	.long __fhd30_end

	.align 5
__fhd30_start:
	.incbin "FHD30.bin"
__fhd30_end:

	.end
