/********************************************************
 * @author  Airead Fan <fgh1987168@gmail.com>		*
 * @date    2011 9月 25 20:42:27 CST			*
 ********************************************************
 *		after studying C 69 days		*
 *		after studying APUE 34 days		*
 ********************************************************/

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include "effects.h"
#include "jpeg.h"
#include "screen.h"
#include "line.h"
#include "plane.h"
#include "pixel.h"
#include "image.h"

/*
 * get a optimized image according screen
 */
int effects_img_get(FB_SCREEN *screenp, char *imagename, FB_IMAGE *imagep)
{

#if _DEBUG_
//	fprintf(stdout, "%s: open %s\n", __func__, imagename);
#endif
	//int fb_load_jpeg(FB_IMAGE *imagep, char *filename)
	if(fb_load_jpeg(imagep, imagename) < 0){
		fprintf(stderr, "%s: fb_load_jpeg() %s failed\n",
			__func__, imagename);
			
		return -1;
	}
		
	//int fb_screen_optimize_image(FB_SCREEN *screenp, FB_IMAGE *imagep);
	if(fb_screen_optimize_image(screenp, imagep) < 0){
		fprintf(stderr, "%s: fb_screen_optimize_image %s failed\n",
			__func__, imagename);
			
		return -1;
	}
	
	return 0;
}

/*
 * Destory image
 */
int effects_img_destory(FB_IMAGE *imagep)
{
	fb_image_destory(imagep);
	
	return 0;
}

/*
 * Blinds effects
 */
int effects_img_blinds(FB_SCREEN *screenp, FB_IMAGE *imagep, int num, int direction)
{
	int i, j, k;
	int skip;

	fb_clear_startpoint(screenp->fb_start, screenp->screensize);

	fb_image_full_image(imagep, &screenp->screen_buf[0], IMAGE_FULL_LOCK);

	switch(direction){
	case DB_EFFECTS_DOWN:
		skip = (screenp->screen_buf[0].height + 1) / num;
		for(i = 1; i <= skip; i++){
			for(j = 0; j < num; j++){
				k = i + skip * j;
				//int fb_image_add_framebuffer_byline(FB_IMAGE *imagep, FB), int line);
				fb_screen_add_image_byline(&screenp->screen_buf[0], screenp, k);
			}
			usleep(1000 * num);
		}
		break;

	case DB_EFFECTS_UP:
		skip = (screenp->screen_buf[0].height + 1) / num;
		for(i = 1; i <= skip; i++){
			for(j = 0; j < num; j++){
				k = i + skip * j;
				//int fb_image_add_framebuffer_byline(FB_IMAGE *imagep, FB), int line);
				fb_screen_add_image_byline(&screenp->screen_buf[0], screenp, screenp->screen_buf[0].height - k);
			}
			usleep(1000 * num);
		}
		break;

	case DB_EFFECTS_LEFT:
		skip = (screenp->screen_buf[0].width + 1) / num;
		for(i = 1; i <= skip; i++){
			if(i >= screenp->screen_buf[0].width){
				break;
			}
			for(j = 0; j < num; j++){
				k = i + skip * j;
				if(k >= screenp->screen_buf[0].width){
					break;
				}
				//int fb_image_add_framebuffer_byline(FB_IMAGE *imagep, FB), int line);
				fb_screen_add_image_bylinev(&screenp->screen_buf[0], screenp, k);
			}
			usleep(1000 * num);
		}
		break;

	case DB_EFFECTS_RIGHT:
		skip = (screenp->screen_buf[0].width + 1) / num;
		for(i = 1; i <= skip; i++){
			if(i >= screenp->screen_buf[0].width){
				break;
			}
			for(j = 0; j < num; j++){
				k = i + skip * j;
				if(k >= screenp->screen_buf[0].width){
					break;
				}
				//int fb_image_add_framebuffer_byline(FB_IMAGE *imagep, FB), int line);
				fb_screen_add_image_bylinev(&screenp->screen_buf[0], screenp, 
							   screenp->screen_buf[0].width - k);
			}
			usleep(1000 * num);
		}
		break;

	default:
		break;

	}

	memcpy(screenp->screenstart, screenp->fb_start, screenp->screensize);

	return 0;
}

