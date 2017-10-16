/*
* function:  generate serial flash iamge which will be burned into
*            serial flash
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

#include "package_image.h"
#include "main.h"
#include "insert.h"
#include "check.h"
#include "pad_to_tail.h"
#include "generated/snx_sdk_conf.h"


char pad_name[PADNAME_MAX_LENGTH];
char pad_type[PADTYPE_MAX_LENGTH];
char pad_value[PADVALUE_MAX_LENGTH];
char pad_type1_value[PADVALUE_MAX_LENGTH];
char pad_type2_value[PADVALUE_MAX_LENGTH];
char *rootfs_flag = "rootfs-r";
unsigned int rootfs_info_addr = 0x1000;
unsigned int rootfs_r_size = 0;
//#ifdef CONFIG_SYSTEM_PLATFORM_ST58660FPGA
#if defined(CONFIG_SYSTEM_PLATFORM_ST58660FPGA) || defined(CONFIG_SYSTEM_PLATFORM_SN98660) || defined (CONFIG_SYSTEM_PLATFORM_SN98661)|| defined (CONFIG_SYSTEM_PLATFORM_SN98670) || defined (CONFIG_SYSTEM_PLATFORM_SN98671) || defined (CONFIG_SYSTEM_PLATFORM_SN98672) || defined (CONFIG_SYSTEM_PLATFORM_SN98293)
char *jffs2_flag = "jffs2-image";
unsigned int jffs2_info_addr = 0x2000;
unsigned int jffs2_size = 0;
#endif
char *common_section = "[COMMON]";
char *pad_value_prefix = "0x";




struct option long_options[] = {
	{ "flash_layout",   1, NULL,  'l' },
	{ "outdir",  1, NULL, 'o' },
	{ NULL, 0, NULL, 0},  
};

static void usage(char *name)
{
	printf("\tUsage:\n");
	printf("\t %s \n", name);

	printf("\t-flashlayout   -l : flash-layout input file\n");
	printf("\t-outdir   -o : output dir\n");
	printf("\t-srcdir   -s : output srcdir\n");
	exit(-1);
}

extern FILE * fopen_file(char *filename, char *mode)
{
	FILE *fp;
	
	fp = fopen(filename, mode);

	if(!fp) {
		fprintf(stderr, "3 open file: %s failed\n", filename);
		exit(-1);
	}

	return fp;
}

extern int open_file(char *filename, int flags, mode_t mode)
{
	int fd;
	
	fd = open(filename, flags, mode);

	if(fd < 0) {
		fprintf(stderr, "1 open file: %s failed\n", filename);
		exit(-1);
	}

	return fd;
}

int get_pad_type(char *user_conf)
{
	FILE *con_fp;
	char *t;
	char *padname = "[PAD_BYTE]";
	char *con_pointer;
	char *delim = " ";
	char section_num = 0;
	char find_pad_byte = 0;
	char line[READ_LENGTH];
	char *type_1 = "1";
	char *type_2 = "2";
	
	con_fp = fopen_file(user_conf, "r");

	while(fgets(line,READ_LENGTH,con_fp)) {

		if(line[strlen(line) -1] == '\n'){
			line[strlen(line) -1] = '\0';
		}

		con_pointer = line;
		while((*con_pointer) && isspace(*con_pointer))
			con_pointer++;

		if((*con_pointer) == '#' || (*con_pointer) == '\0')
			continue;

		if(strlen(line) > LINE_MAX_LENGTH ) {
			fprintf(stderr, "info too long on the line\n");
			return -1;
		}
		
		section_num = 0;
		find_pad_byte = 1;
		t = strtok(line, delim);
		if(strncmp(t,common_section,COMMON_SECTION_LENGTH) == 0)
			break;
		while(t != NULL){
			if((section_num == 0)&&(find_pad_byte == 0)){
				sprintf(pad_name,"%s",t);	
				if(pad_name[strlen(pad_name) - 1] == 0xd) pad_name[strlen(pad_name) - 1] = '\0';
				if(strcmp(pad_name,padname) == 0) {find_pad_byte = 1;break;}
			}
			if((section_num == 0)&&(find_pad_byte == 1)){
				sprintf(pad_type,"%s",t);	
				if(strcmp(pad_name,padname) == 0) {find_pad_byte = 1;}
			}
			if((section_num == 1)&&(find_pad_byte == 1)){
				sprintf(pad_value,"%s",t);
				if(pad_value[strlen(pad_value) - 1] == 0xd) pad_value[strlen(pad_value) - 1] = '\0';
				if(strcmp(pad_type,type_1) == 0){
					memcpy(pad_type1_value,pad_value,strlen(pad_value));
					find_pad_byte = 2;
				}
				if(strcmp(pad_type,type_2) == 0){
					memcpy(pad_type2_value,pad_value,strlen(pad_value));
					find_pad_byte = 2;
				}
			}
			t = strtok(NULL, delim);
			section_num ++;
		}
		
		if(strlen(pad_value) > ACTUAL_PADVALUE_LENGTH){
			fprintf(stderr, "pad value '%s' length too long\n",pad_value);
			return -1;
		}
		
		if(find_pad_byte == 2){
			if(section_num > 2){
				fprintf(stderr, "'%s' related line length too long\n",padname);
				return -1;
			}
		}
	}
	
	fclose (con_fp);
	
	return 0;
}

int main(int argc, char **argv)
{
	int retval;
	char *flash_layout	= NULL;
	char *outdir	= NULL;
	char *srcdir = NULL;
	static char out_file[4096];
	static char user_conf[4096];
	
	while((retval = getopt_long(argc, argv, "l:o:s:", 
		long_options, NULL)) != -1) {		
		
		switch(retval) {
		case 'l':
			flash_layout = optarg;
			break;

		case 'o':
			outdir = optarg;
			break;	
			
		case 's':
			srcdir = optarg;
			break;
			
		default:
			usage(argv[0]);		
		}	
	}

	if(NULL == outdir)
		usage(argv[0]);
		
	char *user_file = "phy_user.conf";
	char *target_file = "PHY-SF.bin";
	
	
	sprintf(user_conf, "%s/%s", srcdir, user_file);
	sprintf(out_file, "%s/%s", outdir, target_file);

	int in_fd = -1, out_bin = -1;
	FILE *con_fp=NULL, *find_size=NULL;
	char *t;
	char *delim = "\t";
	char *p;
		
	char line[READ_LENGTH];
	char type[TYPE_MAX_LENGTH];
	char address_start[ADDRESS_MAX_LENGTH];
	char address_end[ADDRESS_MAX_LENGTH];
	char file[FILE_MAX_LENGTH];
	char section_num = 0;

  char *first_image = "yes";
  
	/**
   	 * read user conf file to get pad type
   	 */
  //read user conf file
	if(get_pad_type(user_conf)){
		fprintf(stderr, "analyse user conf file failed\n");
		return -1;
	}
	
	/**
   	 * read flash layout file and make sf_image
   	 */
  //read flash layout file
	con_fp = fopen_file(flash_layout, "r");
	
	while(fgets(line,READ_LENGTH,con_fp)) {
		
		/*checkout format and abstract section*/
		if(line[strlen(line) -1] == '\n'){
			line[strlen(line) -1] = '\0';
		}

		p = line;
		while((*p) && isspace(*p))
			p++;

		if((*p) == '#' || (*p) == '\0')
			continue;

		if(strlen(line) > LINE_MAX_LENGTH ) {
			fprintf(stderr, "info too long on the line\n");
			return -1;
		}
		
		section_num = 0;
		t = strtok(line, delim);  
		while(t != NULL){
			if(section_num == 0)
				sprintf(type,"%s",t);
			if(section_num == 1)
				sprintf(address_start,"%s",t);
			if(section_num == 2){
				sprintf(address_end,"%s",t);
			}
			if(section_num == 3){
				sprintf(file,"%s",t);
				if(file[strlen(file) - 1] == 0xd)
					file[strlen(file) - 1] = '\0';
			}
			t = strtok(NULL, delim);
			section_num ++;
		}  
		if ((strcmp (file, "null") == 0) || (strcmp (address_start, address_end) == 0))
			continue;
		if(section_num > SECTION_COUNT){
			fprintf(stderr, "section too much on the line\n");
			return -1;
		}

		if((strlen(address_start) > ACTUAL_ADDRESS_LENGTH)||(strlen(address_end) > ACTUAL_ADDRESS_LENGTH)){
			fprintf(stderr, "type '%s' address length too long\n",type);
			return -1;
		}

		/*make serial image*/
		//open file
		in_fd = open_file(file, O_RDONLY, 0660);
		//open SERIAL-IMAGE.bin
		//first open or non-first open
		if(strcmp(first_image,"yes") == 0){
			out_bin = open_file(out_file, O_WRONLY | O_CREAT | O_TRUNC, 0664);
		}else if(strcmp(first_image,"no") == 0){
			out_bin = open_file(out_file, O_WRONLY | O_APPEND, 0664);
		}
		//package image according to the type
		if(package_sfimage(in_fd,out_bin, address_start, address_end,pad_type1_value)){
			fprintf(stderr, "sf_image failed\n");
			return -1;
		}
		//close  SERIAL-IMAGE.bin
		close (out_bin);
		//close input_file
		close(in_fd);
		first_image = "no";
	}
	//if pad_type2_value exists,then make the file up to 16M
	if(strncmp(pad_value_prefix,pad_type2_value,2) == 0){
		out_bin = open_file(out_file, O_WRONLY | O_APPEND, 0664);
		if(pad_to_tail(out_bin, pad_type2_value)){
			fprintf(stderr, "sf_image failed\n");
			return -1;			
		}
		close (out_bin);
	}
	//close flash layout file
	fclose (con_fp);

	if(check(user_conf,out_file,outdir)){
		fprintf(stderr, "process MD5 or CHECKSUM error\n");
		return -1;
	}

	printf("sucessful to generate serial image!\n");
	
	return 0; 
	
}
