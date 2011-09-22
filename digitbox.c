/********************************************************
 * @author  Airead Fan <fgh1987168@gmail.com>		*
 * @date    2011 9月 21 19:40:05 CST			*
 ********************************************************
 *		after studying C 65 days		*
 *		after studying APUE 30 days		*
 ********************************************************/

/*
 * Main function of digit-box
 */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>
#include <linux/input.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include "digitbox.h"
#include "framebuffer.h"
#include "config.h"
#include "udisk.h"
#include "resource.h"
#include "maindeal.h"

uint16_t global_key_code;	/* pressed key code */
pthread_mutex_t key_code_mutex = PTHREAD_MUTEX_INITIALIZER;

int main(int argc, char *argv[])
{	
#if _DEBUG_
	char *debug_p;
	int debug_i;
#endif 
	struct mainstatus status;
	FB fb;			/* framebuffer struct */
	FILE *config_fp;	/* pointer to config file */
	char *value;		/* value of config file */
	char mp3_list[DB_LIST_MAX][DB_NAME_MAX + 1]; /* store <usbpath>/mp3/ *.mp3 */
	int mp3_list_len;
	char img_list[DB_LIST_MAX][DB_NAME_MAX + 1]; /* store <usbpath>/img/ *.jpg */
	int img_list_len;
	pthread_t tid_key_ctrl;			     /* key control thread id */
	int errorcode;				     /* errorcode for thread */
	char udisk_path[DB_NAME_MAX + 1];
	int running;		/* 1 loop, 2 stop */
	uint16_t cur_key_code;

	/*
	 * Initialize everything
	 *   1. read digitbox.conf
	 *   2. initialize framebuffer device
	 *   3. initialize user key control
	 *   4. initialize weather data
	 *   5. turn off echo
	 *   6. init global_key_code_mutex
	 *   7. start key control thread
	 */

	//FILE *config_open(char *filename);
	if((config_fp = config_open("digitbox.conf")) == NULL){
		fprintf(stderr, "can't open digitbox.conf: %s\n", 
			strerror(errno));
		exit(1);
	}

#if _DEBUG_
	
	//char *config_getvalue_byname(FILE *fp, char *name);
	debug_p = config_getvalue_byname(config_fp, "fb_dev");
	fprintf(stdout, "fb_dev: %s\n", debug_p);

	debug_p = config_getvalue_byname(config_fp, "input_dev");
	fprintf(stdout, "input_dev: %s\n", debug_p);
#endif

	value = config_getvalue_byname(config_fp, "fb_dev");
	//int fb_open(char *fbname, FB *fbp);
	if(fb_open(value, &fb) < 0){
		fprintf(stderr, "init framebuffer failed\n");
		exit(1);
	}

	//int config_close(FILE *fp);
	config_close(config_fp);
	
	stty_echo_off();

	//int thread_key_control_start(FILE *fp);
	tid_key_ctrl = thread_key_control_start();
	

	/*
	 * Detect U disk and mount it
	 * Read mp3/image list
	 */

	/* Detect U disk anm mount it */
#if _DEBUG_VIR		    /* should place after actual udisk function */
	
	memset(udisk_path, 0, DB_NAME_MAX + 1);
	//char *strncpy(char *dest, const char *src, size_t n);
	strncpy(udisk_path, "/home/airead/study/virusb", DB_NAME_MAX);

	/* udisk_path first be used for device name */
	//int udisk_detect_vir(char *devname);
	udisk_detect_vir(udisk_path);

	//int udisk_mount_vir(char *devname);
	udisk_mount_vir(udisk_path);

#endif
	
	/* Read mp3/image list */
	//int resource_common_list(char *usbpath, char list[][DB_NAME_MAX + 1], 
	//                        char *type);
	if((img_list_len = resource_common_list(udisk_path, img_list, "jpg")) < 0){
		fprintf(stderr, "resource_common_list(jpg) failed\n");
	}

	if((mp3_list_len = resource_common_list(udisk_path, mp3_list, "mp3")) < 0){
		fprintf(stderr, "resource_common_list(jpg) failed\n");
	}

#if _DEBUG_
	fprintf(stdout, "--------------------img list--------------------\n");
	for(debug_i = 0; debug_i < img_list_len; debug_i++){
		fprintf(stdout, "%3d: %s\n", debug_i, img_list[debug_i]);
	}

	fprintf(stdout, "--------------------mp3 list--------------------\n");
	for(debug_i = 0; debug_i < mp3_list_len; debug_i++){
		fprintf(stdout, "%3d: %s\n", debug_i, mp3_list[debug_i]);
	}
#endif

	/*
	 * Main loop
	 *   1. show picture
	 *   2. play music
	 */
	running = 1;
	cur_key_code = 0;
	while(running){
		common_change_code(&cur_key_code, global_key_code);
		common_change_code(&global_key_code, 0);

		
		common_dealcode(&status, cur_key_code);
		switch(cur_key_code){
		case 0:
			break;
		case KEY_Q:
			running = 0;
			break;
		case KEY_UP:
			fprintf(stdout, "UP\n");
			break;
		default:
			break;
		}

		usleep(33);
	}

	/*
	 * wait key control thread exit
	 */

	//int pthread_join(pthread_t thread, void **value_ptr);
	pthread_join(tid_key_ctrl, NULL);

	/*
	 * Free all resource
	 *   1. close framebuffer
	 *   2. umount U disk
	 *   3. trun on echo
	 *   4. destory key_code_mutex
	 */
			   
	//int fb_close(FB *fbp);
	fb_close(&fb);

	//int umount(const char *target);
			   
	stty_echo_on();
			   
	//int pthread_mutex_destroy(pthread_mutex_t *mutex);
	if((errorcode = pthread_mutex_destroy(&key_code_mutex)) != 0){
		fprintf(stderr, "pthread_mutex_destroy() failed: %s\n",
			strerror(errno));
	}
	
	return 0;
}

