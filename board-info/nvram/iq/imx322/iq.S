
	.code 32
	.text
	.align 5

	.long  0x20151228	@date
	.long  0x19163900	@time
	.long  2		@modes or res[0]
	.long  0x0		@res[1]
	.long  0x00058660	@uwICHeader
	.long  0x0		@uwChipId

	.short 1984		@width
	.short 1105		@height
	.short 30		@rate
	.short 0
	.long __fhd30_start
	.long __fhd30_end

	.short 1344		@width
	.short 745		@height
	.short 60		@rate
	.short 0		@id
	.long __hd60_start
	.long __hd60_end

	.align 5
__fhd30_start:
	.incbin "FHD30.bin"
__fhd30_end:

	.align 5
__hd60_start:
	.incbin "HD60.bin"
__hd60_end:

	.end

