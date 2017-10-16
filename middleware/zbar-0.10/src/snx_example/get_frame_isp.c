#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <getopt.h>             /* getopt_long() */
#include <fcntl.h>              /* low-level i/o */
#include <unistd.h>
#include <errno.h>
#include <malloc.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <asm/types.h>          /* for videodev2.h */
#include <linux/videodev2.h>
#define printf(x,...)

#define CLEAR(x) memset (&(x), 0, sizeof (x))

typedef enum {
    IO_METHOD_READ, IO_METHOD_MMAP, IO_METHOD_USERPTR,
} io_method;

struct buffer {
    void * start;
    size_t length;//buffer's length is different from cap_image_size
};

char * dev_name = "/dev/video0";
io_method io = IO_METHOD_MMAP;//IO_METHOD_READ;//IO_METHOD_MMAP;


int fd = -1;
struct buffer * buffers = NULL;
unsigned int n_buffers = 0;
unsigned int cap_image_size = 0;//to keep the real image size!!

int rate = 30;
int width = 1920;
int height = 1080;

//////////////////////////////////////////
typedef unsigned char u8;

int static inline get_gray_pic8(u8 *p_in, u8 *p_out, 
			unsigned int width, unsigned int height)
{
	unsigned int w=0, h=0;
	for (h = 0; h < (height>>1) ; h++)
	{	
		for (w = 0; w < width * 3 ; w++) // two Line YYYYUVUV  YYYYYYYY
		{	
		
			if((!((w>>2)&1))&(w<(width * 2)) || (w>=(width * 2)))// Y plane Line 1
			{
				
				*(p_out + w + (width*2)*h	// one height shift two line
				- ((w>>3)*4)*(w<(width * 2)? 1:0) //Y line odd
				- (width)*(w<(width * 2)? 0:1) // Y line even
				) = *(p_in + w + (width*3)*h);
			}
			#if 0
			else 
			{
				if(!(w&1))  // U Plane Line 1
				*(p_out + (width*height) 
				+ ((width>>1)*h) + w 
				- ((w>>3)+1)*4
				- ((w- ((w>>3)+1)*4)>>1)) = *(p_in + w + (width*3)*h);
				else	 // V Plane Line 1
				*(p_out + (width*height) + ((width*height)>>2)
				+ ((width>>1)*h) + w -1
				- ((w>>3)+1)*4
				- ((w- ((w>>3)+1)*4)>>1)) = *(p_in + w + (width*3)*h);				
			}
			#endif

		}
	}
	return 0;
}


static void errno_exit(const char * s) {
    fprintf(stderr, "%s error %d, %s\n", s, errno, strerror(errno));
    exit(EXIT_FAILURE);

}

static int xioctl(int fd, int request, void * arg) {
    int r;
    do{
        r = ioctl(fd, request, arg);
    }while (-1 == r && EINTR == errno);

    return r;
}

static void process_image(void * p, void *data_out, int len) {
    //  static char[115200] Outbuff ;
	get_gray_pic8(p, data_out, width, height);
}

