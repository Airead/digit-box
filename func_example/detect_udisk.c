/********************************************************
 * @author  Airead Fan <fgh1987168@gmail.com>		*
 * @date    2011 9月 21 15:49:50 CST			*
 ********************************************************
 *		after studying C 65 days		*
 *		after studying APUE 30 days		*
 ********************************************************/

/*
 * This program demonstrates method
 * detect U disk and mount it
 * if main exit umount it
 *
 * note: run as root
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>

int main(int argc, char *argv[])
{
	char *dirpath;
	char *filename;
	
	dirpath = "/dev/disk/by-uuid";
	filename = "/etc/mtab";

	//step 1. open dir and file
	DIR *dirp;
	FILE *mtab;

	//FILE *fopen(const char *path, const char *mode);
	if((mtab = fopen(filename, "r")) == NULL){
		fprintf(stderr, "open %s failed\n", filename);
		exit(1);
	}
	

	//DIR *opendir(const char *name);
	if((dirp = opendir(dirpath)) == NULL){
		fprintf(stdout, "opendir %s failed\n", dirpath);
		exit(1);
	}else{
		fprintf(stdout, "opendir %s successfully\n", dirpath);
	}

	//step 1.5 show content of dirpath
#if 0
	struct dirent {
		ino_t          d_ino;       /* inode number */
		off_t          d_off;       /* offset to the next dirent */
		unsigned short d_reclen;    /* length of this record */
		unsigned char  d_type;      /* type of file; not supported
					       by all file system types */
		char           d_name[256]; /* filename */
	};
#endif
       
	struct dirent *direntp;

	errno = 0;
	//struct dirent *readdir(DIR *dirp);
	while((direntp = readdir(dirp)) != NULL){
		fprintf(stdout, "%s\n", direntp->d_name);

	}
	if(errno != 0){
		perror("readdir failed");
		exit(1);
	}

	
	//void rewinddir(DIR *dirp);
	rewinddir(dirp);

	//step 1.6 get mounted device name
	char mdev[64][255];
	int i;
	int devnum;

	i = 0;
	//int fscanf(FILE *stream, const char *format, ...);
	while(fscanf(mtab, "%s", mdev[i]) != EOF){
		//int getc(FILE *stream);
		while(getc(mtab) != '\n')
			;
	      
		//int strncmp(const char *s1, const char *s2, size_t n);
		if(strncmp(mdev[i], "/dev/sd", 7) == 0 &&
		   strncmp(mdev[i], "/dev/sda", 8) != 0){
			fprintf(stdout, "mdev: %s\n", mdev[i]);
			i++;
		}
	}

	strncpy(mdev[i], "", 1);

	//step 2. check content
	char buf[255];
	char path[255];
	char cmd[255];
	char *p;
	int flag;		/* if 0 not mount, if 1 mount */

	devnum = i;
	
	errno = 0; 
	//struct dirent *readdir(DIR *dirp);
	while((direntp = readdir(dirp)) != NULL){
		flag = 1;
		
		if(direntp->d_name[0] == '.'){ /* remove . and .. */
			continue;
		}
		//int snprintf(char *str, size_t size, const char *format, ...);
		snprintf(path, sizeof(path) - 1, "%s/%s",dirpath, direntp->d_name);
		
		//ssize_t readlink(const char *path, char *buf, size_t bufsiz);
		if(readlink(path, buf, sizeof(buf) - 1) < 0){
			perror("readlink failed");
			exit(1);
		}
	
		p = strrchr(buf, '/');
		if(p != NULL && strncmp(p, "/sda", 4) != 0){ /* remove sda* */
			//char *strchr(const char *s, int c);
			snprintf(path, sizeof(path) - 1, "/dev%s", p);

			fprintf(stdout, "step 2. %s, devnum = %d\n", path, devnum);

			for(i = 0; i < devnum; i++){ /* check mount */
				if(strcmp(mdev[i], path) == 0){
					flag = 0;
					break;
				}
			}
			
			//step 3. mount umounted usb
			if(flag == 1){
				fprintf(stdout, "need mount %s\n", path);
				
				//int snprintf(char *str, size_t size, const char *format, ...);
				snprintf(cmd, sizeof(cmd) - 1, "sudo mount %s /mnt", path);
				if(system(cmd) < 0){
					fprintf(stderr, "system() %s failed: %s\n", 
						path, strerror(errno));
				}
			}
		}

	}
	if(errno != 0){
		perror("readdir failed");
		exit(1);
	}
		

	sleep(10);

	//step 4. umount usb
	//int snprintf(char *str, size_t size, const char *format, ...);
	snprintf(cmd, sizeof(cmd) - 1, "sudo umount /mnt");
	if(system(cmd) < 0){
		fprintf(stderr, "system() %s failed: %s\n", 
			path, strerror(errno));
	}
	
	//step 5. close dir
	//int closedir(DIR *dirp);
	closedir(dirp);
	fclose(mtab);

	return 0;

}

