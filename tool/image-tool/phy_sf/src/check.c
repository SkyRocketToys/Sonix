/*
* function:  read user configuration file and get md5 and checksum,
*            then write the value you get into the special file
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

#include "check.h"

static int open_file(char *filename, int flags, mode_t mode)
{
	int fd;
	
	fd = open(filename, flags, mode);

	if(fd < 0) {
		fprintf(stderr, "open file: %s failed\n", filename);
		exit(-1);
	}

	return fd;
}

int check(char *user_conf,char *out_file, char *out_dir)
{
	FILE *con_fp;
	char *t;
	char *commonflag = "[COMMON]";
	char *commonsum = "count=";
	char *md5type = "MD5";
	char *checksumtype = "CHECKSUM";
	char *con_pointer;
	char *delim = " ";
	char section_num = 0;
	char find_check_byte = 0;
	char line[READ_LENGTH];
	char common_flag[COMMON_MAX_LENGTH_CHECK];
	char common_count[COMMON_MAX_LENGTH_CHECK];
	char common_index[COMMON_MAX_LENGTH_CHECK];
	char check_type[CHECK_TYPE_MAX_LENGTH];
	char find_all = 0;
	char *report_file = "phy_sf_report.txt";
	char get_write = 0;
	int write_fd = -1;
	char write_file[4096];
	
	snprintf(write_file,strlen(out_dir)+strlen(report_file) + 2, "%s/%s", out_dir, report_file);
	
	con_fp = fopen_file(user_conf, "r");

	while(fgets(line,READ_LENGTH,con_fp)) {
		
		find_all = 0;

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
		t = strtok(line, delim);  
		while(t != NULL){
			if((section_num == 0)&&(find_check_byte == 0)){
				if(t[strlen(t) - 1] == 0xd) t[strlen(t) - 1] = '\0';
				snprintf(common_flag,strlen(t)+1,"%s",t);	
				if(strcmp(common_flag,commonflag) == 0) {find_check_byte = 1;break;}
			}
			if((section_num == 0)&&(find_check_byte == 1)){
				if(t[strlen(t) - 1] == 0xd) t[strlen(t) - 1] = '\0';
				snprintf(common_count,strlen(t)+1,"%s",t);
				common_count[COMMON_SUM_LENGTH] = '\0';
				if(strcmp(common_count,commonsum) == 0) {find_check_byte = 2;break;}
				else {fprintf(stderr, "'%s' line format error\n", commonsum);return -1;};
			}
			if((section_num == 0)&&(find_check_byte == 2)){
				if(t[strlen(t) - 1] == 0xd) t[strlen(t) - 1] = '\0';
				snprintf(common_index,strlen(t)+1,"%s",t);
			}
			if((section_num == 1)&&(find_check_byte == 2)){
				if(t[strlen(t) - 1] == 0xd) t[strlen(t) - 1] = '\0';
				snprintf(check_type,strlen(t)+1,"%s",t);
				if(strcmp(check_type,md5type) == 0) find_check_byte = 3;
				else if (strcmp(check_type,checksumtype) == 0) find_check_byte = 4;
			}
			/*find check type*/
			if ((find_check_byte == 3)||(find_check_byte == 4)){
				find_all = 1;
				
				//open write file
				if(get_write == 0)
					write_fd = open_file(write_file, O_WRONLY | O_CREAT | O_TRUNC, 0664);
				else
					write_fd = open_file(write_file, O_WRONLY | O_APPEND, 0664);
				//check calculation and write it to write file
				if(find_check_byte == 3){
					if(get_md5(user_conf, out_file, write_fd)){
						fprintf(stderr, "mad5 calculating failed\n");
						return -1;
					}
				}
				if(find_check_byte == 4){
					if(get_checksum(user_conf, out_file, write_fd)){
						fprintf(stderr, "checksum calculating failed\n");
						return -1;
					}
				}
				//close write file
				close (write_fd);
				
				find_check_byte = 2;
				get_write = 1;
			}
			t = strtok(NULL, delim);
			section_num ++;
		}
		
		//check format
		if((find_all == 1)&&(section_num > CHECK_SECTION_MAX_LENGTH)){
			find_all = 0;
			fprintf(stderr, "too many elemnets in '%s' line\n",common_index);
			return -1;
		}
	}
	
	fclose (con_fp);
	
	return 0;
}
