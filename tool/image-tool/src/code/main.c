#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include <fcntl.h>

#include <sys/types.h>
#include <sys/stat.h>

#include "flash.h"
#include "hw_setting.h"
#include "uboot.h"
#include "generated/snx_sdk_conf.h"

struct update_info {
	volatile unsigned int flash_info_start;
	volatile unsigned int flash_info_len;

	volatile unsigned int hw_setting_start;
	volatile unsigned int hw_setting_len;

	volatile unsigned int u_boot_start;
	volatile unsigned int u_boot_len;
	
	volatile unsigned int u_env_start;
	volatile unsigned int u_env_len;
	
	volatile unsigned int flash_layout_start;
	volatile unsigned int flash_layout_len;
	
	volatile unsigned int factory_start;
	volatile unsigned int factory_len;
	
	volatile unsigned int u_logo_start;
	volatile unsigned int u_logo_len;

	volatile unsigned int rescue_start;
	volatile unsigned int rescue_len;
}info;


struct bss {
	volatile unsigned int start;
	volatile unsigned int end;
} bss;

static char *file_names[] = {
	"flash_info.image.d",
	"hw_setting.image.d",
	"u_boot.image.d",
	"u_env.bin.d",
	"flash_layout.bin.d",
	"factory.bin.d",
	"u_logo.image.d",
	"rescue.image.d",
	"uexce.bin.d",
	NULL
};

static char file_dir[256];
static char buf[256];

struct option long_options[] = {
    { "exec",    1, NULL, 'e' },
	{ "flash",   1, NULL, 'f' },  
	{ "hwset",   1, NULL, 'h' },
	{ "uboot",   1, NULL, 'u' },
	{ "uenv",   1, NULL,  'n' },
	{ "flashlayout",   1, NULL,  'l' },
	{ "factory",   1, NULL,  't' },
	{ "ulogo",   1, NULL,  'g' },
	{ "rescue",   1, NULL, 's' }, 
	{ "virsion", 1, NULL, 'v' },
	{ "outdir",  1, NULL, 'o' },
	{ "mhwsets",  1, NULL, 'm' },
	{ NULL, 0, NULL, 0},  
};

static void usage(char *name)
{
	printf("\tUsage:\n");
	printf("\t %s \n", name);

	printf("\t-exec    -e : exec code file\n");
	printf("\t-flash   -f : flash info1 input file\n");
	printf("\t-hwset   -h : HW setting input file\n");
	printf("\t-uboot   -u : u-boot input file\n"); 
	printf("\t-uenv   -n : u-env input file\n");
	printf("\t-flashlayout   -l : flash-layout input file\n");
	printf("\t-factory   -t : factory input file\n");
	printf("\t-ulogo   -g : u-logo input file\n");
	printf("\t-rescue   -s : rescue input file\n");     
	printf("\t-virsion -v : u-boot virsion\n");
	printf("\t-outdir  -o : image output dir\n\n");
	printf("\t-mhwsets  -m : multi-hw-settings\n\n");
	exit(-1);
}

static FILE * fopen_file(char *filename, char *mode)
{
	FILE *fp;
	
	fp = fopen(filename, mode);

	if(!fp) {
		fprintf(stderr, "open file: %s failed\n", filename);
		exit(-1);
	}

	return fp;
}

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