/*
 * start key control thread
 */
int thread_key_control_start(void)
{
	pthread_t tid;
	int code;

	//int pthread_create(pthread_t *restrict thread,
        //      const pthread_attr_t *restrict attr,
        //      void *(*start_routine)(void*), void *restrict arg);
	code = pthread_create(&tid, NULL, thread_key_control, NULL);

	if(code != 0){
		fprintf(stderr, "%s:%d: pthread_create failed: %s\n", __FILE__,
			__LINE__, strerror(errno));

		return -1;
	}

	return tid;
}

/*
 * User key control thread
 */
void *thread_key_control(void *arg)
{
	int in_fd;
	FILE *fp;
	char *value;
	struct input_event ev[64];
	int running;
	ssize_t rb;
	int i;

	//FILE *fopen(const char *path, const char *mode);
	if((fp = fopen("digitbox.conf", "rw")) == NULL){
		fprintf(stderr, "fopen config file failed\n");
		pthread_exit((void *)-1);
	}

#if _DEBUG_
	fprintf(stdout, "key control pthread start...\n");
#endif

	value = config_getvalue_byname(fp, "input_dev");

#if _DEBUG_
//	fprintf(stdout, "value = %s\n", value);
#endif

	//int open(const char *pathname, int flags);
	if((in_fd = open(value, O_RDONLY)) < 0){
		fprintf(stderr, "open %s failed: %s\n", value, strerror(errno));
		exit(1);
	}else{
		fprintf(stdout, "open %s successful\n", value);
	}
	
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

	running = 1;
	while(running){
		//ssize_t read(int fd, void *buf, size_t count);
		rb = read(in_fd, ev, sizeof(ev));
		if(rb < (int)sizeof(struct input_event)){
			perror("read error");
			pthread_exit((void *)1);
		}

#if _DEBUG_
//			fprintf(stdout, "receive char\n");
#endif
	
		for(i = 0; i < (int)(rb/sizeof(struct input_event)); i++){
			if(EV_KEY == ev[i].type){
				if(ev[i].value == 1 || ev[i].value == 2){

					common_change_code(&global_key_code,
							   ev[i].code);
					if(ev[i].code == KEY_Q){
						pthread_exit(0);
					}
				}
			}
		}

	}

	//close file descriptor
	close(in_fd);

	//void pthread_exit(void *value_ptr);
	pthread_exit(NULL);
}

/*
 * Common change key code with key_code_mutex
 */
int common_change_code(uint16_t *dest, uint16_t scr)
{
	int errorcode;

	//int pthread_mutex_lock(pthread_mutex_t *mutex);
	if((errorcode = pthread_mutex_lock(&key_code_mutex)) > 0){
		fprintf(stderr, "%s: phtread_mutex_lock faild: %s\n",
			__func__, strerror(errorcode));
			
		return -1;
	}

	*dest = scr;

	//int pthread_mutex_unlock(pthread_mutex_t *mutex);
	if((errorcode = pthread_mutex_unlock(&key_code_mutex)) > 0){
		fprintf(stderr, "%s: phtread_mutex_unlock faild: %s\n",
			__func__, strerror(errorcode));
			
		return -1;
	}

	return 0;
}

int stty_echo_off(void)
{
#if 0
	The termios structure
		Many of the functions described here have a termios_p argument that is
		a pointer to a termios structure.  This structure  contains  at  least
		the following members:

	tcflag_t c_iflag;      /* input modes */
	tcflag_t c_oflag;      /* output modes */
	tcflag_t c_cflag;      /* control modes */
	tcflag_t c_lflag;      /* local modes */
	cc_t     c_cc[NCCS];   /* control chars */
#endif
	struct termios term;

	//int tcgetattr(int fd, struct termios *termios_p);
	if(tcgetattr(STDIN_FILENO, &term) < 0){
		perror("tcgetattr error");
	}

	term.c_lflag &= ~ECHO;	/* set 8 bits/byte */

	//int tcsetattr(int fd, int optional_actions,
        //             const struct termios *termios_p);
	if(tcsetattr(STDIN_FILENO, TCSANOW, &term) < 0){
		perror("tcsetattr error");
	}

	return 0;
}
	


int stty_echo_on(void)
{
	struct termios term;

	//int tcgetattr(int fd, struct termios *termios_p);
	if(tcgetattr(STDIN_FILENO, &term) < 0){
		perror("tcgetattr error");
	}

	term.c_lflag |= ECHO;	/* set 8 bits/byte */

	//int tcsetattr(int fd, int optional_actions,
        //             const struct termios *termios_p);
	if(tcsetattr(STDIN_FILENO, TCSANOW, &term) < 0){
		perror("tcsetattr error");
	}

	return 0;
}
