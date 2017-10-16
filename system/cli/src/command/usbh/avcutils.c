/*
 * avcutils.c
 *
 * @description:
 *    Providing H.264 related utility functions here.
 * @author:
 *    Evan Chang
 * @date:
 *    2015, Dec
 * @version:
 *    v1.0.0
 */
#include <stdio.h>
#include <nonstdlib.h>
#include "avcutils.h"


#define _DEBUG_VERBOSE		0
#define assert(x)

enum {
	SLICE_PIC	= 1,
	SLICE_PA,
	SLICE_PB,
	SLICE_PC,
	SLICE_IDR_PIC,
	SEI,
	SPS,
	PPS,
};

/**
 * H.264 utils
 */
typedef struct bits_handler {
	const char *ptr, *base;
	unsigned length;
	int index;
} bits_handler;

static void bits_handler_init(bits_handler *bs, char *buffer,
		unsigned buffer_length)
{
	bs->ptr = bs->base = buffer;
	bs->length = buffer_length;
	bs->index = 0;
}

/**
 * read_bits
 *
 * @descript Read bits.
 * @param
 * 	bs: bitstream
 * 	bits: number of bits needs to read, support 1 ~ 32.
 */
static unsigned long read_bits(bits_handler *bs, int bits)
{
	int valid_bits = 8 - bs->index;
	int remain_bits = bits - valid_bits;
	unsigned long v = 0;
	int bytealign, i;

	assert(bits < 33);

	if (!(bits > valid_bits)) {
		v = ((*bs->ptr >> (valid_bits - bits)) & ((1 << bits) -1));
		bs->index += bits;
		return v;
	}

	v = (*bs->ptr++ & ((1 << valid_bits) - 1)) << 8;
	bs->index = 0;
	bytealign = ((remain_bits | 0x07) & ~(unsigned long)0x07) >> 3;
	for (i=0; i<bytealign; ++i) {
		//v = (v | *bs->ptr++) << 8;
		if (remain_bits > 8) {
			v = (v | *bs->ptr++) << 8;
			remain_bits -= 8;
		} else {
			v = (v | *bs->ptr) >> (8 - remain_bits);
			bs->index = remain_bits;
		}
	}

	return v;
}

static unsigned long ue_exp_golomb(bits_handler *bs)
{
	int leading_zero_bits = -1;
	int b;

	for (b = 0; !b; ++leading_zero_bits) {
		b = read_bits(bs, 1);
	}

	return (1 << leading_zero_bits) - 1 + read_bits(bs, leading_zero_bits);
}

/*
 * static function slice_type
 * @description
 * 	Return slice type in slice header syntax.
 * @return
 * 	Non-zero if slice type is valid, otherwise return -1 if not a slice,
 * 	return
 */
static int get_slice_type(char *ptr, unsigned remain_sz)
{
	char *p = NULL;
	unsigned short nal_unit_type;
	int slice_type = NOT_SLICE;

	// 4 bytes start code and 4 bytes content should be parsed
	if (remain_sz < 8) {
#if _DEBUG_VERBOSE
		print_msg_queue("%s: remain data size not enough\n", __func__);
#endif
		return ERROR;
	}

	p = ptr;
	p += 4;
	nal_unit_type = (*p++) & 0x1F;
	switch(nal_unit_type) {
		case SLICE_PIC:
		{
			bits_handler bs;
			unsigned first_mb_in_slice;
			unsigned pic_parameter_set_id;

			bits_handler_init(&bs, p, 32);
			first_mb_in_slice = ue_exp_golomb(&bs);
			slice_type = ue_exp_golomb(&bs);
			pic_parameter_set_id = ue_exp_golomb(&bs);
			break;
		}
		case SLICE_PA:
		case SLICE_PB:
		case SLICE_PC:
#if _DEBUG_VERBOSE
			print_msg_queue("*** find coded slice partition, "
				"nal_unit_type %u ***\n", nal_unit_type);
#endif
			break;
		case SLICE_IDR_PIC:
		{
			bits_handler bs;
			unsigned first_mb_in_slice;
			unsigned pic_parameter_set_id;

			bits_handler_init(&bs, p, 32);
			first_mb_in_slice = ue_exp_golomb(&bs);
			slice_type = ue_exp_golomb(&bs);
			pic_parameter_set_id = ue_exp_golomb(&bs);
			break;
		}
		case SEI:
#if _DEBUG_VERBOSE
			print_msg_queue("*** find SEI ***\n");
#endif
			break;
		case SPS:
		case PPS:
			break;
		default:
#if _DEBUF_VREBOSE
			print_msg_queue("*** H.264 nal_unit_type %u ***\n",
					nal_unit_type);
#endif
			break;
	}

	return slice_type;
}