/*
 * Move effects
 */
int effects_img_move(FB_SCREEN *screenp, FB_IMAGE *imagep, int slow, int direction)
{
	int i;
	int k;

	/* first mirror */
	//void *memcpy(void *dest, const void *src, size_t n);
	memcpy(screenp->screen_buf[0].imagestart, 
	       screenp->screenstart, screenp->screensize);

	//fb_clear_startpoint(screenp->fb_start, screenp->screensize);

	/* second mirror */
	fb_image_full_image(imagep, &screenp->screen_buf[1], IMAGE_FULL_LOCK);

	switch(direction){
	case DB_EFFECTS_DOWN:
		k = 0;
		for(i = 0; i < screenp->height; i = i + k + 5){
			fb_image_setpos(&screenp->screen_buf[0], 0, i);
			fb_image_setpos(&screenp->screen_buf[1], 
					0, -(screenp->height - i) - 1);
			
			fb_screen_add_image(screenp, &screenp->screen_buf[0]);
			fb_screen_add_image(screenp, &screenp->screen_buf[1]);
			
			fb_screen_update(screenp);
			
			k = (screenp->height - i) * 0.05;

			usleep(1000 * slow);
		}

		fb_screen_upturn_buf(screenp, 1);
		break;

	case DB_EFFECTS_UP:
		k = 0;
		for(i = 0; i < screenp->height; i = i + k + 5){
			fb_image_setpos(&screenp->screen_buf[0], 0, -i);
			fb_image_setpos(&screenp->screen_buf[1], 
					0, screenp->height - i - 1);
			
			fb_screen_add_image(screenp, &screenp->screen_buf[0]);
			fb_screen_add_image(screenp, &screenp->screen_buf[1]);
			
			fb_screen_update(screenp);
			
			k = (screenp->height - i) * 0.05;
			usleep(1000 * slow);
		}

		fb_screen_upturn_buf(screenp, 1);

		break;

	case DB_EFFECTS_LEFT:
		k = 0;
		for(i = 0; i < screenp->width; i = i + k + 5){
			fb_image_setpos(&screenp->screen_buf[0], -i, 0);
			fb_image_setpos(&screenp->screen_buf[1], 
					screenp->width - i - 1, 0);
			
			fb_screen_add_image(screenp, &screenp->screen_buf[0]);
			fb_screen_add_image(screenp, &screenp->screen_buf[1]);
			
			fb_screen_update(screenp);
			
			k = (screenp->width - i) * 0.05;
			usleep(1000 * slow);
		}

		fb_screen_upturn_buf(screenp, 1);

		break;

	case DB_EFFECTS_RIGHT:
		k = 0;
		for(i = 0; i < screenp->width; i = i + k + 5){
			fb_image_setpos(&screenp->screen_buf[0], i, 0);
			fb_image_setpos(&screenp->screen_buf[1], 
					-(screenp->width - i) - 1, 0);
			
			fb_screen_add_image(screenp, &screenp->screen_buf[0]);
			fb_screen_add_image(screenp, &screenp->screen_buf[1]);
			
			fb_screen_update(screenp);
			
			k = (screenp->width - i) * 0.05;
			usleep(1000 * slow);
		}

		fb_screen_upturn_buf(screenp, 1);

		break;
	default:
		break;

	}

	for(i = 0; i < 2; i++){
		fb_image_setpos(&screenp->screen_buf[i], 0, 0);
	}

	return 0;
}

/*
 * Radiation effects
 */
