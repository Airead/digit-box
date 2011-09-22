/********************************************************
 * @author  Airead Fan <fgh1987168@gmail.com>		*
 * @date    2011 9月 22 13:07:28 CST			*
 ********************************************************
 *		after studying C 66 days		*
 *		after studying APUE 31 days		*
 ********************************************************/

#ifndef DB_MAINDEAL_H
#define DB_MAINDEAL_H

#define DB_TEST_MODE 0
#define DB_VIEW_MODE 0

#if 0
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

#endif

struct mainstatus{
	int mode;
};

int common_dealcode(struct mainstatus *status, uint16_t code);

#endif
