#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


#include <sys/types.h>
#include <sys/stat.h>

#include "crc.h"

int uboot_image(unsigned int in_fd,unsigned int out_fd, unsigned int virsion) 
{
	unsigned int size, pad;
	unsigned char *p, *q;
	unsigned int uboot_crc = 0;
	struct stat stats;
	unsigned int i;

	fstat(in_fd, &stats);
	
	
	#if 1
	pad = 0;
	size = stats.st_size + sizeof(virsion);
	pad  = ((size + 0x1F) & (~0x1F)) - size;
	size = sizeof(size) + size + pad + sizeof(uboot_crc);
	#else
	pad = 0;
	size = stats.st_size + sizeof(virsion);
	size = sizeof(size) + size + sizeof(uboot_crc);
	#endif

	q = p = (unsigned char *)malloc(size);

	if(!q) {
		fprintf(stderr, "malloc failed\n");
		return -1;
	}

	*((unsigned int *)q) = size - sizeof(size);
	q += sizeof(size);

	if(read(in_fd, q, stats.st_size) != stats.st_size) {
		fprintf(stderr, "u-boot read failed\n");
		return -1;
	}
	q += stats.st_size;
	
	#if 1
	for(i = 0; i < pad; i++) {
		*((unsigned char *)q) = 0xFF;
		q++;
	}
	#endif

	*((unsigned int *)q) = virsion;
	q += sizeof(virsion);

	uboot_crc = (unsigned int)crcSlow(p + sizeof(size), stats.st_size 
		+ pad + sizeof(virsion));

	printf ("uboot_crc=%x\n",uboot_crc);

	*((unsigned short *)q) = uboot_crc;
	
	write(out_fd, p, size);

	free(p);

	return 0;
}


int rescue_image(int in_fd, int out_fd, unsigned int virsion) 
{
	unsigned int i, size, pad;
	unsigned char *p, *q;
	unsigned int rescue_crc = 0;
	
	struct stat stats;

	fstat(in_fd, &stats);
	
	pad = 0;
	size = stats.st_size;
	pad  = ((size + 0x1F) & (~0x1F)) - size;
	size = sizeof(size) + size + pad + sizeof(rescue_crc);
		
	q = p = (unsigned char *)malloc(size);

	if(!q) {
		fprintf(stderr, "malloc failed\n");
		return -1;
	}

	*((unsigned int *)q) = size - sizeof(size);
	q += sizeof(size);

	if(read(in_fd, q, stats.st_size) != stats.st_size) {
		fprintf(stderr, "rescue read failed\n");
		return -1;
	}
	q += stats.st_size;
	
	for(i = 0; i < pad; i++) {
		*((unsigned char *)q) = 0xFF;
		q++;
	}
	
	//*((unsigned int *)q) = virsion;
	//q += sizeof(virsion);

	rescue_crc = (unsigned int)crcSlow(p + sizeof(size), stats.st_size + pad);

//	printf ("rescue_crc=%x,pad=%x,stats.st_size=%x\n",rescue_crc,pad,stats.st_size);

	//*((unsigned short *)q) = rescue_crc;
	*((unsigned int *)q) = rescue_crc;	


	write(out_fd, p, size);

	free(p);

	return 0;
}

int ulogo_image(unsigned int in_fd,unsigned int out_fd, unsigned int virsion) 
{
	unsigned int i, size, pad;
	unsigned char *p, *q;
	unsigned int ulogo_crc = 0;
	
	struct stat stats;

	fstat(in_fd, &stats);
	
	pad = 0;
	size = stats.st_size;
	pad  = ((size + 0x1F) & (~0x1F)) - size;
	size = sizeof(size) + size + pad + sizeof(ulogo_crc);
		
	q = p = (unsigned char *)malloc(size);

	if(!q) {
		fprintf(stderr, "malloc failed\n");
		return -1;
	}

	*((unsigned int *)q) = size - sizeof(size);
	q += sizeof(size);

	if(read(in_fd, q, stats.st_size) != stats.st_size) {
		fprintf(stderr, "ulogo read failed\n");
		return -1;
	}
	q += stats.st_size;
	
	for(i = 0; i < pad; i++) {
		*((unsigned char *)q) = 0xFF;
		q++;
	}
	
	//*((unsigned int *)q) = virsion;
	//q += sizeof(virsion);

	ulogo_crc = (unsigned int)crcSlow(p + sizeof(size), stats.st_size + pad);

	printf ("ulogo_crc=%x\n",ulogo_crc);

	//*((unsigned short *)q) = ulogo_crc;
	*((unsigned int *)q) = ulogo_crc;	


	write(out_fd, p, size);

	free(p);

	return 0;
}

