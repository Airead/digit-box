/********************************************************
 * @author  Airead Fan <fgh1987168@gmail.com>		*
 * @date    2011 9月 22 13:06:53 CST			*
 ********************************************************
 *		after studying C 66 days		*
 *		after studying APUE 31 days		*
 ********************************************************/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <linux/input.h>
#include "maindeal.h"
#include "framebuffer.h"
#include "screen.h"
#include "jpeg.h"
#include "config.h"
#include "udisk.h"
#include "pixel.h"
#include "resource.h"
#include "plane.h"

/*
 * initialize mainstatus
 */
int maindeal_mainstatus_init(struct mainstatus *status)
{
#if _DEBUG_
	char *debug_p;
	int debug_i;
#endif 

	char udisk_path[DB_NAME_MAX + 1];
	FILE *config_fp;	/* pointer to config file */
	char *value;		/* value of config file */

#if 0
	struct mainstatus{
		char mp3_list[DB_LIST_MAX][DB_NAME_MAX + 1]; /* store <usbpath>/mp3/ *.mp3 */
		int mp3_list_len;
		char img_list[DB_LIST_MAX][DB_NAME_MAX + 1]; /* store <usbpath>/img/ *.jpg */
		int img_list_len;
	};
#endif
	status->mode = DB_TEST_MODE;
	//FILE *config_open(char *filename);
	if((config_fp = config_open("digitbox.conf", "r")) == NULL){
		fprintf(stderr, "can't open digitbox.conf: %s\n", 
			strerror(errno));
		exit(1);
	}

#if _DEBUG_
	
	//char *config_getvalue_byname(FILE *fp, char *name);
	debug_p = config_getvalue_byname(config_fp, "fb_dev");
	fprintf(stdout, "fb_dev: %s\n", debug_p);

	debug_p = config_getvalue_byname(config_fp, "input_dev");
	fprintf(stdout, "input_dev: %s\n", debug_p);
#endif

	/* initialize framebuffer */
	value = config_getvalue_byname(config_fp, "fb_dev");
	//int fb_open(char *fbname, FB *fbp);
	if(fb_open(value, &status->fb) < 0){
		fprintf(stderr, "init framebuffer failed\n");
		exit(1);
	}
	
	/* initialize screen */
	//int fb_screen_init(FB_SCREEN *screenp, FB *fbp);
	fb_screen_init(&status->screen, &status->fb);

	/* initialize num */
	status->img_cur_pos = 0;
	status->img_mini_cur_pos = 0;
	status->img_mini_offset = 0;
	status->mp3_cur_pos = 0;
	status->mp3_pid = 0;

	//int config_close(FILE *fp);
	config_close(config_fp);

	/*
	 * Detect U disk and mount it
	 * Read mp3/image list
	 */

	/* Detect U disk anm mount it */
#if _DEBUG_VIR		    /* should place after actual udisk function */
	
	memset(udisk_path, 0, DB_NAME_MAX + 1);
	//char *strncpy(char *dest, const char *src, size_t n);
	strncpy(udisk_path, "/home/airead/study/virusb", DB_NAME_MAX);

	/* udisk_path first be used for device name */
	//int udisk_detect_vir(char *devname);
	udisk_detect_vir(udisk_path);

	//int udisk_mount_vir(char *devname);
	udisk_mount_vir(udisk_path);

#endif
	
	/* Read mp3/image list */
	//int resource_common_list(char *usbpath, char list[][DB_NAME_MAX + 1], 
	//                        char *type);
	if((status->img_list_len = resource_common_list(udisk_path, status->img_list, "jpg")) < 0){
		fprintf(stderr, "resource_common_list(jpg) failed\n");
	}

	if((status->mp3_list_len = resource_common_list(udisk_path, status->mp3_list, "mp3")) < 0){
		fprintf(stderr, "resource_common_list(jpg) failed\n");
	}

#if _DEBUG_
	fprintf(stdout, "--------------------img list--------------------\n");
	for(debug_i = 0; debug_i < status->img_list_len; debug_i++){
		fprintf(stdout, "%3d: %s\n", debug_i, status->img_list[debug_i]);
	}

	fprintf(stdout, "--------------------mp3 list--------------------\n");
	for(debug_i = 0; debug_i < status->mp3_list_len; debug_i++){
		fprintf(stdout, "%3d: %s\n", debug_i, status->mp3_list[debug_i]);
	}
#endif

	//int maindeal_img_get_minimg(struct mainstatus *status)
	if(maindeal_img_get_minimg(status) < 0){
		fprintf(stderr, "%s: maindeal_img_get_minimg() failed\n",
			__func__);

		return -1;
	}

	return 0;
}

