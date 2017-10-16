#ifndef _PACKAGE_IMAGE_H_
#define _PACKAGE_IMAGE_H_

#define LINE_MAX_LENGTH 500
#define READ_LENGTH 502

int package_sfimage(int in_fd, int out_fd, char *address_start,char *address_end, 
		char *pad_type1_value);

#endif /* _PACKAGE_IMAGE_H_ */
