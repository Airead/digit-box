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
#include <time.h>
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
#include "font.h"
#include "text.h"
#include "weather.h"
#include "effects.h"
#include "line.h"

extern uint16_t global_key_code;

/*
 * initialize mainstatus
 */
int maindeal_mainstatus_init(struct mainstatus *status)
{
#if _DEBUG_
	char *debug_p;
	int debug_i;
#endif 
	FILE *config_fp;	/* pointer to config file */
	char *value;		/* value of config file */

	status->mode = DB_VIEW_MODE;
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

	/* initialize font */
	value = config_getvalue_byname(config_fp, "font_path");
	//int fb_font_open(char *fontname, FB_FONT *ffp);
	fb_font_open(value, &status->font);

	//int fb_font_set_charsize(FB_FONT *ffp, int fontsize);
	fb_font_set_charsize(&status->font, 150);

	/* initialize num */
	status->img_cur_pos = 0;
	status->img_mini_cur_pos = 0;
	status->img_mini_offset = 0;
	status->mp3_cur_pos = -1;
	status->mp3_pid = 0;


	/* initialize weather information */
	fprintf(stdout, "get weather information...\n");
	//int weather_getinfo(unsigned char *weatherinfo, int second);
	weather_getinfo(status->weatherinfo, 3);


	/*
	 * Detect U disk and mount it
	 * Read mp3/image list
	 */

	/* Detect U disk anm mount it */
#if _DEBUG_VIR		    /* should place after actual udisk function */
	
	memset(status->udisk_path, 0, DB_NAME_MAX + 1);
	value = config_getvalue_byname(config_fp, "usb_path_vir");
	//char *strncpy(char *dest, const char *src, size_t n);
	strncpy(status->udisk_path, value, DB_NAME_MAX);

	/* udisk_path first be used for device name */
	//int udisk_detect_vir(char *devname);
	udisk_detect_vir(status->udisk_path);

	//int udisk_mount_vir(char *devname);
	udisk_mount_vir(status->udisk_path);

#endif

#ifndef _DEBUG_VIR

	memset(status->udisk_path, 0, DB_NAME_MAX + 1);

	value = config_getvalue_byname(config_fp, "mountpath");
	strncpy(status->udisk_path, value, DB_NAME_MAX);
	
	//int udisk_detect_vir(char *devname);
	udisk_detect_mount(status->udisk_path);
	
#endif	

	/* Read mp3/image list */
	//int resource_common_list(char *usbpath, char list[][DB_NAME_MAX + 1], 
	//                        char *type);
	if((status->img_list_len = resource_common_list(status->udisk_path, status->img_list, "jpg")) < 0){
		fprintf(stderr, "resource_common_list(jpg) failed\n");
	}

	if((status->mp3_list_len = resource_common_list(status->udisk_path, status->mp3_list, "mp3")) < 0){
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
	
	/* show welcome */
	maindeal_img_view_entry(status);
	status->view_flag = 1;

	//int config_close(FILE *fp);
	config_close(config_fp);

	return 0;
}

/*
 * destory mainstatus
 */
int maindeal_mainstatus_destory(struct mainstatus *status)
{
	//int kill(pid_t pid, int sig);
	if(status->mp3_pid != 0){
		kill(status->mp3_pid, SIGINT);
	}

	//pid_t waitpid(pid_t pid, int *status, int options);
	waitpid(status->mp3_pid, NULL, 0);
	
	//int fb_screen_destory(FB_SCREEN *screenp);
	fb_screen_destory(&status->screen);

	//int fb_close(FB *fbp);
	fb_close(&status->fb);
	
	//int fb_font_close(FB_FONT *ffp);
	fb_font_close(&status->font);

	/* umount U disk */
#ifndef _DEBUG_VIR	
	if(udisk_umount(status->udisk_path) < 0){
		fprintf(stderr, "%s: udisk_umount() failed\n", __func__);
		return -1;
	}
#endif
	return 0;
}

/*
 * common deal code
 */
int maindeal_common_dealcode(struct mainstatus *status, uint16_t code)
{
	maindeal_mode(status, code);
	maindeal_option(status, code);
	maindeal_mp3(status, code);

	return 0;
}

/*
 * major mode:
 *   1. Test mode: KEY_MINUS ('-')
 *   2. View mode: KEY_1 ('2')
 *   3. Slide mode: KEY_3 ('3')
 */
int maindeal_mode(struct mainstatus *status, uint16_t code)
{
	switch(code){

	case KEY_MINUS:		/* Test mode */
		status->mode = DB_TEST_MODE;
		break;
		
	case KEY_2:		/* View mode */
		status->mode = DB_VIEW_MODE;
		break;

	case KEY_3:		/* Slide mode */
		status->mode = DB_SLIDE_MODE;
		break;

	case KEY_8:
		status->mode = DB_JOKE_MODE;
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
	case DB_TEST_MODE:	/* just test */
		maindeal_option_test(status, code);
		break;

	case DB_VIEW_MODE:	/* view image */
		maindeal_option_view(status, code);
		break;

	case DB_SLIDE_MODE:	/* auto show image */
		maindeal_option_slide(status, code);
		break;
		
	case DB_JOKE_MODE:	/* Only a joke */
		maindeal_option_joke(status, code);
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

		break;
	case KEY_DOWN:		/* play next music */

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
 * Show image at current pos (fullscreen mode)
 */
int maindeal_img_show_fullscr(struct mainstatus *status)
{
	FB_IMAGE image, retimg;
	int i;
	float proportion;

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

	/* clear screen */
	fb_screen_clear(&status->screen);

	fb_image_setpos(&image, 0, 0);

	proportion = 0.01;	
	while(proportion < 1){
		//int fb_image_enlarge(FB_IMAGE *imagep, FB_IMAGE *retimgp, float proportionx, float proportiony);
		fb_image_enlarge(&image, &retimg, proportion, proportion);

		if(retimg.width > status->screen.width ||
		   retimg.height > status->screen.height){
			/*
			 *   In the beginning I did not notice this, so I found
			 * my memory leak. It takes my many hours, I will
			 * remeber it!
			 */
			fb_image_destory(&retimg);

			break;
		}

		//int fb_screen_set_image_center(FB_SCREEN *screenp, FB_IMAGE *imagep);
		fb_screen_set_image_center(&status->screen, &retimg);

		//int fb_screen_add_image(FB_SCREEN *screenp, FB_IMAGE *imagep);
		fb_screen_add_image(&status->screen, &retimg);
		
		fb_screen_update(&status->screen);

		fb_image_destory(&retimg);
		
		proportion += 0.05;
	}

	//int fb_image_destory(FB_IMAGE *imagep);
	fb_image_destory(&image);

	return 0;
}

/*
 * Show image at current pos (fullscreen mode)
 */
int maindeal_img_show_fullscr_bak(struct mainstatus *status)
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

	/* clear screen */
	fb_screen_clear(&status->screen);

	fb_image_setpos(&image, 0, 0);

	//int fb_screen_add_image(FB_SCREEN *screenp, FB_IMAGE *imagep);
	fb_screen_add_image_fullscr(&status->screen, &image);

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
//	fprintf(stdout, "%s: ready to play %s\n", __func__,
//		status->mp3_list[i]);
#endif

	if((mp3_pid = fork()) < 0){
		fprintf(stderr, "%s: fork() failed: %s\n", __func__,
			strerror(errno));

		return -1;
	}else if(mp3_pid == 0){
		/* child process */
		if(execlp("madplay", "madplay", "-Q", status->mp3_list[i], NULL) < 0){
			fprintf(stderr, "%s: madplay %s failed: %s\n", __func__,
				status->mp3_list[i], strerror(errno));
			
			return -1;
		}

		exit(0);
	}

	/* parent process */

	status->mp3_pid = mp3_pid;

	maindeal_text_show(status);
	fb_screen_update(&status->screen);

	return mp3_pid;
}

/*
 * Deal test mode code
 */
int maindeal_option_view(struct mainstatus *status, uint16_t code)
{
	int mini_loc;

	switch(status->view_flag){
	case 0:
		switch(code){
		case KEY_SPACE:		/* view image */
			maindeal_img_view_entry(status);
			status->view_flag = 1;
			break;
		case KEY_A:		/* left screen */

			break;
		case KEY_D:		/* right screen */

			break;
		case KEY_LEFT:		/* select left image */
			maindeal_img_setcurpos(status, -1);
			maindeal_img_show_fullscr(status);
			break;
		case KEY_RIGHT:		/* select right image */
			maindeal_img_setcurpos(status, 1);
			maindeal_img_show_fullscr(status);
			break;
#if _DEBUG_
		case KEY_T:		/* Test some function */
			/* The following lot codes just for interface to test new function */
			maindeal_img_setcurpos(status, 1);
			
			{
				FB_RECT rect;
				FB_POINT point1, point2;
				FB_IMAGE image;
				int i;

				i = status->img_cur_pos;

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

				fb_image_full_image(&image, &status->screen.screen_buf[0], 
						    IMAGE_FULL_LOCK);
				fb_screen_add_image(&status->screen, &status->screen.screen_buf[0]);



#if 0				
				for(i = 0; i < 256; i++){
					fb_screen_set_trans(&status->screen, i);
					fb_screen_update_trans(&status->screen);
				}
#endif
				
			       
					
				//int fb_image_add_image(FB_IMAGE *imagep, FB_IMAGE *retimgp);
				//fb_image_add_image(&image, &status->screen.screen_buf[0]);

				//int fb_screen_add_image(FB_SCREEN *screenp, FB_IMAGE *imagep);
				//fb_screen_add_image(&status->screen, &image);

				//fb_screen_upturn_buf(&status->screen, 0);

				//int fb_image_destory(FB_IMAGE *imagep);
				fb_image_destory(&image);

				return 0;
			}

			
			break;
#endif
		case KEY_UP:		
			/* The following lot codes just for test all effects */
			maindeal_img_setcurpos(status, 1);
#if 0

			//int maindeal_effects_blinds(struct mainstatus *status, int num, int direction);
			maindeal_effects_blinds(status, 9, DB_EFFECTS_RIGHT);
			maindeal_effects_blinds(status, 9, DB_EFFECTS_LEFT);
			maindeal_effects_blinds(status, 9, DB_EFFECTS_UP);
			maindeal_effects_blinds(status, 9, DB_EFFECTS_DOWN);
			
			maindeal_effects_move(status, 10, DB_EFFECTS_DOWN);
			maindeal_effects_move(status, 10, DB_EFFECTS_UP);
			maindeal_effects_move(status, 10, DB_EFFECTS_LEFT);
			maindeal_effects_move(status, 10, DB_EFFECTS_RIGHT);

#endif
			//int maindeal_effects_radiation(struct mainstatus *status, int speed, int direction)
#if 0
			maindeal_effects_radiation(status, 20, DB_EFFECTS_LEFT_TOP);
			maindeal_effects_radiation(status, 20, DB_EFFECTS_RIGHT_TOP);
			maindeal_effects_radiation(status, 20, DB_EFFECTS_LEFT_BOTTOM);
			maindeal_effects_radiation(status, 20, DB_EFFECTS_RIGHT_BOTTOM);
			maindeal_effects_radiation(status, 20, DB_EFFECTS_CENTER);

			maindeal_effects_rect(status, 20, DB_EFFECTS_LEFT_TOP);
			maindeal_effects_rect(status, 20, DB_EFFECTS_RIGHT_TOP);
			maindeal_effects_rect(status, 20, DB_EFFECTS_LEFT_BOTTOM);
			maindeal_effects_rect(status, 20, DB_EFFECTS_RIGHT_BOTTOM);
			maindeal_effects_rect(status, 20, DB_EFFECTS_INNER);

			maindeal_effects_abstract(status);

			maindeal_effects_fade(status, 3, DB_EFFECTS_INNER);
			maindeal_effects_fade(status, 3, DB_EFFECTS_OUTTER);
#endif

			break;

		case KEY_DOWN:		/* select down image */

			break;
		default:
			break;
		}
		break;
	case 1:
		switch(code){
		case KEY_SPACE:		/* show image */
			mini_loc = status->img_mini_cur_pos + status->img_mini_offset;
			if(mini_loc >= status->img_list_len){
				mini_loc -= status->img_list_len;
			}
			status->img_cur_pos = mini_loc;
			
			maindeal_img_show_fullscr(status);
			status->view_flag = 0;
			break;
		case KEY_A:		/* left screen */
			maindeal_img_view_switch(status, -1);
			break;
		case KEY_D:		/* right screen */
			maindeal_img_view_switch(status, 1);
			break;
		case KEY_LEFT:		/* select left image */
			maindeal_img_mini_setoffset(status, -3);
			maindeal_img_view(status);
			break;
		case KEY_RIGHT:		/* select right image */
			maindeal_img_mini_setoffset(status, 3);
			maindeal_img_view(status);
			break;
		case KEY_UP:		/* select up image */
			maindeal_img_mini_setoffset(status, -1);
			maindeal_img_view(status);
			break;
		case KEY_DOWN:		/* select down image */
			maindeal_img_mini_setoffset(status, 1);
			maindeal_img_view(status);
			break;
		default:
			break;
		}
		break;
	}
	return 0;
}

/*
 * View image as matrix (n x n)
 */
int maindeal_img_view(struct mainstatus *status)
{
	FB_IMAGE retimg;
	int loc;
	float proportion;

	maindeal_img_view_add(status);

	loc = status->img_mini_cur_pos + status->img_mini_offset;
	if(loc >= status->img_list_len){
		loc = loc - status->img_list_len;
	}

	proportion = 1;

	while(proportion < 1.1){
		//int fb_image_enlarge(FB_IMAGE *imagep, FB_IMAGE *retimgp, 
		//                    float proportionx, float proportiony)
		fb_image_enlarge(&status->img_list_mini[loc], &retimg,
				 proportion, proportion);

		
		//int fb_image_entlage_setcenter(FB_IMAGE *image, FB_IMAGE *retimgp);
		fb_image_enlarge_setcenter(&status->img_list_mini[loc], &retimg);

		/* draw selected frame */
		maindeal_img_frame_draw(status, &retimg, 190, 255, 4);

		fb_screen_add_image(&status->screen, &retimg);

		maindeal_text_show(status);

		fb_screen_update(&status->screen);
		
		fb_image_destory(&retimg);
		proportion += 0.01;
	}

	maindeal_text_show(status);

	fb_screen_update(&status->screen);

	return 0;
}

/*
 * View image as matrix (n x n)
 */
int maindeal_img_view_add(struct mainstatus *status)
{
	FB_IMAGE retimg;
	int i, j, k;
	int sp;			/* space padding */
	int si;			/* image padding */
	int xpos, ypos;
	int loc;
	float proportion;

	loc = status->img_mini_cur_pos + status->img_mini_offset;
	proportion = 1;

	//int fb_screen_clear(FB_SCREEN *screenp);
	fb_screen_clear(&status->screen);

	sp = DB_VIEW_MODE_PADSPACE;
	si = DB_VIEW_MODE_PADIMAGE;

	k = status->img_mini_cur_pos;
	for(i = 0; i < DB_VIEW_MODE_NUM; i++){

		for(j = 0; j < DB_VIEW_MODE_NUM; j++){
			/* rewind */
			if(k >= status->img_list_len){
				k = 0;
			}

			xpos = (i + 1) * sp * 8 + i * si * 8;
			ypos = (j + 1) * sp * 6 + j * si * 6;
			//int fb_image_setpos(FB_IMAGE *imagep, int x, int y);
			fb_image_setpos(&status->img_list_mini[k], xpos, ypos);

			//int maindeal_img_frame_draw(struct mainstatus *status, int imgnum, 
			//    COLOR_32 startcolor, COLOR_32 stopcolor, int thick);
			maindeal_img_frame_draw(status, &status->img_list_mini[k], 50, 255, 3);
	

			//int fb_screen_add_image(FB_SCREEN *screenp, FB_IMAGE *imagep);
			fb_screen_add_image(&status->screen, 
					    &status->img_list_mini[k]);
			k++;
		}
	}

	//int fb_image_enlarge(FB_IMAGE *imagep, FB_IMAGE *retimgp, 
	//                    float proportionx, float proportiony)
	fb_image_enlarge(&status->img_list_mini[loc], &retimg,
			 proportion, proportion);

	//int fb_image_entlage_setcenter(FB_IMAGE *image, FB_IMAGE *retimgp);
	fb_image_enlarge_setcenter(&status->img_list_mini[loc], &retimg);

	/* draw selected frame */
	maindeal_img_frame_draw(status, &retimg, 190, 255, 4);

	fb_screen_add_image(&status->screen, &retimg);

	fb_image_destory(&retimg);

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
int maindeal_img_frame_draw(struct mainstatus *status, FB_IMAGE *imagep, 
			    COLOR_32 startcolor, COLOR_32 stopcolor, int thick)
{
	int i;
	int tmp;		/* RGB temporary value */
	COLOR_32 color;
	FB_POINT point1;
	FB_RECT rect;

	for(i = 0; i < thick; i++){
		tmp = startcolor + (stopcolor - startcolor) / thick * i;
		color = fb_formatRGB(tmp, tmp, tmp);	

		fb_set_pixel(&point1, imagep->x - i - 1, imagep->y - i - 1 , color);
		fb_rect_set(&rect, &point1, imagep->width + 2 * i, 
			    imagep->height + 2 * i);

		//int fb_rect_draw_nonfill(FB *fbp, FB_RECT *rectp);
		fb_rect_draw_nofill_screen(&status->screen, &rect);
	}

	return 0;
}

/*
 * Switch screen according num
 */
int maindeal_img_view_switch(struct mainstatus *status, int num)
{
	int i;
	int k;

	/* first mirror */
	//void *memcpy(void *dest, const void *src, size_t n);
	memcpy(status->screen.screen_buf[0].imagestart, 
	       status->screen.screenstart, status->screen.screensize);

	/* set image current pos */
	maindeal_img_mini_setcurpos(status, 9 * num);

	/* second mirror */
	maindeal_img_view_add(status);

	memcpy(status->screen.screen_buf[1].imagestart, 
	       status->screen.screenstart, status->screen.screensize);

	k = 0;
	for(i = 0; i < status->screen.width; i = i + k + 5){
		fb_image_setpos(&status->screen.screen_buf[0], (0 - i) * num, 0);
		fb_image_setpos(&status->screen.screen_buf[1], 
				(status->screen.width - i) * num - 1, 0);
		
		fb_screen_add_image(&status->screen, &status->screen.screen_buf[0]);
		fb_screen_add_image(&status->screen, &status->screen.screen_buf[1]);

		fb_screen_update(&status->screen);
		
		k = (status->screen.width - i) * 0.05;
	}

	maindeal_img_view(status);
	
	for(i = 0; i < 2; i++){
		fb_image_setpos(&status->screen.screen_buf[i], 0, 0);
	}

	return 0;
}

/*
 * Enlarge selected mini image
 */
int maindeal_img_view_entry(struct mainstatus *status)
{
	float proportion;

	maindeal_img_view_add(status);
	memcpy(status->screen.screen_buf[1].imagestart,
	       status->screen.screenstart, status->screen.screensize);

	proportion = 2;

	while(proportion > 1){
		fb_screen_add_image_enlarge(&status->screen, 
					    &status->screen.screen_buf[1],
					    proportion, proportion);

		fb_screen_update(&status->screen);

		proportion -= 0.4;
	}

	maindeal_img_view(status);
	
	return 0;
}

/*
 * all texts are show here
 */
int maindeal_text_show(struct mainstatus *status)
{
	unsigned char *mp3_info;

	//int fb_font_set_charsize(FB_FONT *ffp, int fontsize);
	fb_font_set_charsize(&status->font, 150);

	//int fb_text_show_cn(FB_SCREEN *screenp, unsigned char *str, FB_FONT *ffp, int x, int y)
	fb_text_show_cn(&status->screen, (unsigned char *)
			"digit-box", &status->font, 0, 30);

	fb_font_set_charsize(&status->font, 96);

	/* show weather information */
	if(strcmp((char *)status->weatherinfo, "") != 0){ 
		fb_text_show_cn(&status->screen, (unsigned char *)
				"北京天气", &status->font, 200, 25);
		
		fb_text_show_cn(&status->screen, status->weatherinfo, &status->font, 300, 25);
	}

	/* show mp3 information */
	if(status->mp3_cur_pos != -1){
		mp3_info = (unsigned char *)
			strrchr(status->mp3_list[status->mp3_cur_pos], '/') + 1;
		fb_text_show_cn(&status->screen, mp3_info, &status->font, 20, 595);
	}

	return 0;
}


/*
 * Deal test mode code
 */
int maindeal_mp3(struct mainstatus *status, uint16_t code)
{
	switch(code){
	case KEY_P:		/* play previous music */
		maindeal_mp3_play_init(status);
		maindeal_mp3_setcurpos(status, -1);
		maindeal_mp3_play(status);
		break;
	case KEY_N:		/* play next music */
		maindeal_mp3_play_init(status);
		maindeal_mp3_setcurpos(status, 1);
		maindeal_mp3_play(status);
		break;
	case KEY_K:		/* play next music */
		maindeal_mp3_play_init(status);
		break;
	case KEY_R:		/* play next music */
		maindeal_mp3_play_init(status);
		maindeal_mp3_play(status);
		break;
	default:
		break;
	}
	
	return 0;
}

/*
 * show current image with blinds effects
 */
int maindeal_effects_blinds(struct mainstatus *status, int num, int direction)
{
	FB_IMAGE image;


	effects_img_get(&status->screen, status->img_list[status->img_cur_pos],
			&image);


	effects_img_blinds(&status->screen, &image, num, direction);


	effects_img_destory(&image);

	return 0;
}

/*
 * show current image with move effects
 */
int maindeal_effects_move(struct mainstatus *status, int slow, int direction)
{
	FB_IMAGE image;


	effects_img_get(&status->screen, status->img_list[status->img_cur_pos],
			&image);


	effects_img_move(&status->screen, &image, slow, direction);


	effects_img_destory(&image);

	return 0;

}

/*
 * show current image with radiation effects
 */
int maindeal_effects_radiation(struct mainstatus *status, int speed, int rad_flag)
{
	FB_IMAGE image;


	effects_img_get(&status->screen, status->img_list[status->img_cur_pos],
			&image);


	effects_img_radiation(&status->screen, &image, speed, rad_flag);


	effects_img_destory(&image);

	return 0;

}

/*
 * show current image with radiation effects
 */
int maindeal_effects_rect(struct mainstatus *status, int speed, int flag)
{
	FB_IMAGE image;


	effects_img_get(&status->screen, status->img_list[status->img_cur_pos],
			&image);


	effects_img_rect(&status->screen, &image, speed, flag);


	effects_img_destory(&image);

	return 0;

}

int maindeal_effects_abstract(struct mainstatus *status)
{
	FB_IMAGE image;


	effects_img_get(&status->screen, status->img_list[status->img_cur_pos],
			&image);


	effects_img_abstract(&status->screen, &image);


	effects_img_destory(&image);

	return 0;
}

int maindeal_effects_fade(struct mainstatus *status, int speed, int flag)
{
	FB_IMAGE image;


	effects_img_get(&status->screen, status->img_list[status->img_cur_pos],
			&image);


	effects_img_fade(&status->screen, &image, speed, flag);


	effects_img_destory(&image);

	return 0;
}

/*
 * Deal slide mode code
 */
int maindeal_option_slide(struct mainstatus *status, uint16_t code)
{
	int running;
	uint16_t cur_key_code;

	/* main loop */
	running = 1;
	cur_key_code = 0;
	while(running){
		common_change_code(&cur_key_code, global_key_code);

		if(cur_key_code != 0 && cur_key_code != KEY_3){
				running = 0;
		}

		//maindeal_option_slide
		maindeal_option_slide_autoplay(status);

		sleep(1);
	}

	status->mode = DB_VIEW_MODE;
	status->view_flag = 1;
	maindeal_img_view_entry(status);

	return 0;
}

/*
 * slide auto play
 */
int maindeal_option_slide_autoplay(struct mainstatus *status)
{
	int speed;

	//void srand(unsigned int seed);
	srand(time(NULL));
	
	switch(rand() % 6){
	case 0:			/* show fullscr */
		//int maindeal_effects_abstract(struct mainstatus *status);
		maindeal_effects_abstract(status);
		break;
	case 1:			/* blinds effects */
		//int maindeal_effects_blinds(struct mainstatus *status, int num, int direction);
		maindeal_effects_blinds(status, rand() % 30 + 1, rand() % 4);
		break;
	case 2:			/* move effects */
		//int maindeal_effects_move(struct mainstatus *status, int slow, int direction);
		maindeal_effects_move(status, rand() % 8 + 5,  rand() % 4);
		break;
	case 3:			/* radiation effects */
		//int maindeal_effects_radiation(struct mainstatus *status, int speed, int rad_flag);
		maindeal_effects_radiation(status, rand() % 20 + 10, rand() % 7 + 4);
		break;
	case 4:			/* rect effects */
		//int maindeal_effects_rect(struct mainstatus *status, int speed, int flag);
		maindeal_effects_rect(status, rand() % 10 + 10, rand() % 6 + 4);
		break;
	case 5:
		speed = rand() % 10 + 9;
		//int maindeal_effects_fade(struct mainstatus *status, int speed, int flag);
		maindeal_effects_fade(status, speed, DB_EFFECTS_INNER);
		maindeal_effects_fade(status, speed, DB_EFFECTS_OUTTER);
		break;
	default:
		break;
	}

	maindeal_img_setcurpos(status, 1);

	return 0;
}

/*
 * Deal joke mode code
 */
int maindeal_option_joke(struct mainstatus *status, uint16_t code)
{
	FB_IMAGE retimg;
	FB_IMAGE rotaimg;
	FB_IMAGE image;
	int running;
	uint16_t cur_key_code;
	float radian;
	int loc;
	int i, j, pad;

	//int fb_image_init(FB_IMAGE *imagep, int width, int height, int components);
	fb_image_init(&rotaimg, 100, 100, 4);


	loc = status->img_mini_cur_pos + status->img_mini_offset;
	if(loc >= status->img_list_len){
		loc -= status->img_list_len;
	}

	//int effects_img_get(FB_SCREEN *screenp, char *imagename, FB_IMAGE *imagep);
	effects_img_get(&status->screen, status->img_list[loc], &image);

	//int fb_image_full_image(FB_IMAGE *imagep, FB_IMAGE *retimgp, int flag);
	fb_image_full_image(&image, &rotaimg, IMAGE_FULL_LOCK);

	//int effects_img_destory(FB_IMAGE *imagep)
	effects_img_destory(&image);

	fb_screen_clear(&status->screen);

	running = 1;
	radian = 0;
	cur_key_code = 0;
	fb_image_setpos(&rotaimg, 200, 200);

	fb_image_rotate2(&rotaimg, &retimg, radian);
	/* if you not understand, you should ignore it! it's math! */
	pad = (status->screen.width - 3 * retimg.width) / 4 / 8;

	/* main loop */
	while(running && radian > -1){
		common_change_code(&cur_key_code, global_key_code);

		if(cur_key_code != 0 && cur_key_code != KEY_8){
			running = 0;
		}
		
		//int fb_image_rotate(FB_IMAGE *imagep, FB_IMAGE *retimgp, float radian)
		fb_image_rotate2(&rotaimg, &retimg, radian);
		
		for(i = 0; i < 3; i++){
			for(j = 0; j < 3; j++){
				fb_image_setpos(&retimg, (i + 1) * pad * 8 + i * retimg.width,
						(j + 1) * pad * 6 + j * retimg.height - 50);
				fb_screen_add_image(&status->screen, &retimg);
			}
		}
		
		fb_screen_update(&status->screen);
		
		fb_image_destory(&retimg);
		
		radian += 0.05;

		usleep(1000);
	}
	
	status->mode = DB_VIEW_MODE;
	status->view_flag = 1;
	maindeal_img_view_entry(status);

	fb_image_destory(&rotaimg);

	return 0;
}