/*
 * destory mainstatus
 */
int maindeal_mainstatus_destory(struct mainstatus *status)
{
	
	return 0;
}

/*
 * common deal code
 */
int maindeal_common_dealcode(struct mainstatus *status, uint16_t code)
{
	maindeal_mode(status, code);
	maindeal_option(status, code);
	return 0;
}

/*
 * major mode:
 *   1. Test mode: KEY_MINUS ('-')
 *   2. View mode: KEY_1 ('1')
 */

int maindeal_mode(struct mainstatus *status, uint16_t code)
{
	switch(code){

	case KEY_MINUS:		/* Test mode */
#if _DEBUG_
		fprintf(stdout, "%s: start TEST mode\n", __func__);
#endif

		status->mode = DB_TEST_MODE;
		break;
		
	case KEY_2:		/* View mode */
#if _DEBUG_
		fprintf(stdout, "%s: start VIEW mode\n", __func__);
#endif

		status->mode = DB_VIEW_MODE;
		break;
	default:
		break;
	}

	return 0;
}

/*
 * Do something according mode 
 */
int maindeal_option(struct mainstatus *status, uint16_t code)
{
	switch(status->mode){
	case DB_TEST_MODE:
		maindeal_option_test(status, code);
		break;
	case DB_VIEW_MODE:
		maindeal_option_view(status, code);
		break;
	default:
		fprintf(stderr, "%s: mode error!\n", __func__);
		break;
	}

	return 0;
}

/*
 * set image current pos 
 *
 * return current pos after change
 */
int maindeal_img_setcurpos(struct mainstatus *status, int num)
{
	status->img_cur_pos += num;

	if(status->img_cur_pos >= status->img_list_len){
		status->img_cur_pos %= status->img_list_len;
	}
	while(status->img_cur_pos < 0){
		status->img_cur_pos += status->img_list_len;
	}

	return status->img_cur_pos;
}

/*
 * set image current pos 
 *
 * return current pos after change
 */
int maindeal_img_mini_setoffset(struct mainstatus *status, int num)
{
	int len;
	
	len = DB_VIEW_MODE_NUM * DB_VIEW_MODE_NUM;
	
	status->img_mini_offset += num;

	if(status->img_mini_offset >= len){
		status->img_mini_offset %= len;
		/* deal LETT and RIGHT */
		if(abs(num) != 1){ 
			status->img_mini_offset++;
		}
		if(status->img_mini_offset == 3){ /* 2 --> 0 */
			status->img_mini_offset = 0;
		}
	}
	while(status->img_mini_offset < 0){
		status->img_mini_offset += len;
		/* deal LETT and RIGHT */
		if(abs(num) != 1){
			status->img_mini_offset--;
		}
		if(status->img_mini_offset == 5){ /* 6 --> 8 */
			status->img_mini_offset = 8;
		}
	}

	return status->img_mini_offset;
}

/*
 * set mini image current pos
 *
 * return current pos after change
 */
int maindeal_img_mini_setcurpos(struct mainstatus *status, int num)
{
	status->img_mini_cur_pos += num;

	if(status->img_mini_cur_pos >= status->img_list_len){
		status->img_mini_cur_pos %= status->img_list_len;
	}
	while(status->img_mini_cur_pos < 0){
		status->img_mini_cur_pos += status->img_list_len;
	}

	return status->img_mini_cur_pos;
}


/*
 * set mp3 current pos
 *
 * return current pos after change
 */
int maindeal_mp3_setcurpos(struct mainstatus *status, int num)
{
	status->mp3_cur_pos += num;

	if(status->mp3_cur_pos >= status->mp3_list_len){
		status->mp3_cur_pos %= status->mp3_list_len;
	}
	while(status->mp3_cur_pos < 0){
		status->mp3_cur_pos += status->mp3_list_len;
	}

	return status->mp3_cur_pos;
}

/*
 * Deal test mode code
 */
int maindeal_option_test(struct mainstatus *status, uint16_t code)
{

	switch(code){
	case KEY_SPACE:		/* show next image */
		maindeal_img_setcurpos(status, 1);
		maindeal_img_show(status);
		break;
	case KEY_LEFT:		/* show previous image */
		maindeal_img_setcurpos(status, -1);
		maindeal_img_show(status);
		break;
	case KEY_RIGHT:		/* show next image */
		maindeal_img_setcurpos(status, 1);
		maindeal_img_show(status);
		break;
	case KEY_UP:		/* play previous music */
		maindeal_mp3_play_init(status);
		maindeal_mp3_setcurpos(status, -1);
		maindeal_mp3_play(status);
		break;
	case KEY_DOWN:		/* play next music */
		maindeal_mp3_play_init(status);
		maindeal_mp3_setcurpos(status, 1);
		maindeal_mp3_play(status);
		break;
	default:
		break;
	}
	
	return 0;
	
}

