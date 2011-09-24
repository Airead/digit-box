/********************************************************
 * @author  Airead Fan <fgh1987168@gmail.com>		*
 * @date    2011 9月 21 20:00:01 CST			*
 ********************************************************
 *		after studying C 65 days		*
 *		after studying APUE 30 days		*
 ********************************************************/

#ifndef DIGITBOX_H
#define DIGITBOX_H

#include <stdint.h>

#define DB_LIST_MAX 255
#define DB_NAME_MAX 255

#define DB_TEST_MODE 0
#define DB_VIEW_MODE 2
#define DB_SLIDE_MODE 3

/* (n+1)*sp + n*si = 100 , video mode: 800x600 */
#define DB_VIEW_MODE_NUM 3		/* matrix num */
#define DB_VIEW_MODE_PADSPACE 7		/* pad: H:W = 8:6 */
#define DB_VIEW_MODE_PADIMAGE 24	/* pad: H:W = 8:6 */


int thread_key_control_start(void);
void *thread_key_control(void *arg);
int stty_echo_off(void);
int stty_echo_on(void);
int common_change_code(uint16_t *dest, uint16_t scr);

#endif

