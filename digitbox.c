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
#include <sys/types.h>
#include <sys/wait.h>
#include "digitbox.h"
#include "config.h"
#include "resource.h"
#include "maindeal.h"
#include "screen.h"

uint16_t global_auto_flag;
uint16_t global_key_code;	/* pressed key code */
pthread_mutex_t key_code_mutex = PTHREAD_MUTEX_INITIALIZER;


int main(int argc, char *argv[])
{	
	struct mainstatus status;

	pthread_t tid_key_ctrl;			     /* key control thread id */
	int errorcode;				     /* errorcode for thread */

	int running;		/* 1 loop, 2 stop */
	uint16_t cur_key_code;

	/*
	 * Initialize everything
	 *   3. initialize user key control
	 *   5. turn off echo
	 *   6. init global_key_code_mutex
	 *   7. start key control thread
	 */
	
	if(maindeal_mainstatus_init(&status) < 0){
		fprintf(stderr, "maindeal_mainstatus_init() failed\n");
		exit(1);
	}

	/* close echo */
	stty_echo_off();

	//int thread_key_control_start(FILE *fp);
	tid_key_ctrl = thread_key_control_start();
	
	/*
	 * Main loop
	 *   1. show picture
	 *   2. play music
	 */
	running = 1;
	cur_key_code = 0;
	while(running){
		common_change_code(&cur_key_code, global_key_code);

		if(global_key_code != 0){
			/* global key code will be use in maindeal.c */
			maindeal_common_dealcode(&status, cur_key_code);

			common_change_code(&global_key_code, 0);
		}

		if(cur_key_code == KEY_Q){
			running = 0;
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
	 *   5. kill mp3 process
	 */
			   
	maindeal_mainstatus_destory(&status);

	//int pthread_mutex_destroy(pthread_mutex_t *mutex);
	if((errorcode = pthread_mutex_destroy(&key_code_mutex)) != 0){
		fprintf(stderr, "pthread_mutex_destroy() failed: %s\n",
			strerror(errno));
	}
	
	kill(status.mp3_pid, SIGINT);
	waitpid(status.mp3_pid, NULL, 0);

	stty_echo_on();

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
	FILE *config_fp;
	char *value;
	struct input_event ev[64];
	int running;
	ssize_t rb;
	int i;

	//FILE *config_open(char *filename);
	if((config_fp = config_open("digitbox.conf", "r")) == NULL){
		fprintf(stderr, "fopen config file failed\n");
		common_change_code(&global_key_code, KEY_Q);
		pthread_exit((void *)-1);
	}


#if _DEBUG_
	fprintf(stdout, "key control pthread start...\n");
#endif

	value = config_getvalue_byname(config_fp, "input_dev");

#if _DEBUG_
//	fprintf(stdout, "value = %s\n", value);
#endif

	//int open(const char *pathname, int flags);
	if((in_fd = open(value, O_RDONLY)) < 0){
		fprintf(stderr, "open %s failed: %s\n", value, strerror(errno));
		common_change_code(&global_key_code, KEY_Q);
		pthread_exit((void *)1);
	}else{
		fprintf(stdout, "open %s successful\n", value);
	}
	
#if 0
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
			common_change_code(&global_key_code, KEY_Q);
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
						running = 0;
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
