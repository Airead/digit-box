/********************************************************
 * @author  Airead Fan <fgh1987168@gmail.com>		*
 * @date    2011 9月 21 10:15:46 CST			*
 ********************************************************
 *		after studying C 65 days		*
 *		after studying APUE 30 days		*
 ********************************************************/

/*
 * This program demonstrates method
 * user key control
 *
 * note: run as root
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <linux/input.h>
#include <errno.h>
#include <fcntl.h>

#define KEY_RELEASE 0
#define KEY_PRESS 1
#define KEY_KEEPING_PRESSED 2

int main(int argc, char *argv[])
{
	//step 1. open input device
	int fd;
        char *input_dev;

	input_dev = "/dev/input/by-path/platform-i8042-serio-0-event-kbd";
	
	//int open(const char *pathname, int flags);
	if((fd = open(input_dev, O_RDONLY)) < 0){
		fprintf(stderr, "open %s failed: %s\n", input_dev, strerror(errno));
		exit(1);
	}else{
		fprintf(stdout, "open %s successful\n", input_dev);
	}

	//step 2. read/show input codes
#if 0
/*
 * The event structure itself
 */

	struct input_event {
		struct timeval time;
		__u16 type;
		__u16 code;
		__s32 value;
	};
#endif
	struct input_event ev[64];
	int running;
	ssize_t rb;
	int i;

	running = 1;
	while(running){
		//ssize_t read(int fd, void *buf, size_t count);
		rb = read(fd, ev, sizeof(ev));
		if(rb < (int)sizeof(struct input_event)){
			perror("read error");
			exit(1);
		}else{
			fprintf(stdout, "receive char\n");
		}
		
		for(i = 0; i < (int)(rb/sizeof(struct input_event)); i++){
			if(EV_KEY == ev[i].type){
				fprintf(stdout, "%d\n", ev[i].code);
				if(ev[i].value == 1 || ev[i].value == 2){
					if(ev[i].code == KEY_UP){
						fprintf(stdout, "UP\n");
					}else if(ev[i].code == KEY_DOWN){
						fprintf(stdout, "DOWN\n");
					}
				}
			}
		}

	}
	
	
	//step 3. close fd
	close(fd);
	
	return 0;
}