int effects_img_radiation(FB_SCREEN *screenp, FB_IMAGE *imagep, int speed, int rad_flag)
{
	int i, j;
	FB_POINT point1, point2;
	
	fb_screen_clear(screenp);
	fb_screen_add_image_fullscr(screenp, imagep);
	fb_screen_set_trans(screenp, 255);

	switch(rad_flag){
	case DB_EFFECTS_LEFT_TOP:
		for(i = 0; i < screenp->height; i += speed){
			for(j = 0; j < speed; j++){
				fb_set_pixel(&point1, 0, 0, 0);
				fb_set_pixel(&point2, screenp->width - 1, i + j, 0);
				fb_draw_line_screen_trans(screenp, &point1,
							  &point2, 0);
			}
			fb_screen_update_trans(screenp);
		}
	
		for(i = screenp->width - 1; i >= 0; i -= speed){
			for(j = 0; j < speed; j++){
				fb_set_pixel(&point1, 0, 0, 0);
				fb_set_pixel(&point2, i - j, screenp->height - 1, 0);
				fb_draw_line_screen_trans(screenp, &point1,
							  &point2, 0);
			}
			fb_screen_update_trans(screenp);
		}

		break;
	case DB_EFFECTS_RIGHT_TOP:
		for(i = 0; i < screenp->height; i += speed){
			for(j = 0; j < speed; j++){
				fb_set_pixel(&point1, screenp->width - 1, 0, 0);
				fb_set_pixel(&point2, 0, i + j, 0);
				fb_draw_line_screen_trans(screenp, &point1,
							  &point2, 0);
			}
			fb_screen_update_trans(screenp);
		}
	
		for(i = 0; i < screenp->width; i += speed){
			for(j = 0; j < speed; j++){
				fb_set_pixel(&point1, screenp->width - 1, 0, 0);
				fb_set_pixel(&point2, i + j, screenp->height, 0);
				fb_draw_line_screen_trans(screenp, &point1,
							  &point2, 0);
			}
			fb_screen_update_trans(screenp);
		}

		break;
	case DB_EFFECTS_LEFT_BOTTOM:
		for(i = 0; i < screenp->width; i += speed){
			for(j = 0; j < speed; j++){
				fb_set_pixel(&point1, 0, screenp->height - 1, 0);
				fb_set_pixel(&point2, i + j, 0, 0);
				fb_draw_line_screen_trans(screenp, &point1,
							  &point2, 0);
			}
			fb_screen_update_trans(screenp);
		}
	
		for(i = 0; i < screenp->height; i += speed){
			for(j = 0; j < speed; j++){
				fb_set_pixel(&point1, 0, screenp->height - 1, 0);
				fb_set_pixel(&point2, screenp->width - 1, i + j, 0);
				fb_draw_line_screen_trans(screenp, &point1,
							  &point2, 0);
			}
			fb_screen_update_trans(screenp);
		}

		break;
	case DB_EFFECTS_RIGHT_BOTTOM:
		for(i = screenp->width; i >= 0; i -= speed){
			for(j = 0; j < speed; j++){
				fb_set_pixel(&point1, screenp->width - 1, screenp->height - 1, 0);
				fb_set_pixel(&point2, i - j, 0, 0);
				fb_draw_line_screen_trans(screenp, &point1,
							  &point2, 0);
			}
			fb_screen_update_trans(screenp);
		}
	
		for(i = 0; i < screenp->height; i += speed){
			for(j = 0; j < speed; j++){
				fb_set_pixel(&point1, screenp->width - 1, screenp->height - 1, 0);
				fb_set_pixel(&point2, 0, i + j, 0);
				fb_draw_line_screen_trans(screenp, &point1,
							  &point2, 0);
			}
			fb_screen_update_trans(screenp);
		}

		break;
	case DB_EFFECTS_CENTER:
		fb_set_pixel(&point1, screenp->width / 2, screenp->height / 2, 0);
		/* right */
		for(i = 0; i < screenp->height; i+=speed){
			for(j = 0; j < speed; j++){
				fb_set_pixel(&point2, screenp->width, i + j, 0);
				fb_draw_line_screen_trans(screenp, &point1, &point2, 0);
			}
			fb_screen_update_trans(screenp);
		}
		/* bottom */
		for(i = screenp->width; i >= 0; i-=speed){
			for(j = 0; j < speed; j++){
				fb_set_pixel(&point2, i - j, screenp->height - 1, 0);
				fb_draw_line_screen_trans(screenp, &point1, &point2, 0);
			}
			fb_screen_update_trans(screenp);
		}
		/* left */
		for(i = screenp->height; i >= 0; i -= speed){
			for(j = 0; j < speed; j++){
				fb_set_pixel(&point2, 0, i - j, 0);
				fb_draw_line_screen_trans(screenp, &point1, &point2, 0);
			}
			fb_screen_update_trans(screenp);
		}
		/* top */
		for(i = 0; i < screenp->width; i+=speed){
			for(j = 0; j < speed; j++){
				fb_set_pixel(&point2, i + j, 0, 0);
				fb_draw_line_screen_trans(screenp, &point1, &point2, 0);
			}
			fb_screen_update_trans(screenp);
		}
		
		break;
	default:
		break;
	}
	return 0;
}

