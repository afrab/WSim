
/**
 *  \file   logger.c
 *  \brief  Wsim logger module
 *  \author Antoine Fraboulet
 *  \date   2006
 **/

#include <stdarg.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "logger.h"

#define DEFAULT_LOGFILE      stdout
#define DEFAULT_LOGFILENAME "stdout"

#define MAXFILENAME 500

static char  logger_filename[MAXFILENAME];
static FILE* logger_logfile;
int logger_verbose_level = -1;

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

static int logger_open_logfile(const char* filename)
{
  if (strcmp(filename, "stdout") == 0)
    {
      /* REAL_STDOUT("wsim:log:pipe:stdout\n"); */
      logger_logfile = stdout;
      return 0;
    }

  if (strcmp(filename, "stderr") == 0)
    {
      /* REAL_STDOUT("wsim:log:pipe:stderr\n"); */
      logger_logfile = stderr;
      return 0;
    }
  
  /* REAL_STDOUT("wsim:log:file:%s\n",filename); */
  return (logger_logfile = fopen(filename,"wb")) == NULL ;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void logger_init(const char* filename, int verbose)
{
  if (logger_open_logfile(filename))
    {
      ERROR(" ** Cannot open logfile, defaulting to %s\n", DEFAULT_LOGFILENAME);
      logger_logfile = DEFAULT_LOGFILE;
      strcpy(logger_filename, DEFAULT_LOGFILENAME);
    }
  else
    {
      strcpy(logger_filename,filename);
      /* REAL_STDOUT("wsim:log:%s\n",filename); */
    }

  logger_verbose_level = verbose;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void logger_close()
{
  if ((logger_logfile != stdout) && (logger_logfile != stderr))
    {
      fclose(logger_logfile);
    }
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

#define MAX_VNSPRINTF_BUF_LENGTH 300

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

static void
mylog(char* fmt, va_list ap)
{
  int length;
  char buf[MAX_VNSPRINTF_BUF_LENGTH + 1];

  length = vsnprintf(buf,MAX_VNSPRINTF_BUF_LENGTH,fmt,ap);
  if (length >= MAX_VNSPRINTF_BUF_LENGTH)
    length = MAX_VNSPRINTF_BUF_LENGTH;

  if (buf[length-1] == '\n')
    {
      /* buf[--length] = '\0'; */
      fprintf(logger_logfile,"%s",buf);
    }
  else
    {
      fprintf(logger_logfile,"%s",buf);
    }

  /*  fprintf(out," @ PC=[0x%04x]\n",MCU_GET_PC()); */
  /*  fprintf(logger_logfile,"\n"); */
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void ERROR(char* fmt, ...)
{
  int length;
  va_list ap;
  char buf[MAX_VNSPRINTF_BUF_LENGTH + 1];

  va_start(ap, fmt);
  length = vsnprintf(buf,MAX_VNSPRINTF_BUF_LENGTH,fmt,ap);
  va_end(ap);

  fprintf(logger_logfile,"%s",buf);
  if ((logger_logfile != stdout) && (logger_logfile != stderr))
    {
      fprintf(stderr,"%s",buf);
    }
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void FIXME(char* fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  mylog(fmt,ap);
  va_end(ap);
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void OUTPUT(char* fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  mylog(fmt,ap);
  va_end(ap);
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

#if defined(VERBOSE_IS_A_FUNC)
void VERBOSE(int level, char* fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  if (logger_verbose_level >= level)
    {
      mylog(fmt,ap);
    }
  va_end(ap);
}
#endif

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void REAL_STDOUT(char* fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stdout,fmt,ap);
  va_end(ap);
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void REAL_STDERR(char* fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr,fmt,ap);
  va_end(ap);
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */
