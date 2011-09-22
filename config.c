/********************************************************
 * @author  Airead Fan <fgh1987168@gmail.com>		*
 * @date    2011 9月 21 20:42:44 CST			*
 ********************************************************
 *		after studying C 65 days		*
 *		after studying APUE 30 days		*
 ********************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "digitbox.h"
#include "config.h"

/*
 * open digit-box config file
 */
FILE *config_open(char *filename)
{
	FILE *fp;

	//FILE *fopen(const char *path, const char *mode);
	if((fp = fopen(filename, "rw")) == NULL){
		fprintf(stderr, "open %s failed: %s\n", filename, 
			strerror(errno));
		return NULL;
	}

	return fp;
}

/*
 * close digit-box config file
 */

int config_close(FILE *fp)
{
	fclose(fp);

	return 0;
}

/*
 * get digit-box config value of name
 *
 * note: strlen("key=value") should less than DB_NAME_MAX(default: 255)
 */
char *config_getvalue_byname(FILE *fp, char *name)
{
	int name_len;
	char buf[DB_NAME_MAX + 1];
	char *p;

	p = NULL;
	name_len = strlen(name);
	memset(buf, 0, sizeof(buf));
	memset(global_config_value, 0, DB_NAME_MAX + 1);

	//void rewind(FILE *stream);
	rewind(fp);

	//char *fgets(char *s, int size, FILE *stream);
	while(fgets(buf, sizeof(buf) - 1, fp) != NULL){
		if(buf[0] == '#'){ /* skip comment */
			continue;
		}
		
		//int strncmp(const char *s1, const char *s2, size_t n);
		if(strncmp(buf, name, name_len) == 0){
			p = strchr(buf, '=');
			p[strlen(p) - 1] = '\0';
			break;
		}
	}
	 
	if(p == NULL){
		strncpy(global_config_value, "", 1);
	}else{
		/* (p) point "=value", (p+1) point "value" */
		strncpy(global_config_value, p + 1, DB_NAME_MAX);	
	}

	return global_config_value;
}

/*
 * set digit-box config value of name
 */
int config_setvalue_byname(FILE *fp, char *name, char *value)
{
	
	return 0;
}
