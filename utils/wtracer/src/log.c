
/**
 *  \file   log.c
 *  \brief  Simulator activity tracer
 *  \author Antoine Fraboulet
 *  \date   2006
 **/

#include <stdarg.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "log.h"

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void ERROR(char* fmt, ...)
{
  FILE* out = stderr;
  va_list ap;
  va_start(ap, fmt);
  vfprintf(out,fmt,ap);
  va_end(ap);
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void OUTPUT(char* fmt, ...)
{
  FILE* out = stderr;
  va_list ap;
  va_start(ap, fmt);
  vfprintf(out,fmt,ap);
  va_end(ap);
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void MESSAGE(char* fmt, ...)
{
  FILE* out = stderr;
  va_list ap;
  va_start(ap, fmt);
  vfprintf(out,fmt,ap);
  va_end(ap);
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */
