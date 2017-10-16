/*
* function:  read flash-layout  file and package the image 
*            together according to flash-layout,this type 
*            fill the empty with 0xFF
*
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include<ctype.h>

#include "package_image.h"

int package_sfimage(int in_fd, int out_fd, char *address_start, 
		char *address_end, char *pad_type1_value)
{
	unsigned int size_in, size_out;
	unsigned char *p, *q;
	unsigned int start = 0, end = 0;
	unsigned int pad = 0;
	unsigned char padvalue_type1 = 0;
	
	struct stat stats_in;
	struct stat stats_out;
	
	unsigned char *pp , *qq;
	int check = 0;
	
	fstat(in_fd, &stats_in);
	fstat(out_fd, &stats_out);
	
	size_in = stats_in.st_size;
	size_out = stats_out.st_size;
	
	start = strtoll(address_start, NULL, 0);
	end = strtoll(address_end, NULL, 0);
	padvalue_type1 = (unsigned char) strtoll(pad_type1_value, NULL, 0);
	
	/**
   	 * make serial image
   	 */
	/*pad in front*/
	check = start - size_out;
	if(check < 0){
		fprintf(stderr, "flash layout error\n");
		return -1;
	}
	pad = start - size_out;
	if(pad > 0){
		q = p = (unsigned char *)malloc(pad);
		if(!q) {
			fprintf(stderr, "malloc failed\n");
			return -1;
		}

		memset(q,padvalue_type1,pad);
		
		write(out_fd, p, pad);
	
		free(p);
	}

	/*abstract content of in_fd*/
	qq = pp = (unsigned char *)malloc(size_in);
	if(!qq) {
		fprintf(stderr, "malloc failed\n");
		return -1;
	}
	if(read(in_fd, qq, size_in) != size_in) {
		fprintf(stderr, "image read failed\n");
		return -1;
	}
	write(out_fd, pp, size_in);
	free(pp);
	
	/*pad in back*/
	check = end - start + 1 - size_in;
	if(check < 0){
		fprintf(stderr, "flash layout error\n");
		return -1;
	}
	pad = end - start + 1 - size_in;
	qq = pp = (unsigned char *)malloc(pad);
	if(!qq) {
		fprintf(stderr, "malloc failed\n");
		return -1;
	}
	
	memset(qq,padvalue_type1,pad);
	
	write(out_fd, pp, pad);
	free(pp);


	return 0;
}
