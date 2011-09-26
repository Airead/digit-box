/********************************************************
 * @author  Airead Fan <fgh1987168@gmail.com>		*
 * @date    2011 9月 22 09:31:13 CST			*
 ********************************************************
 *		after studying C 66 days		*
 *		after studying APUE 31 days		*
 ********************************************************/

#ifndef DB_UDISK_H
#define DB_UDISK_H

#include "digitbox.h"

#define DB_DEVICE_MAX 64

int udisk_detect_mount(char *directory);
int udisk_mount(char dev_list[][DB_NAME_MAX + 1],char dev_list_mounted[][DB_NAME_MAX + 1], char *directory);
int udisk_umount(char *directory);
int udisk_detect_vir(char *devname);
int udisk_mount_vir(char *devname);
int udisk_devlist_get(char *dirpath, char dev_list[][DB_NAME_MAX + 1]);
int udisk_devlist_get_mounted(char *mtabfile, char dev_list_mounted[][DB_NAME_MAX + 1]);

#endif
