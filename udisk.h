/********************************************************
 * @author  Airead Fan <fgh1987168@gmail.com>		*
 * @date    2011 9月 22 09:31:13 CST			*
 ********************************************************
 *		after studying C 66 days		*
 *		after studying APUE 31 days		*
 ********************************************************/

#ifndef DB_UDISK_H
#define DB_UDISK_H

int udisk_detect(char *devname);
int udisk_mount(char *devname);
int udisk_detect_vir(char *devname);
int udisk_mount_vir(char *devname);

#endif
