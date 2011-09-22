/********************************************************
 * @author  Airead Fan <fgh1987168@gmail.com>		*
 * @date    2011 9月 22 09:22:27 CST			*
 ********************************************************
 *		after studying C 66 days		*
 *		after studying APUE 31 days		*
 ********************************************************/

/*
 * Detect umounted U disk, and get it's device name
 * stroe it in devname
 *
 * return value: 
 * if detect return 1; else return 0; error return -1
 */
int udisk_detect(char *devname)
{
	
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
int udisk_mount(char *devname)
{
	
	return 0;
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

