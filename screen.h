/********************************************************
 * @author  Airead Fan <fgh1987168@gmail.com>		*
 * @date    2011 9月 07 15:22:47 CST			*
 ********************************************************
 *		after studying C 51 days		*
 *		after studying APUE 16 days		*
 ********************************************************/

#ifndef FB_SCREEN_H
#define FB_SCREEN_H

#include "framebuffer.h"
#include "image.h"

typedef struct screen{
	int width;
	int height;
	unsigned long screensize;
	unsigned long pixelbits;
	unsigned char *screenstart; /* screen mem */
	unsigned char *fb_start;    /* framebuffer mem */
	FB_IMAGE screen_buf[2];	    /* double buffer */
}FB_SCREEN;

int fb_screen_init(FB_SCREEN *screenp, FB *fbp);
int fb_screen_destory(FB_SCREEN *screenp);
int fb_screen_info(FB_SCREEN *screenp);
int fb_screen_add_image(FB_SCREEN *screenp, FB_IMAGE *imagep);
int fb_screen_optimize_image(FB_SCREEN *screenp, FB_IMAGE *imagep);
int fb_screen_update(FB_SCREEN *screenp);
int fb_screen_clear(FB_SCREEN *screenp);
int fb_screen_add_image_enlarge(FB_SCREEN *screenp, FB_IMAGE *imagep, 
				float proportionx, float proportiony);
int fb_screen_add_image_fullscr(FB_SCREEN *screenp, FB_IMAGE *imagep);
int fb_screen_set_image_center(FB_SCREEN *screenp, FB_IMAGE *imagep);
int fb_screen_upturn_buf(FB_SCREEN *screenp, int num);
int fb_screen_clear_buf(FB_SCREEN *screenp, int num);
int fb_screen_add_image_byline(FB_IMAGE *imagep, FB_SCREEN *screenp, int line);
int fb_screen_add_image_bylinev(FB_IMAGE *imagep, FB_SCREEN *screenp, int line);

#endif
