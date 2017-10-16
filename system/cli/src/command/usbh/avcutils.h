/*
 * avcutils.h
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
#ifndef __AVCUTILS_H__
#define __AVCUTILS_H__

enum {
	ERROR = -2,
	NOT_SLICE,
	TYPE_P,
	TYPE_B,
	TYPE_I,
	TYPE_SP,
	TYPE_SI
};

/* Function: snx_avc_get_slice_type
 *
 * param:
 *     addr, input buffer address
 *     size, input buffer length
 * return:
 *     ERROR, something wrong, most case is end of buffer.
 *     slice type, is equal to TYPE_P or TYPE_I
 * NOTICE: passing buffer should at least one complete frame
 */
int snx_avc_get_slice_type(unsigned char *addr, unsigned size);

#endif // __AVCUTILS_H__
