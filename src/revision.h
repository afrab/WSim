
/**
 *  \file   revision.h
 *  \brief  WSim simulator revision file
 *  \author Antoine Fraboulet
 *  \date   2010
 **/

#ifndef _REVISION_H_
#define _REVISION_H_

#include <ctype.h>
 
#define SVN_REVISION "$Revision$"

static char* extract_revision_number()
{
#define REV_LIMIT 6
  int    i = 0;
  char  *p = SVN_REVISION;
  static char rev[REV_LIMIT+1];

  while (! isdigit((unsigned char)*p))
    p++; 

  while ((i<REV_LIMIT) && isdigit((unsigned char)*p))
    rev[i++] = *p++;

  rev[i] = 0;
  return rev;
}

#endif
