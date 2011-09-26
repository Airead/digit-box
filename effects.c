/********************************************************
 * @author  Airead Fan <fgh1987168@gmail.com>		*
 * @date    2011 9月 25 20:42:27 CST			*
 ********************************************************
 *		after studying C 69 days		*
 *		after studying APUE 34 days		*
 ********************************************************/

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "effects.h"
#include "jpeg.h"
#include "screen.h"

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


	return 0;
}