/*
 * Rect effects
 */
int effects_img_rect(FB_SCREEN *screenp, FB_IMAGE *imagep, int speed, int flag)
{
	int i, j;
	int cx;
	FB_RECT rect;
	FB_POINT point1;

	fb_screen_clear(screenp);
	fb_screen_add_image_fullscr(screenp, imagep);
	fb_screen_set_trans(screenp, 255);

	switch(flag){
	case DB_EFFECTS_LEFT_TOP:
		fb_set_pixel_trans(&point1, 0, 0, 0);
		for(i = 0; i < screenp->width; i += speed){
			for(j = 0; j < speed; j++){
				//int fb_rect_set(FB_RECT *fb_rectp, FB_POINT *point, int width, int height);
				fb_rect_set(&rect, &point1, i + j, i + j);
				//int fb_rect_draw_nofill_screen_trans(FB_SCREEN *screenp, FB_RECT *rectp, unsigned char trans) 
				fb_rect_draw_nofill_screen_trans(screenp, &rect, 0);
			}
			fb_screen_update_trans(screenp);
			
		}

		break;
	case DB_EFFECTS_RIGHT_TOP:
		for(i = 0; i < screenp->width; i += speed){
			for(j = 0; j < speed; j++){
				fb_set_pixel_trans(&point1, screenp->width - 1 - i - j, 0, 0);
				//int fb_rect_set(FB_RECT *fb_rectp, FB_POINT *point, int width, int height);
				fb_rect_set(&rect, &point1, i + j, i + j);
				//int fb_rect_draw_nofill_screen_trans(FB_SCREEN *screenp, FB_RECT *rectp, unsigned char trans) 
				fb_rect_draw_nofill_screen_trans(screenp, &rect, 0);
			}
			fb_screen_update_trans(screenp);
			
		}

		break;
	case DB_EFFECTS_LEFT_BOTTOM:
		for(i = 0; i < screenp->width; i += speed){
			for(j = 0; j < speed; j++){
				fb_set_pixel_trans(&point1, 0, screenp->width - 1 - i - j, 0);
				//int fb_rect_set(FB_RECT *fb_rectp, FB_POINT *point, int width, int height);
				fb_rect_set(&rect, &point1, i + j, i + j);
				//int fb_rect_draw_nofill_screen_trans(FB_SCREEN *screenp, FB_RECT *rectp, unsigned char trans) 
				fb_rect_draw_nofill_screen_trans(screenp, &rect, 0);
			}
			fb_screen_update_trans(screenp);
			
		}

		break;
	case DB_EFFECTS_RIGHT_BOTTOM:
		for(i = 0; i < screenp->width; i += speed){
			for(j = 0; j < speed; j++){
				fb_set_pixel_trans(&point1, screenp->width - 1 - i - j, 
						   screenp->height - 1 - i - j, 0);
				//int fb_rect_set(FB_RECT *fb_rectp, FB_POINT *point, int width, int height);
				fb_rect_set(&rect, &point1, i + j, i + j);
				//int fb_rect_draw_nofill_screen_trans(FB_SCREEN *screenp, FB_RECT *rectp, unsigned char trans) 
				fb_rect_draw_nofill_screen_trans(screenp, &rect, 0);
			}
			fb_screen_update_trans(screenp);
			
		}

		break;
	case DB_EFFECTS_INNER:
		cx = screenp->width / 2;

		for(i = 0; i < cx; i += speed){
			for(j = 0; j < speed; j++){
				fb_set_pixel_trans(&point1, i + j , i + j, 0);
				//int fb_rect_set(FB_RECT *fb_rectp, FB_POINT *point, int width, int height);
				fb_rect_set(&rect, &point1, screenp->width - 2 * (i + j), screenp->height );
				//int fb_rect_draw_nofill_screen_trans(FB_SCREEN *screenp, FB_RECT *rectp, unsigned char trans) 
				fb_rect_draw_nofill_screen_trans(screenp, &rect, 0);
			}
				
			fb_screen_update_trans(screenp);
			
		}
		
		break;
	default:
		break;
	}
	return 0;
}

