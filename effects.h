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

#define DB_EFFECTS_UP 0
#define DB_EFFECTS_DOWN 1
#define DB_EFFECTS_LEFT 2 
#define DB_EFFECTS_RIGHT 3


int effects_img_get(FB_SCREEN *screenp, char *imagename, FB_IMAGE *imagep);
int effects_img_destory(FB_IMAGE *imagep);

int effects_img_blinds(FB_SCREEN *screenp, FB_IMAGE *imagep, int num, int direction);
int effects_img_move(FB_SCREEN *screenp, FB_IMAGE *imagep, int slow, int direction);

#endif
