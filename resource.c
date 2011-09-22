/********************************************************
 * @author  Airead Fan <fgh1987168@gmail.com>		*
 * @date    2011 9月 22 09:52:03 CST			*
 ********************************************************
 *		after studying C 66 days		*
 *		after studying APUE 31 days		*
 ********************************************************/

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <dirent.h>
#include "digitbox.h"
#include "resource.h"

/*
 * Read resource(mp3/image) list from <usb path>
 *
 * return value:
 *   list length, error return -1
 * note:
 * mp3 list from <usb path>/digitbox/mp3
 * img list from <usb path>/digitbox/img
 */

int resource_common_list(char *usbpath, char list[][DB_NAME_MAX + 1], char *type)
{
	DIR *dirp;
	char path_com[DB_NAME_MAX + 1];
	char *root;
	int list_len;

	root = "digitbox";

	memset(path_com, 0, sizeof(path_com));


	/* make path like <usbpath>/mp3(or other) */
	//int snprintf(char *str, size_t size, const char *format, ...);
	snprintf(path_com, sizeof(path_com) - 1, "%s/%s/%s", usbpath, root, type);

	//DIR *opendir(const char *name);
	if((dirp = opendir(path_com)) == NULL){
		fprintf(stderr, "%s:%d: opendir %s failed: %s\n", __FILE__, 
			__LINE__, path_com, strerror(errno));
		
		return -1;
	}

	//int resource_get_list(DIR *dirp, char list[][DB_NAME_MAX + 1], char *type);
	list_len = resource_get_list(dirp, list, path_com, type);

	//int closedir(DIR *dirp);
	closedir(dirp);

	return list_len;		/* list length */
}

int resource_get_list(DIR *dirp, char list[][DB_NAME_MAX + 1], 
		      char *path_com, char *type)
{
	struct dirent *direntp;
	int list_len;
	char *p;		/* for point ".mp3"/".img" */	

	errno = 0;
	list_len = 0;
	//struct dirent *readdir(DIR *dirp);
	while((direntp = readdir(dirp)) != NULL){
		//char *strrchr(const char *s, int c);
		p = strrchr(direntp->d_name, '.') + 1;
#if _DEBUG_
//		fprintf(stdout, "%s: p->%s\n", __func__, p);
#endif
		if(p != NULL){	/* exist ".xxx" */
			if(strcmp(p, type) == 0){
				memset(list[list_len], 0, DB_NAME_MAX + 1);

				snprintf(list[list_len++], DB_NAME_MAX,"%s/%s",
					 path_com, direntp->d_name);
			}
		}
		
	}
	if(errno != 0){
		fprintf(stderr, "%s:%d: readdir failed: %s\n", __FILE__, 
			__LINE__, strerror(errno));
		
		return -1;
	}

#if _DEBUG_
	int i;
	for(i = 0; i < list_len; i++){
//		fprintf(stdout, "%s: list[%d]: %s\n", __func__, i, list[i]);
	}
#endif

	return list_len;
}