/*
 * Abstract effects
 */
int effects_img_abstract(FB_SCREEN *screenp, FB_IMAGE *imagep)
{
	int i, j, k;
	unsigned char *p;

	fb_screen_clear(screenp);
	//int fb_image_full_image(FB_IMAGE *imagep, FB_IMAGE *retimgp, int flag);
	fb_image_full_image(imagep, &screenp->screen_buf[0], IMAGE_FULL_LOCK);
	fb_screen_add_image(screenp, &screenp->screen_buf[0]);

	for(k = 0; k < 128; k++){
		p = screenp->screenstart;
		for(i = 0; i < screenp->height; i++){
			for(j = 0; j < screenp->fwidth; j++){
				*p = *p - 2;
				p++;
				*p = *p - 2;
				p++;
				*p = *p - 2;
				p++;

				p++;
			}
		}
		fb_screen_update(screenp);
	}
	
	
	return 0;
}

/*
 * Fade effects
 *
 * OPTION: DB_EFFECTS_INNER || DB_EFFECTS_OUTTER
 */
int effects_img_fade(FB_SCREEN *screenp, FB_IMAGE *imagep, int speed, int flag)
{
	int i;

	switch(flag){
	case DB_EFFECTS_INNER:
		fb_screen_set_trans(screenp, 0);
		for(i = 0; i < 256; i += speed){
			//int fb_screen_set_trans(FB_SCREEN *screenp, unsigned char trans);
			fb_screen_set_trans(screenp, i);
			//int fb_screen_update_trans(FB_SCREEN *screenp);
			fb_screen_update_trans(screenp);
		}
		fb_screen_set_trans(screenp, 255);
		fb_screen_update_trans(screenp);
		break;

	case DB_EFFECTS_OUTTER:
		fb_screen_clear(screenp);
		fb_screen_add_image_fullscr(screenp, imagep);
		fb_screen_set_trans(screenp, 255);
		for(i = 255; i >= 0; i -= speed){
			//int fb_screen_set_trans(FB_SCREEN *screenp, unsigned char trans);
			fb_screen_set_trans(screenp, i);
			//int fb_screen_update_trans(FB_SCREEN *screenp);
			fb_screen_update_trans(screenp);
		}
		fb_screen_set_trans(screenp, 0);
		fb_screen_update_trans(screenp);
		break;
		
	default:
		break;
	}
	
	return 0;
}
