/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * tn.razy@gmail.com
 */

#include "msg.h"
#include "setting.h"

#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>

int msg_out[] =
{
    /* 
     * message out just stdout and stderror 
     * 1: stdout
     * 2: stderr
     * */
    #define XX(NAME, VALUE, FP, PRE_MSG, COLOR)             (FP & 1) + 1,
    MSG_MAP(XX)
    #undef XX

    /* end */
    0
};

char *msg_prefix[] =
{
    #define XX(NAME, VALUE, FP, PRE_MSG, COLOR)             PRE_MSG,
    MSG_MAP(XX)
    #undef XX

    NULL
};

int msg_color[] = 
{
    #define XX(NAME, VALUE, FP, PRE_MSG, COLOR)             COLOR,
    MSG_MAP(XX)
    #undef XX

    /* end */
    0
};

void prmsg(enum msg_types msg_type, const char *format, ...)
{
    	if(msg_type < 0 || msg_type >= MSG_UNKNOW)
	{
		___MSG(stderr,  msg_prefix[MSG_ERROR], 
		  		"Error message type.", 
		  		msg_color[MSG_ERROR], 
		  		AT);
		return;
	}

	char msg_buf[BUFSIZ] = {0};
	memset(&msg_buf, 0, BUFSIZ);

	va_list ap;
	va_start(ap, format);
	vsnprintf(msg_buf, sizeof msg_buf, format, ap);
	va_end(ap);

	___MSG(     msg_out[msg_type] == 2 ? stderr : stdout, 
	  			msg_prefix[msg_type],
	  			msg_buf, 
	  			msg_color[msg_type], 
	  			"");

#ifdef MSG2_FILE
    	/* ... */
    	char *logfile;
	FILE *f;

	if(msg_type != MSG_ERROR)
		return;

	logfile = cfg_get_logfile();

	f = fopen(logfile, "a+");

	if(!f && !logfile)
		return;

	fwrite(msg_buf, strlen(msg_buf), 1, f);
	fputc('\n', f);

	fclose(f);
	free(logfile);
#endif
}

void die(const char *format, ...)
{
	char buf[512] = {0};

	va_list ap;
	va_start(ap, format);
	vsnprintf(buf, sizeof buf, format, ap);
	va_end(ap);

	prmsg(MSG_ERROR, buf);

	exit(1);
}