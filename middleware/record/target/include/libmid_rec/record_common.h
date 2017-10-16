#ifndef __RECORD_COMMON_H__
#define __RECORD_COMMON_H__

#define AUDIO_CAP			(0x1<<0)
#define GPS_CAP				(0x1<<1)
#define GSENSOR_CAP		(0x1<<2)


#define	RD_WORD(ptr)		(WORD)(((WORD)*((BYTE*)(ptr)+1)<<8)|(WORD)*(BYTE*)(ptr))
#define	RD_DWORD(ptr)		(DWORD)(((DWORD)*((BYTE*)(ptr)+3)<<24)|((DWORD)*((BYTE*)(ptr)+2)<<16)|((WORD)*((BYTE*)(ptr)+1)<<8)|*(BYTE*)(ptr))

#define WR_WORD(ptr, val) 	{*(BYTE*)(ptr)=(BYTE)(val); *((BYTE*)(ptr)+1)=(BYTE)((WORD)(val)>>8);}
#define	WR_DWORD(ptr,val)	{*(BYTE*)(ptr)=(BYTE)(val); *((BYTE*)(ptr)+1)=(BYTE)((WORD)(val)>>8); *((BYTE*)(ptr)+2)=(BYTE)((DWORD)(val)>>16); *((BYTE*)(ptr)+3)=(BYTE)((DWORD)(val)>>24);}
#define WR_FOURCC(ptr_d,ptr_s)	{*(char*)(ptr_d)=*(char*)(ptr_s); *((char*)(ptr_d)+1)=*((char*)(ptr_s)+1); *((char*)(ptr_d)+2)=*((char*)(ptr_s)+2); *((char*)(ptr_d)+3)=*((char*)(ptr_s)+3);}
#define	WR_BDWORD(ptr,val)	{*((BYTE*)(ptr))=(BYTE)((DWORD)(val)>>24);*((BYTE*)(ptr)+1)=(BYTE)((DWORD)(val)>>16);*((BYTE*)(ptr)+2)=(BYTE)((WORD)(val)>>8);  *((BYTE*)(ptr) + 3)=(BYTE)(val); }
#define	WR_LDWORD(ptr,val)  	WR_DWORD(ptr,val)

#define RD_BWORD(ptr)		(WORD)(((WORD)*((BYTE*)(ptr))<<8)|((WORD)*((BYTE*)(ptr)+1)))
#define RD_BDWORD(ptr)		(DWORD)(((DWORD)*((BYTE*)(ptr))<<24)|((DWORD)*((BYTE*)(ptr)+1)<<16)|((WORD)*((BYTE*)(ptr)+2)<<8)|((DWORD)*((BYTE*)(ptr)+3)))


#define HD_SKIP_FRAME_SIZE 0x24

#define FHD_SKIP_FRAME_SIZE 0x45

#define HD_DEMUX_FRAME_SIZE 400*1024

#define FHD_DEMUX_FRAME_SIZE 600*1024

#endif

