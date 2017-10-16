#ifndef _PAD_H
#define _PAD_H


#define LINE_MAX_LENGTH 500
#define READ_LENGTH 502

#define COMMON_MAX_LENGTH 20
#define COMMON_SUM_LENGTH 6
#define INSERT_TYPE_MAX_LENGTH 20
#define INSERT_DATA_MAX_LENGTH 15
#define ACTUAL_INSERT_DATA_LENGTH 10
#define SECTION_MAX_LENGTH 5

struct insert_info {
	char start[INSERT_DATA_MAX_LENGTH];
	char length[INSERT_DATA_MAX_LENGTH];
	char value[INSERT_DATA_MAX_LENGTH];
};
#if 0
int deal_with_insertdata(char *user_conf,char *out_file, unsigned int rootfs_r_size, unsigned int jffs2_size);
#endif
int insert_data(char *filename, unsigned int insert_start, unsigned int insert_length, unsigned int insert_value);
extern FILE * fopen_file(char *filename, char *mode);
#endif /* _PAD_H */
