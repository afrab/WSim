/**
 *  \file   console_utils.c
 *  \brief  WSim Console utils functions
 *  \author Antoine Fraboulet
 *  \date   2007
 **/

#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <stdlib.h>
#include <limits.h>

#include "console_utils.h"
#include "liblogger/logger.h"

/* ************************************************** */
/* ************************************************** */

int console_read(console_state cs, char *buff, int sizemax)
{
  char c;
  int eol = 0;
  int request_eol;
  int length = 0;
  
  /* ok, switch is a little bit overkill here */
  /* but it will be completed ...             */
  switch (cs->fd_input)
    {
    case 0:
      request_eol = 1; 
      break;
    default:
      /* telnet sends '1310' == '\r\n' for each newline          */
      /* eol == 2 at the end of a line after it receives '13'10' */
      request_eol = 2; 
      break;
    }

  while ((eol < request_eol) && (length < sizemax))
    {
      if (read(cs->fd_input,&c,sizeof(char)) == -1)
	{
	  return CON_IO_ERROR;
	}
      else
	{
	  /* printf("read 0x%02x\n",c); */
	  if (c == '\n' || c == '\r')
	    {
	      eol ++;
	    }
	  else
	    {
	      buff[length] = c;
	      length++;
	    }
	}
    }
  buff[length] = '\0';

  return length;
}

/* ************************************************** */
/* ************************************************** */

#define MAX_VNSPRINTF_BUF_LENGTH 256

int console_write(console_state cs, char* fmt, ...)
{
  int length;
  char buff[MAX_VNSPRINTF_BUF_LENGTH + 1];

  va_list ap;
  va_start(ap, fmt);

  /* -1 is necessary for \n -> 0x13'0x10 */
  length = vsnprintf(buff,MAX_VNSPRINTF_BUF_LENGTH - 1,fmt,ap);

  if (length >= MAX_VNSPRINTF_BUF_LENGTH)
    length = MAX_VNSPRINTF_BUF_LENGTH;

  switch (cs->fd_output)
    {
    case 1:
      /* \n is ok */
      break;
    default:
      if (buff[length - 1] == '\n')
	{
	  buff[length - 1] = '\r';
	  buff[length    ] = '\n';
	  buff[length + 1] = '\0';
	  length ++;
	}
      break;
    }

  if (write(cs->fd_output,buff,length) == -1)
    {
      ERROR("libconsole: ======================================\n");
      ERROR("libconsole: cannot write value to file descriptor \n");
      ERROR("libconsole: ======================================\n");
    }
  va_end(ap);

  return CON_IO_OK;
}

/* ************************************************** */
/* ************************************************** */

static int console_any_to_i(char *buff, long *v, int base)
{
  int ret;
  long lvl;
  lvl = strtol(buff,(char**)NULL,base);
  if ((lvl == LONG_MIN) || (lvl == LONG_MAX))
    {
      *v  = 0;
      ret = 1;
    }
  else
    {
      *v  = lvl;
      ret = 0;
    }
  return ret;
}

int console_atoi(char *buff, long *v)
{
  return console_any_to_i(buff,v,10);
}

int console_htoi(char *buff, long *v)
{
  return console_any_to_i(buff,v,16);
}

/* ************************************************** */
/* ************************************************** */
