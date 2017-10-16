#include <stdio.h>
#include <stdlib.h>
#include <zbar.h>
#include <sys/stat.h>
#define video_dev "/dev/video0"
typedef unsigned char u8;
int init_isp(const char *dev,int width,int height,int rate);
int get_frame(char *data);
int close_isp();

zbar_image_scanner_t *scanner = NULL;

int main (int argc, char **argv)
{
	int size;
    /* create a reader */
    scanner = zbar_image_scanner_create();
    /* configure the reader */
    zbar_image_scanner_set_config(scanner, 0, ZBAR_CFG_ENABLE, 1);
    /* obtain image data */
    int width = 640, height = 480,rate=30;
    if(argc>=3)
    {
		width=atoi(argv[1]);
		height=atoi(argv[2]);
		if(argc>3)
			rate=atoi(argv[3]);
	}
	printf("width : %d height : %d rate : %d \n",width,height,rate);
    size= width*height;
	char buf[640*480];
	//buf=malloc(width*height);
	if(!buf)
	{
		perror("malloc");
		exit(-1);
	}
	init_isp(video_dev,width,height,rate);
	

    /* wrap image data */
    zbar_image_t *image = zbar_image_create();
    zbar_image_set_format(image, *(int*)"Y800");
    zbar_image_set_size(image, width, height);
	while(1)
	{	
	get_frame(buf);
	get_frame(buf);
	get_frame(buf);
    zbar_image_set_data(image, buf,size, zbar_image_free_data);

    /* scan the image for barcodes */
    int n = zbar_scan_image(scanner, image);//cost mush time if the picture's size is very big
    /* extract results */
    const zbar_symbol_t *symbol = zbar_image_first_symbol(image);
    for(; symbol; symbol = zbar_symbol_next(symbol)) {
        /* do something useful with results */
        zbar_symbol_type_t typ = zbar_symbol_get_type(symbol);
        const char *data = zbar_symbol_get_data(symbol);
        printf("decoded %s symbol \"%s\"\n",
               zbar_get_symbol_name(typ), data);
    	}
	}
    /* clean up */
    zbar_image_destroy(image);
	
    zbar_image_scanner_destroy(scanner);
	close_isp();

    return(0);
}