int snx_avc_get_slice_type(unsigned char *paddr, unsigned size)
{
	unsigned *ptr = (unsigned*)paddr;
	unsigned left_sz = size, value, status = 0;
	short last_status;
	int slice_type = ERROR;

	if ((unsigned) ptr & 0x03L) {
#if _DEBUG_VERBOSE
		print_msg_queue("%s: bit-stream buffer not aligned\n");
#endif
	}
get_next_four_bytes:
	if (left_sz > 3) {
		value = *(unsigned*) ptr++;
		left_sz -= 4;
	} else if (left_sz > 0) {
		char *ptr_noaligned = (char*) ptr;
		value = 0x0L;
		while (left_sz) {
			value = value << 8;
			value |= *ptr_noaligned;
			ptr_noaligned++;
			left_sz--;
		}
		ptr = (unsigned*) ptr_noaligned;
	} else {
		goto parse_end;
	}
parse_start:
	if (value == 0x01000000L) {
		last_status = 0;
		status = 5;
	}
state_start:
	switch (status) {
		case 0:
			if ((value & 0xFF000000L) == 0L) {
				status = 1;
			if ((value & 0xFFFF0000L) == 0L) {
				status = 2;
			if ((value & 0xFFFFFF00L) == 0L) {
				status = 3;
			if ((value & 0xFFFFFFFFL) == 0L)
				status = 4;
			}}}
			goto get_next_four_bytes;
		case 1:
			if ((value & 0x00FFFFFFL) == 0x00010000) {
				last_status = status;
				status = 5;
			} else // redo parsing with current value
				status = 0;
			if (last_status != 4)
			goto state_start;
		case 2:
			if ((value & 0x0000FFFFL) == 0x00000100) {
				last_status = status;
				status = 5;
			} else // redo parsing with current value
				status = 0;
			if (last_status != 4)
			goto state_start;
		case 3:
			if ((value & 0x000000FFL) == 0x00000001) {
				last_status = status;
				status = 5;
			} else // redo parsing with current value
				status = 0;
			if (last_status != 4)
			goto state_start;
		case 4:
			// get next four bytes,
			// if value is one of case 1, 2, 3 or equal to
			// 0x01000000, then founded
			last_status = status;
			goto get_next_four_bytes;
		case 5:
		{
			char *start_code = NULL;
			if (last_status == 0) {
				start_code = --ptr;
				left_sz += 4;
			} else if (last_status == 1) {
				start_code = ptr - 2;
				start_code += 3;
				left_sz += 5;
			} else if (last_status == 2) {
				start_code = ptr - 2;
				start_code += 2;
				left_sz += 6;
			} else if (last_status == 3) {
				start_code = ptr - 2;
				start_code += 1;
				left_sz += 7;
			} else {
#if _DEBUG_VERBOSE
				print_msg_queue("%s: start code parsing "
						"failed\n", __func__);
#endif
			}


			// TODO: need to handle error case
			if ((slice_type = get_slice_type(start_code, left_sz))
				       	< TYPE_P) {
				// integer pointer address must align 4 bytes
				status = 0;
				ptr = ((unsigned)start_code + 4) & 0xFFFFFFFCL;
				left_sz -= 4 + ((unsigned)start_code & 0x03L);
				goto get_next_four_bytes;
			}
			break;
		}
	}
parse_end:
	return ((slice_type > 4) ? (slice_type - 5) : slice_type);
}

