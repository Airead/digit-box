/********************************************************
 * @author  Airead Fan <fgh1987168@gmail.com>		*
 * @date    2011 9月 22 09:22:27 CST			*
 ********************************************************
 *		after studying C 66 days		*
 *		after studying APUE 31 days		*
 ********************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/mount.h>
#include "digitbox.h"
#include "udisk.h"

/*
 * Detect umounted U disk, and get it's device name
 * stroe it in devname
 *
 * return value: 
 * if detect return 1; else return 0; error return -1
 */
int udisk_detect_mount(char *directory)
{
	char *dirpath;
	char *mtabfile;
	char dev_list[DB_DEVICE_MAX][DB_NAME_MAX + 1];
	char dev_list_mounted[DB_DEVICE_MAX][DB_NAME_MAX + 1];
	
	//int mkdir(const char *pathname, mode_t mode);
	if(mkdir(directory, 0755) < 0){
		if(errno == EEXIST){
			
		}else{
			fprintf(stderr, "%s: mkdir %s failed: %s\n", __func__,
				directory, strerror(errno));
			return -1;
		}
	}

	
	dirpath = "/dev/disk/by-uuid";
	mtabfile = "/etc/mtab";

	/* get device list */
	udisk_devlist_get(dirpath, dev_list);

#if _DEBUG_
	int i;
	fprintf(stdout, "--------------------device list--------------------\n");
	i = 0;
	while(dev_list[i][0] != '\0'){
		fprintf(stdout, "%s: dev_list[%d]: %s\n", __func__,
			i, dev_list[i]);
		i++;
	}
	fprintf(stdout, "--------------------device list end--------------------\n\n");
#endif
	
	/* get devlist mounted */
	udisk_devlist_get_mounted(mtabfile, dev_list_mounted);
#if _DEBUG_
	i = 0;
	fprintf(stdout, "--------------------mounted device list--------------------\n");
	while(dev_list_mounted[i][0] != '\0'){
		fprintf(stdout, "%s: dev_list_mounted[%d]: %s\n", __func__,
			i, dev_list_mounted[i]);
		i++;
	}
	fprintf(stdout, "--------------------mounted device list end--------------------\n\n");
#endif
	
	/* mount umounted device */	
	if(udisk_mount(dev_list, dev_list_mounted, directory) < 0){
		fprintf(stderr, "%s: udisk_mount() failed\n", __func__);
		return -1;
	}
	
	return 0;
}

/*
 * Mount U disk by device name
 *
 * return usb path
 *
 * note:
 *   this path should not end with '/'
 */
int udisk_mount(char dev_list[][DB_NAME_MAX + 1], 
		char dev_list_mounted[][DB_NAME_MAX + 1], char *directory)
{
	int i, j;
	char cmd[DB_NAME_MAX + 1];
	int flag; 		/* 1 mount, 0 not mount, 2 success */
	
	fprintf(stdout, "--------------------udisk mount--------------------\n");
	i = 0;

	flag = 1;
	while(dev_list[i][0] != '\0'){
		j = 0;
		while(dev_list_mounted[j][0] != '\0'){
			if(strcmp(dev_list[i], dev_list_mounted[j]) == 0){
				flag = 0;
				break;
			}
			j++;
		}

		if(flag == 1){
			fprintf(stdout, "mount %s to %s\n", dev_list[i], directory); 
			snprintf(cmd, DB_NAME_MAX, "sudo mount %s %s\n", 
				 dev_list[i], directory);
#if _DEBUG_
			fprintf(stdout, "%s: cmd: %s\n", __func__, cmd);
#endif	
			if(system(cmd) < 0){
				return -1;
			}
			
			flag = 2;
			break;
		}
		i++;
	}
	fprintf(stdout, "--------------------udisk mount end--------------------\n\n");
	if(flag == 2){
		return 0;
	}else{
		return -1;
	}
}

/*
 * Virtual umounted U disk function for test
 */
int udisk_detect_vir(char *devname)
{
	return 0;
}

/*
 * Virtual mount U disk function for test
 */
int udisk_mount_vir(char *devname)
{
	return 0;
}

/*
 * Store device name to dev_list
 */
int udisk_devlist_get(char *dirpath, char dev_list[][DB_NAME_MAX + 1])
{
	struct dirent *direntp;
	int i;
	char buf[255];
	char path[255];
	char *p;
	ssize_t n;
	DIR *dirp;
	
	//DIR *opendir(const char *name);
	if((dirp = opendir(dirpath)) == NULL){
		fprintf(stdout, "opendir %s failed\n", dirpath);
		exit(1);
	}else{
		fprintf(stdout, "opendir %s successfully\n", dirpath);
	}
	
	i = 0;
	errno = 0; 
	//struct dirent *readdir(DIR *dirp);
	while((direntp = readdir(dirp)) != NULL){
		if(direntp->d_name[0] == '.'){ /* remove . and .. */
			continue;
		}
		//int snprintf(char *str, size_t size, const char *format, ...);
		snprintf(path, sizeof(path) - 1, "%s/%s", dirpath, direntp->d_name);
		
		//ssize_t readlink(const char *path, char *buf, size_t bufsiz);
		if((n = readlink(path, buf, sizeof(buf) - 1)) < 0){
			perror("readlink failed");
			exit(1);
		}
		buf[n] = '\0';
	
		p = strrchr(buf, '/');
		
#if _DEBUG_
		fprintf(stdout, "%s: buf: %s\n", __func__, buf);
#endif
		if(p != NULL && strncmp(p, "/sda", 4) != 0){ /* remove sda* */
			//char *strchr(const char *s, int c);
			snprintf(dev_list[i], DB_NAME_MAX, "/dev%s", p);
			i++;
		}
	}
	if(errno != 0){
		perror("readdir failed");
		exit(1);
	}

	strncpy(dev_list[i], "", 1);

	return 0;
}

int udisk_devlist_get_mounted(char *mtabfile, char dev_list_mounted[][DB_NAME_MAX + 1])
{
	FILE *mtab;
	int i;

	//FILE *fopen(const char *path, const char *mode);
	if((mtab = fopen(mtabfile, "r")) == NULL){
		fprintf(stderr, "open %s failed\n", mtabfile);
		exit(1);
	}

	i = 0;
	//int fscanf(FILE *stream, const char *format, ...);
	while(fscanf(mtab, "%s", dev_list_mounted[i]) != EOF){
		//int getc(FILE *stream);
		while(getc(mtab) != '\n')
			;
	      
		fprintf(stdout, "dev_list_mounted: %s\n", dev_list_mounted[i]);
		//int strncmp(const char *s1, const char *s2, size_t n);
		if(strncmp(dev_list_mounted[i], "/dev/sd", 7) == 0){
			i++;
		}
	}
	
	strncpy(dev_list_mounted[i], "", 1);

	return 0;
}

/*
 * Umount U disk
 */
int udisk_umount(char *directory)
{
	char cmd[DB_NAME_MAX + 1];
	
	fprintf(stdout, "umount U disk from %s\n", directory); 
	snprintf(cmd, DB_NAME_MAX, "sudo umount %s\n", directory);
#if _DEBUG_
	fprintf(stdout, "%s: cmd: %s\n", __func__, cmd);
#endif	
	if(system(cmd) < 0){
		return -1;
	}

	//int rmdir(const char *pathname);
	rmdir(directory);
			
	return 0;
}