static int read_frame(char *data) {
	printf("%s %d data : 0x%X\n",__func__,__LINE__,data);
    struct v4l2_buffer buf;
    unsigned int i;
    switch (io) {
    case IO_METHOD_READ:
        if (-1 == read(fd, buffers[0].start, buffers[0].length)) {
            switch (errno) {
            case EAGAIN:
                return 0;
            case EIO:
                /* Could ignore EIO, see spec. */
                /* fall through */
            default:
                errno_exit("read");
            }
        }
        //      printf("length = %d/r", buffers[0].length);
        //      process_image(buffers[0].start, buffers[0].length);
        printf("image_size = %d,/t IO_METHOD_READ buffer.length=%d\n",
                cap_image_size, buffers[buf.index].length);
		printf("here ::\n");
		printf("%s %d data : 0x%X\n",__func__,__LINE__,data);
        process_image(buffers[buf.index].start,data, cap_image_size);
        break;
    case IO_METHOD_MMAP:
        CLEAR (buf);
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        if (-1 == xioctl(fd, VIDIOC_DQBUF, &buf)) {
            switch (errno) {
            case EAGAIN:
                return 0;
            case EIO:
                /* Could ignore EIO, see spec. */

                /* fall through */
            default:

                errno_exit("VIDIOC_DQBUF");

            }

        }
        assert(buf.index < n_buffers);
        //      printf("length = %d/r", buffers[buf.index].length);

        //      process_image(buffers[buf.index].start, buffers[buf.index].length);
        printf("image_size = %d,/t IO_METHOD_MMAP buffer.length=%d\n",
                cap_image_size, buffers[buf.index].length);
		printf("%s %d data : 0x%X\n",__func__,__LINE__,data);
	//memcpy(data,buffers[buf.index].start,cap_image_size);
        process_image(buffers[buf.index].start,data, cap_image_size);
        if (-1 == xioctl(fd, VIDIOC_QBUF, &buf))
            errno_exit("VIDIOC_QBUF");
        break;

    case IO_METHOD_USERPTR:
        CLEAR (buf);
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_USERPTR;
        if (-1 == xioctl(fd, VIDIOC_DQBUF, &buf)) {
            switch (errno) {
            case EAGAIN:
                return 0;
            case EIO:
                /* Could ignore EIO, see spec. */
                /* fall through */

            default:
                errno_exit("VIDIOC_DQBUF");
            }
        }

        for (i = 0; i < n_buffers; ++i)
            if (buf.m.userptr == (unsigned long) buffers[i].start && buf.length  == buffers[i].length)
                break;

        assert(i < n_buffers);
        //      printf("length = %d/r", buffers[i].length);
        //      process_image((void *) buf.m.userptr, buffers[i].length);

        printf("image_size = %d,/t IO_METHOD_USERPTR buffer.length=%d\n",
                cap_image_size, buffers[buf.index].length);
		printf("%s %d data : 0x%X\n",__func__,__LINE__,data);

        process_image(buffers[buf.index].start, data,cap_image_size);
        if (-1 == xioctl(fd, VIDIOC_QBUF, &buf))
            errno_exit("VIDIOC_QBUF");
        break;
    }
    return cap_image_size;
}

int get_frame(char *data) {

    unsigned int count;
	count = 1;
	int ret=0;
	printf("data : 0x%X\n",data);
//	while (count-- > 0)
		{
        for (;;) {
            fd_set fds;
            struct timeval tv;
            int r;
            FD_ZERO(&fds);
            FD_SET(fd, &fds);
            /* Timeout. */
            tv.tv_sec = 2;
            tv.tv_usec = 0;
            r = select(fd + 1, &fds, NULL, NULL, &tv);
            if (r < 0) {
                if (EINTR == errno)
                    continue;
                errno_exit("select");
            }
            if (0 == r) {
                fprintf(stderr, "select timeout\n");
                exit(EXIT_FAILURE);
            }
			printf("%d select ok\n", count);
			count++;
			printf("data : 0x%X\n",data);
		ret=read_frame(data);
		printf("read : %d\n",ret);
            if (ret)
               // continue;//
               break;
            /* EAGAIN - continue select loop. */
        }
    }
	return ret;
}

static void stop_capturing(void) {
    enum v4l2_buf_type type;
    switch (io) {
    case IO_METHOD_READ:
        /* Nothing to do. */
        break;
    case IO_METHOD_MMAP:
    case IO_METHOD_USERPTR:
        type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        if (-1 == xioctl(fd, VIDIOC_STREAMOFF, &type))
            errno_exit("VIDIOC_STREAMOFF");
        break;
    }
}

static void start_capturing(void) {
    unsigned int i;
    enum v4l2_buf_type type;
    switch (io) {
    case IO_METHOD_READ:
        /* Nothing to do. */
        break;
    case IO_METHOD_MMAP:
        for (i = 0; i < n_buffers; ++i) {
            struct v4l2_buffer buf;
            CLEAR (buf);
            buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            buf.memory = V4L2_MEMORY_MMAP;
            buf.index = i;
            if (-1 == xioctl(fd, VIDIOC_QBUF, &buf))
                errno_exit("VIDIOC_QBUF");
        }
        type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        if (-1 == xioctl(fd, VIDIOC_STREAMON, &type))
            errno_exit("VIDIOC_STREAMON");
        break;
    case IO_METHOD_USERPTR:
        for (i = 0; i < n_buffers; ++i) {
            struct v4l2_buffer buf;
            CLEAR (buf);
            buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            buf.memory = V4L2_MEMORY_USERPTR;
            buf.index = i;
            buf.m.userptr = (unsigned long) buffers[i].start;
            buf.length = buffers[i].length;
            if (-1 == xioctl(fd, VIDIOC_QBUF, &buf))
                errno_exit("VIDIOC_QBUF");
        }

        type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        if (-1 == xioctl(fd, VIDIOC_STREAMON, &type))
            errno_exit("VIDIOC_STREAMON");
        break;
    }
}

