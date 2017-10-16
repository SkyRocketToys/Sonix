/*
* function:    checksum   algorithm
*              this codes can generate checksum of file
* 
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include <fcntl.h>

#include <sys/types.h>
#include <sys/stat.h>
#include<ctype.h>

char *checksum_prefix = "Checksum(0x):\n";
#define BUFFER_SIZE 4096 

unsigned short checksum( unsigned char * bufp, unsigned long buf_len ) 
{ 
	unsigned short sum = 0; 

	while( buf_len -- ) 
	{ 
		sum += *(bufp++); 
	} 

	return sum; 

} 

int get_checksum(char *user_conf, char *filename, int fd) 
{ 
	unsigned short chksum; 
	unsigned char buf[BUFFER_SIZE];

	int n_chars; 
	FILE *fp;

	if( ( fp = fopen(filename, "rb " ) ) == NULL ) 
	{ 
		fprintf( stderr, "Open file fail! " ); 
		perror(filename); 
		exit(1); 
	} 

	chksum = 0; 

	while( (n_chars = fread( buf , 1, BUFFER_SIZE, fp) ) > 0 ) 
	{ 
		chksum += checksum( buf, n_chars ); 
	} 

	if( n_chars == -1 ) 
	{ 
		fprintf( stderr, "Read file fail! " ); 
		perror( filename ); 
		exit(1); 
	} 

	char checksum_value[256];
	sprintf(checksum_value,"%4x",chksum);

	write(fd, "\n", 1);
	write(fd, checksum_prefix, strlen(checksum_prefix));
	write(fd, checksum_value, 4);
	write(fd, "\n", 1);

	return 0; 

}
