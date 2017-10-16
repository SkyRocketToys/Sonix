#ifndef __U_BOOT_H__
#define __U_BOOT_H__

int uboot_image(unsigned int in_fd,unsigned int out_fd, unsigned int virsion);
int rescue_image(int in_fd, int out_fd, unsigned int virsion); 
int ulogo_image(unsigned int in_fd,unsigned int out_fd, unsigned int virsion);
#endif /* __U_BOOT_H__ */
