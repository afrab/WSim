
/**
 *  \file   revision.h
 *  \brief  WSim simulator revision file
 *  \author Antoine Fraboulet
 *  \date   2010
 **/

#ifndef _REVISION_H_
#define _REVISION_H_

#include <ctype.h>
#include "config.h"
#include "liblogger/logger.h"

#define SVN_REVISION "$Revision$" 

#define REVISION  "20110816"
#define REV_LIMIT 20

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

#if EXTRACT_REVISION != 0
static char* extract_revision_number()
{
  int    i = 0;
  char  *p = REVISION;
  static char rev[REV_LIMIT+1];

  while (! isdigit((unsigned char)*p))
    p++; 

  while ((i<REV_LIMIT) && 
         (isdigit(*p) || *p == '.' || *p=='-'))
    rev[i++] = *p++;

  rev[i] = 0;
  return rev;
}
#else
#define extract_revision_number() REVISION
#endif

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

#define VERSION_STRING()   ( "WSim version " PACKAGE_VERSION ", rev. " REVISION ", build " __DATE__ )

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

#endif