static void uninit_device(void) {
    unsigned int i;
    switch (io) {
    case IO_METHOD_READ:
        free(buffers[0].start);
        break;
    case IO_METHOD_MMAP:
        for (i = 0; i < n_buffers; ++i)
            if (-1 == munmap(buffers[i].start, buffers[i].length))
                errno_exit("munmap");
        break;
    case IO_METHOD_USERPTR:
        for (i = 0; i < n_buffers; ++i)
            free(buffers[i].start);
        break;
    }
    free(buffers);
}

static void init_read(unsigned int buffer_size) {
    buffers = calloc(1, sizeof(*buffers));
    if (!buffers) {
        fprintf(stderr, "Out of memory\n");
        exit(EXIT_FAILURE);
    }
    buffers[0].length = buffer_size;
    buffers[0].start = malloc(buffer_size);
    if (!buffers[0].start) {
        fprintf(stderr, "Out of memory\n");
        exit(EXIT_FAILURE);
    }
}

static void init_mmap(void) {
    struct v4l2_requestbuffers req;
    CLEAR (req);
    req.count = 4;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;
    if (-1 == xioctl(fd, VIDIOC_REQBUFS, &req)) {
        if (EINVAL == errno) {
            fprintf(stderr, "%s does not support "
                "memory mapping\n", dev_name);
            exit(EXIT_FAILURE);
        } else {
            errno_exit("VIDIOC_REQBUFS");
        }
    }
    if (req.count < 2) {
        fprintf(stderr, "Insufficient buffer memory on %s\n", dev_name);
        exit(EXIT_FAILURE);
    }
    buffers = calloc(req.count, sizeof(*buffers));
    if (!buffers) {
        fprintf(stderr, "Out of memory\n");
        exit(EXIT_FAILURE);
    }

    for (n_buffers = 0; n_buffers < req.count; ++n_buffers) {
        struct v4l2_buffer buf;
        CLEAR (buf);
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;
        buf.index = n_buffers;
        if (-1 == xioctl(fd, VIDIOC_QUERYBUF, &buf))
            errno_exit("VIDIOC_QUERYBUF");

	printf("querybuf:%08x\n", buf.length);
        buffers[n_buffers].length = buf.length;
        buffers[n_buffers].start = mmap(NULL /* start anywhere */, buf.length,
                PROT_READ | PROT_WRITE /* required */,
                MAP_SHARED /* recommended */, fd, buf.m.offset);
        if (MAP_FAILED == buffers[n_buffers].start)
            errno_exit("mmap");
    }
}

static void init_userp(unsigned int buffer_size) {
    struct v4l2_requestbuffers req;
    unsigned int page_size;
    page_size = getpagesize();
    buffer_size = (buffer_size + page_size - 1) & ~(page_size - 1);
    CLEAR (req);
    req.count = 4;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_USERPTR;
    if (-1 == xioctl(fd, VIDIOC_REQBUFS, &req)) {
        if (EINVAL == errno) {
            fprintf(stderr, "%s does not support "
                "user pointer i/o\n", dev_name);
            exit(EXIT_FAILURE);
        } else {
            errno_exit("VIDIOC_REQBUFS");
        }
    }
    buffers = calloc(4, sizeof(*buffers));
    if (!buffers) {
        fprintf(stderr, "Out of memory\n");
        exit(EXIT_FAILURE);
    }

    for (n_buffers = 0; n_buffers < 4; ++n_buffers) {
        buffers[n_buffers].length = buffer_size;
        buffers[n_buffers].start = memalign(/* boundary */page_size,
                buffer_size);
        if (!buffers[n_buffers].start) {
            fprintf(stderr, "Out of memory\n");
            exit(EXIT_FAILURE);
        }
    }
}

