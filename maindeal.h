/********************************************************
 * @author  Airead Fan <fgh1987168@gmail.com>		*
 * @date    2011 9月 22 13:07:28 CST			*
 ********************************************************
 *		after studying C 66 days		*
 *		after studying APUE 31 days		*
 ********************************************************/

#ifndef DB_MAINDEAL_H
#define DB_MAINDEAL_H

#include "digitbox.h"
#include "framebuffer.h"
#include "screen.h"
#include "pixel.h"


struct mainstatus{ 
	int mode;
	FB fb;			/* framebuffer struct */
	FB_SCREEN screen;
	char mp3_list[DB_LIST_MAX][DB_NAME_MAX + 1]; /* store <usbpath>/mp3/ *.mp3 */
	int mp3_list_len;
	int mp3_cur_pos;
	pid_t mp3_pid;				     /* mp3 process id */
	char img_list[DB_LIST_MAX][DB_NAME_MAX + 1]; /* store <usbpath>/img/ *.jpg */
	int img_list_len;
	int img_cur_pos;	/* current show image */
	FB_IMAGE img_list_mini[DB_LIST_MAX]; /* mini image */
	int img_mini_cur_pos;
	int img_mini_offset;
};

int maindeal_mainstatus_init(struct mainstatus *status);
int maindeal_mainstatus_destory(struct mainstatus *status);
int maindeal_common_dealcode(struct mainstatus *status, uint16_t code);
int maindeal_mode(struct mainstatus *status, uint16_t code);
int maindeal_option(struct mainstatus *status, uint16_t code);
int maindeal_img_setcurpos(struct mainstatus *status, int num);
int maindeal_option_test(struct mainstatus *status, uint16_t code);
int maindeal_img_show(struct mainstatus *status);
int maindeal_mp3_play(struct mainstatus *status);
int maindeal_mp3_play_init(struct mainstatus *status);
int maindeal_option_view(struct mainstatus *status, uint16_t code);
int maindeal_img_view(struct mainstatus *status);
int maindeal_img_get_minimg(struct mainstatus *status);
int maindeal_img_frame_draw(struct mainstatus * status, FB_IMAGE *imagep, 
			    COLOR_32 startcolor, COLOR_32 stopcolor, int thick);
int maindeal_img_show_fullscr(struct mainstatus *status);

#endif
