/********************************************************
 * @author  Airead Fan <fgh1987168@gmail.com>		*
 * @date    2011 9月 22 13:06:53 CST			*
 ********************************************************
 *		after studying C 66 days		*
 *		after studying APUE 31 days		*
 ********************************************************/

#include <stdint.h>
#include <linux/input.h>
#include "maindeal.h"

/*
 * common deal code
 */
int common_dealcode(struct mainstatus *status, uint16_t code)
{

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
		status->mode = DB_TEST_MODE;
		break;
	case KEY_1:		/* View mode */
		status->mode = DB_VIEW_MODE;
		break;
	default:
		break;
	}

	return 0;
}