/*
 * Show image at current pos
 */
int maindeal_img_show(struct mainstatus *status)
{
	FB_IMAGE image;
	int i;

	i = status->img_cur_pos;

#if _DEBUG_
//	fprintf(stdout, "ready to open %s\n", status->img_list[i]);
#endif
		
	//int fb_load_jpeg(FB_IMAGE *imagep, char *filename)
	if(fb_load_jpeg(&image, status->img_list[i]) < 0){
		fprintf(stderr, "%s: fb_load_jpeg() %s failed\n",
			__func__, status->img_list[i]);
			
		return -1;
	}
		
	//int fb_screen_optimize_image(FB_SCREEN *screenp, FB_IMAGE *imagep);
	if(fb_screen_optimize_image(&status->screen, &image) < 0){
		fprintf(stderr, "%s: fb_screen_optimize_image %s failed\n",
			__func__, status->img_list[i]);
			
		return -1;
	}
	fb_image_setpos(&image, 0, 0);

	//int fb_screen_add_image(FB_SCREEN *screenp, FB_IMAGE *imagep);
	fb_screen_add_image(&status->screen, &image);

	fb_screen_update(&status->screen);

	//int fb_image_destory(FB_IMAGE *imagep);
	fb_image_destory(&image);

	return 0;
}

/*
 * close playing process
 */
int maindeal_mp3_play_init(struct mainstatus *status)
{
	if(status->mp3_pid == 0){
		return -1;
	}

	//int kill(pid_t pid, int sig);
	if(kill(status->mp3_pid, SIGINT) < 0){
		fprintf(stderr, "%s: kill failed: %s\n", __func__,
			strerror(errno));
		
		return -1;
	}

	//pid_t waitpid(pid_t pid, int *status, int options);
	waitpid(status->mp3_pid, NULL, 0);

	status->mp3_pid = -1;

	return 0;
}

/*
 * play music at current pos
 */
int maindeal_mp3_play(struct mainstatus *status)
{
	pid_t mp3_pid;
	int i;
	
	i = status->mp3_cur_pos;

#if _DEBUG_
	fprintf(stdout, "%s: ready to play %s\n", __func__,
		status->mp3_list[i]);
#endif

	if((mp3_pid = fork()) < 0){
		fprintf(stderr, "%s: fork() failed: %s\n", __func__,
			strerror(errno));

		return -1;
	}else if(mp3_pid == 0){
		/* child process */
		if(execlp("madplay", "madplay", status->mp3_list[i], NULL) < 0){
			fprintf(stderr, "%s: madplay %s failed: %s\n", __func__,
				status->mp3_list[i], strerror(errno));
			
			return -1;
		}

		exit(0);
	}

	/* parent process */

	status->mp3_pid = mp3_pid;

	return mp3_pid;
}

/*
 * Deal test mode code
 */
int maindeal_option_view(struct mainstatus *status, uint16_t code)
{
	switch(code){
	case KEY_SPACE:
		maindeal_img_mini_setcurpos(status, 9);
		maindeal_img_view(status);
		break;
	case KEY_LEFT:
		maindeal_img_mini_setoffset(status, -3);
		maindeal_img_view(status);
		break;
	case KEY_RIGHT:
		maindeal_img_mini_setoffset(status, 3);
		maindeal_img_view(status);
		break;
	case KEY_UP:
		maindeal_img_mini_setoffset(status, -1);
		maindeal_img_view(status);
		break;
	case KEY_DOWN:
		maindeal_img_mini_setoffset(status, 1);
		maindeal_img_view(status);
		break;
	case KEY_ENTER:
		status->img_cur_pos = 
			status->img_mini_cur_pos + status->img_mini_offset;
		maindeal_img_show(status);
		break;
	case KEY_BACKSPACE:
		maindeal_img_view(status);
		break;
	default:
		break;
	}
	
	return 0;
}

/*
 * View image as matrix (n x n)
 */
