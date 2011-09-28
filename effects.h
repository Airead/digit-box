/********************************************************
 * @author  Airead Fan <fgh1987168@gmail.com>		*
 * @date    2011 9月 25 20:42:35 CST			*
 ********************************************************
 *		after studying C 69 days		*
 *		after studying APUE 34 days		*
 ********************************************************/

#ifndef DB_EFFECTS_H
#define DB_EFFECTS_H

#include "screen.h"
#include "image.h"

#define DB_EFFECTS_PI 3.1415926
#define DB_EFFECTS_UP 0
#define DB_EFFECTS_DOWN 1
#define DB_EFFECTS_LEFT 2 
#define DB_EFFECTS_RIGHT 3

#define DB_EFFECTS_LEFT_TOP 4
#define DB_EFFECTS_RIGHT_TOP 5
#define DB_EFFECTS_LEFT_BOTTOM 6
#define DB_EFFECTS_RIGHT_BOTTOM 7
#define DB_EFFECTS_CENTER 8
#define DB_EFFECTS_INNER 9
#define DB_EFFECTS_OUTTER 10

int effects_img_get(FB_SCREEN *screenp, char *imagename, FB_IMAGE *imagep);
int effects_img_destory(FB_IMAGE *imagep);

int effects_img_blinds(FB_SCREEN *screenp, FB_IMAGE *imagep, int num, int direction);
int effects_img_move(FB_SCREEN *screenp, FB_IMAGE *imagep, int slow, int direction);
int effects_img_radiation(FB_SCREEN *screenp, FB_IMAGE *imagep, int speed, int rad_flag);
int effects_img_rect(FB_SCREEN *screenp, FB_IMAGE *imagep, int speed, int flag);
int effects_img_abstract(FB_SCREEN *screenp, FB_IMAGE *imagep);
int effects_img_fade(FB_SCREEN *screenp, FB_IMAGE *imagep, int speed, int flag);

#endif