static void init_device(int rate, int width, int height) {
    struct v4l2_capability cap;
    struct v4l2_cropcap cropcap;
    struct v4l2_crop crop;
    struct v4l2_format fmt;
	struct v4l2_streamparm parm;

	printf("VIDIOC_QUERYCAP\n");
    if (-1 == xioctl(fd, VIDIOC_QUERYCAP, &cap)) {
        if (EINVAL == errno) {
            fprintf(stderr, "%s is no V4L2 device\n", dev_name);
            exit(EXIT_FAILURE);
        } else {
            errno_exit("VIDIOC_QUERYCAP");
        }
    }

	printf("V4L2_CAP_VIDEO_CAPTURE\n");
    if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
        fprintf(stderr, "%s is no video capture device\n", dev_name);
        exit(EXIT_FAILURE);
    }

    switch (io) {
    case IO_METHOD_READ:
        if (!(cap.capabilities & V4L2_CAP_READWRITE)) {
            fprintf(stderr, "%s does not support read i/o\n", dev_name);
            exit(EXIT_FAILURE);
        }
        break;
    case IO_METHOD_MMAP:
    case IO_METHOD_USERPTR:
        if (!(cap.capabilities & V4L2_CAP_STREAMING)) {
            fprintf(stderr, "%s does not support streaming i/o\n", dev_name);
            exit(EXIT_FAILURE);
        }
        break;
    }

//frame rate

	printf("VIDIOC_S_PARM\n");
	memset(&parm, 0, sizeof(parm));
	parm.type = V4L2_CAP_VIDEO_CAPTURE;
	parm.parm.capture.timeperframe.numerator = 1;
	parm.parm.capture.timeperframe.denominator = rate;
	if (xioctl(fd, VIDIOC_S_PARM, &parm) == -1)
		errno_exit("VIDIOC_S_PARM");

	printf("VIDIOC_G_PARM\n");
	memset(&parm, 0, sizeof(parm));
	parm.type = V4L2_CAP_VIDEO_CAPTURE;
	if (xioctl(fd, VIDIOC_G_PARM, &parm) == -1)
		errno_exit("VIDIOC_G_PARM");
	printf("set farme rate:%d\n", parm.parm.capture.timeperframe.denominator);

    //////not all capture support crop!!!!!!!

    /* Select video input, video standard and tune here. */

    printf("-#-#-#-#-#-#-#-#-#-#-#-#-#-\n");
    CLEAR (cropcap);
    cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (0 == xioctl(fd, VIDIOC_CROPCAP, &cropcap)) {
        crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

        crop.c.left = cropcap.defrect.left;
        crop.c.top = cropcap.defrect.top;
        crop.c.width = width;
        crop.c.height = height;

        printf("----->has ability to crop!!\n");
        printf("cropcap.defrect = (%d, %d, %d, %d)\n", cropcap.defrect.left,
                cropcap.defrect.top, cropcap.defrect.width,
                cropcap.defrect.height);
        if (-1 == xioctl(fd, VIDIOC_S_CROP, &crop)) {
            switch (errno) {
            case EINVAL:
                /* Cropping not supported. */
                break;
            default:
                /* Errors ignored. */
                break;
            }
            printf("-----!!but crop to (%d, %d, %d, %d) Failed!!\n",
                    crop.c.left, crop.c.top, crop.c.width, crop.c.height);
        } else {
            printf("----->sussess crop to (%d, %d, %d, %d)\n", crop.c.left,
                    crop.c.top, crop.c.width, crop.c.height);
        }
    } else {
        /* Errors ignored. */
        printf("!! has no ability to crop!!\n");
    }

    printf("-#-#-#-#-#-#-#-#-#-#-#-#-#-\n");
    printf("\n");
    ////////////crop finished!
    //////////set the format
    CLEAR (fmt);
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.width = width;
    fmt.fmt.pix.height = height;
    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_SNX420;
    fmt.fmt.pix.field = V4L2_FIELD_NONE;
    {
        printf("-#-#-#-#-#-#-#-#-#-#-#-#-#-\n");
        printf("=====will set fmt to (%d, %d)--", fmt.fmt.pix.width,
                fmt.fmt.pix.height);
        if (fmt.fmt.pix.pixelformat == V4L2_PIX_FMT_YUYV) {
            printf("V4L2_PIX_FMT_YUYV\n");
        } else if (fmt.fmt.pix.pixelformat == V4L2_PIX_FMT_SNX420) {
            printf("V4L2_PIX_FMT_SNX420\n");
        } else if (fmt.fmt.pix.pixelformat == V4L2_PIX_FMT_NV12) {
            printf("V4L2_PIX_FMT_NV12\n");
        }
    }

    if (-1 == xioctl(fd, VIDIOC_S_FMT, &fmt))
        errno_exit("VIDIOC_S_FMT");
	{
        printf("=====after set fmt\n");
        printf("    fmt.fmt.pix.width = %d\n", fmt.fmt.pix.width);
        printf("    fmt.fmt.pix.height = %d\n", fmt.fmt.pix.height);
        printf("    fmt.fmt.pix.sizeimage = %d\n", fmt.fmt.pix.sizeimage);
        cap_image_size = fmt.fmt.pix.sizeimage;
        printf("    fmt.fmt.pix.bytesperline = %d\n", fmt.fmt.pix.bytesperline);
        printf("-#-#-#-#-#-#-#-#-#-#-#-#-#-\n");
        printf("\n");
    }
    cap_image_size = fmt.fmt.pix.sizeimage;
    /* Note VIDIOC_S_FMT may change width and height. */
    printf("-#-#-#-#-#-#-#-#-#-#-#-#-#-\n");

    printf("After Buggy driver paranoia\n");
    printf("    >>fmt.fmt.pix.sizeimage = %d\n", fmt.fmt.pix.sizeimage);
    printf("    >>fmt.fmt.pix.bytesperline = %d\n", fmt.fmt.pix.bytesperline);
    printf("-#-#-#-#-#-#-#-#-#-#-#-#-#-\n");
    printf("\n");
    switch (io) {
    case IO_METHOD_READ:
        init_read(fmt.fmt.pix.sizeimage);
        break;
    case IO_METHOD_MMAP:
		printf("xxxxxxxxxxxxxxxxxxxxxxxxmapXXXXXXXXXXXXXXXXxxx\n");
        init_mmap();
        break;
    case IO_METHOD_USERPTR:
        init_userp(fmt.fmt.pix.sizeimage);
        break;
    }
}

