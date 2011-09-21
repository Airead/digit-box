/********************************************************
 * @author  Airead Fan <fgh1987168@gmail.com>		*
 * @date    2011 9月 21 19:26:48 CST			*
 ********************************************************
 *		after studying C 65 days		*
 *		after studying APUE 30 days		*
 ********************************************************/

/*
 * This program demonstrates method
 * play background music
 *
 * note: ues `madplay'
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>

int main(int argc, char *argv[])
{
	if(argc < 2){
		fprintf(stderr, "usage: %s <mp3name>\n", argv[0]);
		exit(1);
	}

	pid_t pid;

	//pid_t fork(void);
	if((pid = fork()) < 0){
		perror("fork() failed");
		exit(1);
	}else if(pid == 0){
		/* children process */
		//int execlp(const char *file, const char *arg, ...);
		if(execlp("madplay", "madplay", argv[1], NULL) < 0){
			perror("execlp failed");
			exit(1);
		}

		exit(0);
	}
	
	/* parent process */
	sleep(10);
	//int kill(pid_t pid, int sig);
	kill(pid, SIGTERM);
	

	return 0;
}

