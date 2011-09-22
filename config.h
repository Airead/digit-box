/********************************************************
 * @author  Airead Fan <fgh1987168@gmail.com>		*
 * @date    2011 9月 21 20:43:19 CST			*
 ********************************************************
 *		after studying C 65 days		*
 *		after studying APUE 30 days		*
 ********************************************************/

#ifndef DB_CONFIG_H
#define DB_CONFIG_H

char global_config_value[DB_NAME_MAX + 1];

FILE *config_open(char *filename, char *mode);
int config_close(FILE *fp);
char *config_getvalue_byname(FILE *fp, char *name);
int config_setvalue_byname(FILE *fp, char *name, char *value);

#endif