int main(int argc, char **argv)
{
	int retval;
	unsigned int i;
	struct stat stats;


	char *exec	= NULL;
	char *flash	= NULL;
	char *hwset	= NULL;
	char *uboot	= NULL;
	char *uenv	= NULL;
	char *flashlayout	= NULL;
	char *factory	= NULL;
	char *ulogo	= NULL;
	char *rescue	= NULL;
	unsigned int virsion = 0;
	char *outdir	= NULL;
	char *mhwsets	= NULL;



	int in_fd, out_fd = -1, out_bin;
	FILE *in_fp, *out_fp;
	
	while((retval = getopt_long(argc, argv, "e:f:h:u:n:l:t:g:s:v:o:m:", 
		long_options, NULL)) != -1) {		
		
		switch(retval) {
		case 'e':
			exec = optarg;
			break;

		case 'f':
			flash = optarg;
			break;	
				
		case 'h':
			hwset = optarg;
			break;
		case 'u':	
			uboot = optarg;
			break;	
		
		case 'n':
			uenv = optarg;
			break;
			
		case 'l':
			flashlayout = optarg;
			break;
			
		case 't':
			factory = optarg;
			break;
			
		case 'g':
			ulogo = optarg;
			break;

		case 's':
			rescue = optarg;
			break;	
			
		case 'v':
			virsion = strtoll(optarg, NULL, 0);
			break;

		case 'o':
			outdir = optarg;
			break;

		case 'm':
			mhwsets = optarg;
			break;
			
		default:
			usage(argv[0]);		
		}	
	}

	if(NULL == outdir)
		usage(argv[0]);

	/*
	 *	Calculate start & len
	 */

	/**
	 * exec bin
	 */
	memset(&info, 0, sizeof(info));

	in_fd = open_file(exec, O_RDONLY, 0660);
	fstat(in_fd, &stats);

	read(in_fd, buf, 8);
	read(in_fd, &bss, 8);

	printf("bss_start = 0x%x\n", bss.start);
	printf("bss_end = 0x%x\n", bss.end);
		
	info.flash_info_start = stats.st_size + bss.end - bss.start;
	close(in_fd);
	/***********************************************/

	if(flash) {
		sprintf(file_dir, "%s/%s", outdir, file_names[0]);
		
		in_fp = fopen_file(flash, "r");
		out_fp = fopen_file(file_dir, "w");
		
		if(flash_info_image(in_fp, out_fp))
			fprintf(stderr, "flash_info_image failed\n");

		fflush(out_fp);
		fstat(fileno(out_fp), &stats);
		info.flash_info_len =  stats.st_size;

		fclose(in_fp);
		fclose(out_fp);
	}

	printf("flash : %08x - %08x\n", info.flash_info_start,info.flash_info_len);

	info.hw_setting_start = info.flash_info_start + info.flash_info_len;

	if(hwset) {
		sprintf(file_dir, "%s/%s", outdir, file_names[1]);
		
		in_fp = fopen_file(hwset, "r");
		out_fp = fopen_file(file_dir, "w");
		
		if(hw_setting_image(in_fp, out_fp))
			fprintf(stderr, "HW_setting_image failed\n");

		fflush(out_fp);
		fstat(fileno(out_fp), &stats);
		info.hw_setting_len =  stats.st_size;
		
		fclose(in_fp);
		fclose(out_fp);
	}
#if defined(CONFIG_SYSTEM_PLATFORM_ST58660FPGA) || defined(CONFIG_SYSTEM_PLATFORM_SN98660) || defined(CONFIG_SYSTEM_PLATFORM_SN98661) || defined (CONFIG_SYSTEM_PLATFORM_SN98670) || defined (CONFIG_SYSTEM_PLATFORM_SN98671) || defined (CONFIG_SYSTEM_PLATFORM_SN98672) || defined (CONFIG_SYSTEM_PLATFORM_SN98293)
	else {
		// sprintf(file_dir, "%s/%s", outdir, file_names[1]);
		// out_fp = fopen_file(file_dir, "r");
		// fstat(fileno(out_fp), &stats);
		// info.hw_setting_len =  stats.st_size;
		// fclose(out_fp);
		info.hw_setting_len = 0;
	}	
#endif

	if(mhwsets){
		sprintf(file_dir, "%s/%s", outdir, file_names[1]);
		out_fp = fopen_file(file_dir, "r");
		fstat(fileno(out_fp), &stats);
		info.hw_setting_len =  stats.st_size;
		fclose(out_fp);
	}

	printf("hwset : %08x - %08x\n", info.hw_setting_start,info.hw_setting_len);

	info.u_boot_start = info.hw_setting_start + info.hw_setting_len;
	
	if(uboot) {
		sprintf(file_dir, "%s/%s", outdir, file_names[2]);

		in_fd = open_file(uboot, O_RDONLY, 0660);
		out_fd = open_file(file_dir, O_WRONLY | O_CREAT | O_TRUNC, 0660);

		if(uboot_image(in_fd, out_fd, virsion))
			fprintf(stderr, "uboot_image failed\n");

		fstat(out_fd, &stats);
		info.u_boot_len =  stats.st_size;
			
		close(in_fd);
		close(out_fd);

	}

	printf("uboot : %08x - %08x\n", info.u_boot_start,info.u_boot_len);

	info.u_env_start = info.u_boot_start + info.u_boot_len;
	
	if(uenv) {
		in_fd = open_file(uenv, O_RDONLY, 0660);

		fstat(in_fd, &stats);
		info.u_env_len =  stats.st_size;
			
		close(in_fd);
	}

	printf("uenv : %08x - %08x\n", info.u_env_start,info.u_env_len);


	info.flash_layout_start = info.u_env_start + info.u_env_len;
	
	if(flashlayout) {
		in_fd = open_file(flashlayout, O_RDONLY, 0660);

		fstat(in_fd, &stats);
		info.flash_layout_len =  stats.st_size;
			
		close(in_fd);
	}

	printf("flashlayout : %08x - %08x\n", info.flash_layout_start,info.flash_layout_len);

	info.factory_start = info.flash_layout_start + info.flash_layout_len;
	
	if(factory) {
		in_fd = open_file(factory, O_RDONLY, 0660);

		fstat(in_fd, &stats);
		info.factory_len =  stats.st_size;
			
		close(in_fd);
	}

	printf("factory : %08x - %08x\n", info.factory_start,info.factory_len);

	info.u_logo_start = info.factory_start + info.factory_len;
	
	if(ulogo) {
		#if 0
		in_fd = open_file(ulogo, O_RDONLY, 0660);
		fstat(in_fd, &stats);
		info.u_logo_len =  stats.st_size;
		close(in_fd);
		#else
		sprintf(file_dir, "%s/%s", outdir, file_names[6]);

		in_fd = open_file(ulogo, O_RDONLY, 0660);
		out_fd = open_file(file_dir, O_WRONLY | O_CREAT | O_TRUNC, 0660);

		if(ulogo_image(in_fd, out_fd, virsion))
			fprintf(stderr, "ulogo_image failed\n");

		fstat(out_fd, &stats);
		info.u_logo_len =  stats.st_size;
			
		close(in_fd);
		close(out_fd);
		#endif
	}

	printf("ulogo : %08x - %08x\n", info.u_logo_start,info.u_logo_len);

	info.rescue_start = info.u_logo_start + info.u_logo_len;

	if(rescue) {
		#if 0
		in_fd = open_file(rescue, O_RDONLY, 0660);
		fstat(in_fd, &stats);
		info.rescue_len =  stats.st_size;
		close(in_fd);
		#else
		sprintf(file_dir, "%s/%s", outdir, file_names[7]);

		in_fd = open_file(rescue, O_RDONLY, 0660);
		out_fd = open_file(file_dir, O_WRONLY | O_CREAT | O_TRUNC, 0660);

		if(rescue_image(in_fd, out_fd, virsion))
			fprintf(stderr, "rescue_image failed\n");

		fstat(out_fd, &stats);
		info.rescue_len =  stats.st_size;
			
		close(in_fd);
		close(out_fd);
		#endif
	}

	printf("rescue : %08x - %08x\n", info.rescue_start,info.rescue_len);


	/**
   	 * storage to exec file 
   	 */
	sprintf(file_dir, "%s/%s", outdir, file_names[8]);
	out_bin = open_file(file_dir, O_WRONLY | O_CREAT | O_TRUNC, 0660);
	in_fd   = open_file(exec, O_RDONLY, 0660);

	read(in_fd, buf, 24);
	write(out_bin, buf, 24);

#if 1	
	read(in_fd, buf, sizeof(info));
	write(out_bin, &info, sizeof(info));

	while((retval = read(in_fd, buf, 136)) > 0){
			write(out_bin, buf, retval);
		}

	buf[0] = (char)0;
	
	for(i = (bss.start); i < (bss.end); i++)
		write(out_bin, buf, 1);
#endif
		
	close(in_fd);


	/**
	 * flash
	 */
	if(flash) {
		sprintf(file_dir, "%s/%s", outdir, file_names[0]);
		
		in_fd = open_file(file_dir, O_RDONLY, 0660);

		while((retval = read(in_fd, buf, 128)) > 0) {
			//write(out_fd, buf, retval);
			write(out_bin, buf, retval);
		}
		
		close(in_fd);
	}

#if 1

	if(hwset) {
		
		sprintf(file_dir, "%s/%s", outdir, file_names[1]);
		
		in_fd = open_file(file_dir, O_RDONLY, 0660);

		while((retval = read(in_fd, buf, 128)) > 0) {
			//write(out_fd, buf, retval);
			write(out_bin, buf, retval);
		}
		
		close(in_fd);

	}

	if(mhwsets) {
		sprintf(file_dir, "%s/%s", outdir, file_names[1]);
		
		in_fd = open_file(file_dir, O_RDONLY, 0660);

		while((retval = read(in_fd, buf, 128)) > 0) {
			//write(out_fd, buf, retval);
			write(out_bin, buf, retval);
		}
		
		close(in_fd);
	}
	


	if(uboot) {
		sprintf(file_dir, "%s/%s", outdir, file_names[2]);
		
		in_fd = open_file(file_dir, O_RDONLY, 0660);

		while((retval = read(in_fd, buf, 128)) > 0) {
			//write(out_fd, buf, retval);
			write(out_bin, buf, retval);
		}
		
		close(in_fd);

	}

	if(uenv) {
		in_fd = open_file(uenv, O_RDONLY, 0660);

		while((retval = read(in_fd, buf, 128)) > 0) {
			//write(out_fd, buf, retval);
			write(out_bin, buf, retval);
		}
		
		close(in_fd);
	}

	if(flashlayout) {
		in_fd = open_file(flashlayout, O_RDONLY, 0660);

		while((retval = read(in_fd, buf, 128)) > 0) {
			//write(out_fd, buf, retval);
			write(out_bin, buf, retval);
		}
		
		close(in_fd);
	}

	if(factory) {
		in_fd = open_file(factory, O_RDONLY, 0660);

		while((retval = read(in_fd, buf, 128)) > 0) {
			//write(out_fd, buf, retval);
			write(out_bin, buf, retval);
		}
		
		close(in_fd);
	}

	if(ulogo) {
		sprintf(file_dir, "%s/%s", outdir, file_names[6]);
		
		in_fd = open_file(file_dir, O_RDONLY, 0660);

		while((retval = read(in_fd, buf, 128)) > 0) {
			//write(out_fd, buf, retval);
			write(out_bin, buf, retval);
		}
		
		close(in_fd);
	}

	if(rescue) {
		sprintf(file_dir, "%s/%s", outdir, file_names[7]);

		in_fd = open_file(file_dir, O_RDONLY, 0660);

		while((retval = read(in_fd, buf, 128)) > 0) {
			//write(out_fd, buf, retval);
			write(out_bin, buf, retval);
		}
		
		close(in_fd);
	}
#endif

	//close(out_fd);
	close (out_bin);
	
	return 0;
}
