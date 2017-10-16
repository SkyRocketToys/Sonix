#ifndef _crc_h
#define _crc_h

typedef unsigned short  crc;

crc crcSlow(unsigned char const message[], unsigned int nBytes);

unsigned int  xorcrypt(unsigned char const message[], unsigned int nBytes);

#endif /* _crc_h */
