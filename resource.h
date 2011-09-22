/********************************************************
 * @author  Airead Fan <fgh1987168@gmail.com>		*
 * @date    2011 9月 22 09:51:28 CST			*
 ********************************************************
 *		after studying C 66 days		*
 *		after studying APUE 31 days		*
 ********************************************************/

#ifndef DB_RESOURCE_H
#define DB_RESOURCE_H

#include <dirent.h>

int resource_common_list(char *usbpath, char list[][DB_NAME_MAX + 1], char *type);
int resource_get_list(DIR *dirp, char list[][DB_NAME_MAX + 1], char *type);

#endif