static void close_device(void) {
    if (-1 == close(fd))
        errno_exit("close");
    fd = -1;
}

static void open_device(void) {
    struct stat st;
    if (-1 == stat(dev_name, &st)) {
        fprintf(stderr, "Cannot identify '%s': %d, %s\n", dev_name, errno,
                strerror(errno));
        exit(EXIT_FAILURE);
    }
    if (!S_ISCHR(st.st_mode)) {
        fprintf(stderr, "%s is no device\n", dev_name);
        exit(EXIT_FAILURE);
    }
    fd = open(dev_name, O_RDWR /* required */| O_NONBLOCK, 0);
    if (-1 == fd) {
        fprintf(stderr, "Cannot open '%s': %d, %s\n", dev_name, errno,
                strerror(errno));
        exit(EXIT_FAILURE);
    }

	printf("open_device Ok\n");
}


static const char short_options[] = "d:hmruR:W:H:";
static const struct option long_options[] = { { "device", required_argument,
        NULL, 'd' }, { "help", no_argument, NULL, 'h' }, { "mmap", no_argument,
        NULL, 'm' }, { "read", no_argument, NULL, 'r' }, { "userp",
        no_argument, NULL, 'u' }, { "frame", required_argument, NULL, 'R' },
        { "width", required_argument, NULL, 'W' },
        { "height", required_argument, NULL, 'H' },
        { 0, 0, 0, 0 } };
int init_isp(char *_dev,int _width,int _height,int _rate)
{
	dev_name=_dev;
	width=_width;
	height=_height;
	rate=_rate;
	open_device();
	init_device(rate, width, height);
	start_capturing();
	return 0;
}

int close_isp()
{
	stop_capturing();
	uninit_device();
	close_device();
	return 0;
}