int maindeal_img_view(struct mainstatus *status)
{
	int i, j, k;
	int sp;			/* space padding */
	int si;			/* image padding */
	int xpos, ypos;

	//int fb_screen_clear(FB_SCREEN *screenp);
	fb_screen_clear(&status->screen);

	sp = DB_VIEW_MODE_PADSPACE;
	si = DB_VIEW_MODE_PADIMAGE;

	k = status->img_mini_cur_pos;
	for(i = 0; i < DB_VIEW_MODE_NUM; i++){
		for(j = 0; j < DB_VIEW_MODE_NUM; j++){
			xpos = (i + 1) * sp * 8 + i * si * 8;
			ypos = (j + 1) * sp * 6 + j * si * 6;
			//int fb_image_setpos(FB_IMAGE *imagep, int x, int y);
			fb_image_setpos(&status->img_list_mini[k], xpos, ypos);
	
			//int fb_screen_add_image(FB_SCREEN *screenp, FB_IMAGE *imagep);
			fb_screen_add_image(&status->screen, 
					    &status->img_list_mini[k]);
			k++;
		}
	}


#if 0       
	fb_image_setpos(&status->img_list_mini[2], 1, 1);
	//int fb_screen_add_image(FB_SCREEN *screenp, FB_IMAGE *imagep);
	fb_screen_add_image(&status->screen, &status->img_list_mini[2]);
#endif
	
	fb_screen_update(&status->screen);

	/* draw unselected frame */
	k = status->img_mini_cur_pos;
	for(i = 0; i < DB_VIEW_MODE_NUM; i++){
		for(j = 0; j < DB_VIEW_MODE_NUM; j++){
			//int maindeal_img_frame_draw(struct mainstatus *status, int imgnum, 
			//    COLOR_32 startcolor, COLOR_32 stopcolor, int thick);
			maindeal_img_frame_draw(status, k, 50, 255, 3);
		
			k++;
		}
	}

#if _DEBUG_
//	fprintf(stdout, "%s: img_mini_offset = %d\n", __func__, 
//		status->img_mini_offset);
#endif

	/* draw selected frame */
	maindeal_img_frame_draw(status, status->img_mini_cur_pos + 
				status->img_mini_offset, 210, 255, 3);

	return 0;
}

/*
 * Get all image's mini mirror
 */
int maindeal_img_get_minimg(struct mainstatus *status)
{
	FB_IMAGE img_tmp;
	int i;
	
	for(i = 0; i < status->img_list_len; i++){
		//int fb_load_jpeg(FB_IMAGE *imagep, char *filename)
		if(fb_load_jpeg(&img_tmp, status->img_list[i]) < 0){
			fprintf(stderr, "%s: fb_load_jpeg() %s failed\n",
				__func__, status->img_list[i]);
			
			return -1;
		}
		
		//int fb_screen_optimize_image(FB_SCREEN *screenp, FB_IMAGE *imagep);
		if(fb_screen_optimize_image(&status->screen, &img_tmp) < 0){
			fprintf(stderr, "%s: fb_screen_optimize_image %s failed\n",
				__func__, status->img_list[i]);
			
			return -1;
		}

		
		fprintf(stdout, "get mini image: %s\n", 
			strrchr(status->img_list[i], '/') + 1);
		
		//int fb_image_getmini(FB_IMAGE *imagep, FB_IMAGE *retimgp, 
		//                    int img_width, int img_height)
		fb_image_getmini(&img_tmp, &status->img_list_mini[i], 
				 DB_VIEW_MODE_PADIMAGE * 8,
				 DB_VIEW_MODE_PADIMAGE * 6);
		
		
		//int fb_image_destory(FB_IMAGE *imagep);
		fb_image_destory(&img_tmp);
	}

	return 0;
}

/*
 * Draw a frame for image,
 * @imgnum: status->img_list_mini[imgnum];
 */
int maindeal_img_frame_draw(struct mainstatus *status, int imgnum, 
			    COLOR_32 startcolor, COLOR_32 stopcolor, int thick)
{
	int i;
	int tmp;		/* RGB temporary value */
	COLOR_32 color;
	FB_IMAGE *imagep;
	FB_POINT point1;
	FB_RECT rect;

	imagep = &status->img_list_mini[imgnum];
	
	for(i = 0; i < thick; i++){
		tmp = startcolor + (stopcolor - startcolor) / thick * i;
		color = fb_formatRGB(tmp, tmp, tmp);	

		fb_set_pixel(&point1, imagep->x - i - 1, imagep->y - i - 1 , color);
		fb_rect_set(&rect, &point1, imagep->width + 2 * i, 
			    imagep->height + 2 * i);

		//int fb_rect_draw_nonfill(FB *fbp, FB_RECT *rectp);
		fb_rect_draw_nofill(&status->fb, &rect);
	}

	return 0;
}
