/*
* function: read user configuration file and insert data to special address
*           
*
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include<ctype.h>

#include "insert.h"

//static struct insert_info insert_box;
#if 0
int deal_with_insertdata(char *user_conf,char *out_file, unsigned int rootfs_r_size, unsigned int jffs2_size)
{
	FILE *con_fp;
	char *t;
	char *commonflag = "[COMMON]";
	char *commonsum = "count=";
	char *inserttype = "INSERT_DATA";
	char *con_pointer;
	char *delim = " ";
	char section_num = 0;
	char find_insert_byte = 0;
	char line[READ_LENGTH];
	char common_flag[COMMON_MAX_LENGTH];
	char common_count[COMMON_MAX_LENGTH];
	char common_index[COMMON_MAX_LENGTH];
	char insert_type[INSERT_TYPE_MAX_LENGTH];
	char find_all = 0;
	char change_rootfs_r_value[INSERT_DATA_MAX_LENGTH];
	unsigned int rootfs_r_address_ui_1 = 0,rootfs_r_address_ui_2 = 0;

	//get rootfs-r size 
	printf("get the rootfs-r address and size:\n");
	if(rootfs_r_endaddress_1[strlen(rootfs_r_endaddress_1) - 1] == '\n') rootfs_r_endaddress_1[strlen(rootfs_r_endaddress_1) - 1] = '\0';
	if(rootfs_r_endaddress_2[strlen(rootfs_r_endaddress_1) - 2] == '\n') rootfs_r_endaddress_2[strlen(rootfs_r_endaddress_2) - 1] = '\0';
	if((strcmp(rootfs_r_endaddress_1,"empty")  != 0 )&&(strlen(rootfs_r_endaddress_1)  != 0)){
		rootfs_r_address_ui_1 = (unsigned int)strtoll(rootfs_r_endaddress_1, NULL, 0);
		rootfs_r_address_ui_1 -= 3;
		sprintf(rootfs_r_endaddress_1,"0x%08X",rootfs_r_address_ui_1);
		printf("address1:%s\n",rootfs_r_endaddress_1);
	}
	if((strcmp(rootfs_r_endaddress_2,"empty")  != 0 )&&(strlen(rootfs_r_endaddress_2)  != 0)){
		rootfs_r_address_ui_2 = (unsigned int)strtoll(rootfs_r_endaddress_2, NULL, 0);
		rootfs_r_address_ui_2 -= 3;
		sprintf(rootfs_r_endaddress_2,"0x%08X",rootfs_r_address_ui_2);
		printf("address2:%s\n",rootfs_r_endaddress_2);
	}
	sprintf(change_rootfs_r_value,"0x%08x",rootfs_r_len);
	printf("size:%s\n",change_rootfs_r_value);
	
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
			if((section_num == 0)&&(find_insert_byte == 0)){
				sprintf(common_flag,"%s",t);	
				if(common_flag[strlen(common_flag) - 1] == 0xd) common_flag[strlen(common_flag) - 1] = '\0';
				if(strcmp(common_flag,commonflag) == 0) {find_insert_byte = 1;break;}
			}
			if((section_num == 0)&&(find_insert_byte == 1)){
				sprintf(common_count,"%s",t);
				if(common_count[strlen(common_count) - 1] == 0xd) common_count[strlen(common_count) - 1] = '\0';
				common_count[COMMON_SUM_LENGTH] = '\0';
				if(strcmp(common_count,commonsum) == 0) {find_insert_byte = 2;break;}
				else {fprintf(stderr, "'%s' line format error\n", commonsum);return -1;};
			}
			if((section_num == 0)&&(find_insert_byte == 2)){
				sprintf(common_index,"%s",t);
			}
			if((section_num == 1)&&(find_insert_byte == 2)){
				sprintf(insert_type,"%s",t);
				if(strcmp(insert_type,inserttype) == 0) find_insert_byte = 3;
			}
			if((section_num == 2)&&(find_insert_byte == 3)){
				sprintf(insert_box.start,"%s",t);
			}
			if((section_num == 3)&&(find_insert_byte == 3)){
				sprintf(insert_box.length,"%s",t);
			}
			if((section_num == 4)&&(find_insert_byte == 3)){
				sprintf(insert_box.value,"%s",t);
				if(insert_box.value[strlen(insert_box.value) - 1] == 0xd) insert_box.value[strlen(insert_box.value) - 1] = '\0';
				find_insert_byte = 4;
			}
			/*find INSERT_DATA*/
			if (find_insert_byte == 4){
				find_insert_byte = 2;
				//checkout format
				find_all = 1;
				if((strlen(insert_box.start) > ACTUAL_INSERT_DATA_LENGTH)||(strlen(insert_box.length) > ACTUAL_INSERT_DATA_LENGTH)||(strlen(insert_box.value) > ACTUAL_INSERT_DATA_LENGTH)){
					fprintf(stderr, "elemnet length too long in '%s' line\n",common_index);
					return -1;
				}
				//find out the value need to be changed for rootfs-r size
				if(strcmp(rootfs_r_endaddress_1,"empty")  != 0 ){
					if(strcmp(insert_box.start, rootfs_r_endaddress_1) == 0){
						memcpy(insert_box.value,change_rootfs_r_value,strlen(change_rootfs_r_value));
					}
					
				}
				if(strcmp(rootfs_r_endaddress_2,"empty")  != 0 ){
					if(strcmp(insert_box.start, rootfs_r_endaddress_2) == 0){
						memcpy(insert_box.value,change_rootfs_r_value,strlen(change_rootfs_r_value));
					}
				}
				//printf("%s-%s-%s\n",insert_box.start,insert_box.length,insert_box.value);
				//insert data
				if(insert_data(out_file, insert_box.start, insert_box.length, insert_box.value)){
					fprintf(stderr, "insert data failed\n");
					return -1;
				}
			}
			t = strtok(NULL, delim);
			section_num ++;
		}
		
		//check format
		if((find_all == 1)&&(section_num > SECTION_MAX_LENGTH)){
			find_all = 0;
			fprintf(stderr, "too many elemnets in '%s' line\n",common_index);
		}
	}
	
	fclose (con_fp);
	
	return 0;
}
#endif
int insert_data(char *filename, unsigned int insert_start, unsigned int insert_length, unsigned int insert_value)
{
	/*unsigned int length = 0;
	unsigned int start = 
	
	start = strtoll(insert_start, NULL, 0);
	length = strtoll(insert_length, NULL, 0);*/
	unsigned char value_char = 0;//1
	unsigned short value_short = 0;//2
	unsigned int  value_int = 0;//4
	
	//rewrite value in address_start
	FILE *stream = fopen(filename,"rb+");
	if(!stream) {
		fprintf(stderr, "2 open file: %s failed\n", filename);
		return -1;
	}
	
	fseek(stream,insert_start,SEEK_SET);
	
	if(insert_length == 1){
		value_char = (unsigned char) insert_value;
		fwrite(&value_char,insert_length,1,stream);
	}
	else if(insert_length == 2){
		value_short = (unsigned short) insert_value;
		fwrite(&value_short,insert_length,1,stream);
	}
	else if(insert_length == 4){
		value_int = (unsigned int) insert_value;
		fwrite(&value_int,insert_length,1,stream);
	}
	else{
		fprintf(stderr, "write length is illegal\n");
		return -1;
	}
		

	fclose(stream);

	return 0;
}
