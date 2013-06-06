/**
 * @file framebuff.c
 * @brief 
 * @author Airead Fan <fgh1987168@gmail.com>
 * @date 2012/09/19 14:51:49
 */

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <string.h>
#include <errno.h>
#include <sys/mman.h>
#include <unistd.h>
#include <linux/fb.h>

int main(int argc, char *argv[])
{
    int fd;
    char *devfb = "/dev/fb0";
	struct fb_fix_screeninfo finfo;
	struct fb_var_screeninfo vinfo;
	unsigned long screensize; /* bytes */
    unsigned long pos;
	unsigned char *start;	/* pointer mmap framebuffer starting */
    unsigned long x, y;

    fd = open(devfb, O_RDWR);
    if (fd < 0) {
        fprintf(stderr, "open %s failed: %s\n", devfb, strerror(errno));
        exit(1);
    }

    /* get screen information */
	//int ioctl(int d, int request, ...);
	if(ioctl(fd, FBIOGET_FSCREENINFO, &finfo) < 0){
		fprintf(stderr, "get fixed screen information failed: %s\n",
			strerror(errno));
		return -1;			
	}
	if(ioctl(fd, FBIOGET_VSCREENINFO, &vinfo) < 0){
		fprintf(stderr, "get var screen information failed: %s\n",
			strerror(errno));
		return -1;			
	}
    fprintf(stdout, "vinfo: xres %u, yres %u\n", vinfo.xres, vinfo.yres);
    fprintf(stdout, "vinfo: xres_vir %u, yres_vir %u\n", vinfo.xres_virtual, vinfo.yres_virtual);
    fprintf(stdout, "vinfo: xoffset %u, yresoffset %u\n", vinfo.xoffset, vinfo.yoffset);
    fprintf(stdout, "vinfo: height %u, width %u\n", vinfo.height, vinfo.width);

	screensize = finfo.line_length * vinfo.yres;

	/* get frame buffer start pointer via mmap */
	//void *mmap(void *addr, size_t length, int prot, int flags,
        //          int fd, off_t offset);
	start = mmap(NULL, screensize, PROT_READ 
				| PROT_WRITE, MAP_SHARED, fd, 0);
	if((void *)start == MAP_FAILED){
		fprintf(stderr, "mmap failed: %s\n", strerror(errno));
		return -1;
	}

    for (x = 0; x < vinfo.xres; x++) {
//        for (y = 0; y < vinfo.yres; y++) {
//            usleep(1);
            pos = x * 4 + 10 * finfo.line_length;
            start[pos] = 0xff;    /* blue */
            start[pos + 1] = 0x00; /* green */
            start[pos + 2] = 0xff; /* red */
            start[pos + 3] = 0x00;
//            fprintf(stdout, "(x, y) = (%lu, %lu)\n", x, y);
//        }
    }

//    memset(start, 0xff, screensize);

	//int munmap(void *addr, size_t length);
	if(munmap(start, screensize) < 0){
		fprintf(stderr, "munmap failed: %s\n", strerror(errno));
		return -1;
	}

    close(fd);

    return 0;
}


