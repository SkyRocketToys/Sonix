#ifndef _CHECK_H_
#define _CHECK_H_


#define LINE_MAX_LENGTH 500
#define READ_LENGTH 502

#define COMMON_MAX_LENGTH_CHECK 256
#define COMMON_SUM_LENGTH 6
#define CHECK_TYPE_MAX_LENGTH 256
#define CHECK_SECTION_MAX_LENGTH 2

int get_checksum(char *user_conf, char *filename, int write_fd) ;
int get_md5(char *user_conf, char *filename, int write_fd);
int check(char *user_conf,char *out_file, char *out_dir);
extern FILE * fopen_file(char *filename, char *mode);

#endif /* _CHECK_H_ */
