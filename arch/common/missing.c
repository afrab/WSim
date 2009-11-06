/**
 *  \file   missing.c
 *  \brief  Missing functions
 *  \author Antoine Fraboulet
 *  \date   2009
 **/

#include <stdio.h>
#include <string.h>
#include "missing.h"

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

#if !defined(FUNC_STRTOK_R_DEFINED)
/* 
 * strtok_r() is missing on windows/mingw32 so we reimplement it 
 */

char *strtok_r(char *str, const char *delim, char **saveptr)
{
  if (!str)
    str = *saveptr;

  while (*str && (strchr(delim, *str) != NULL))
    {
      str++;
    }
 
  if (*str) 
    {
        char *start = str;
        *saveptr = start + 1;
 
        while (**saveptr && (strchr(delim, **saveptr) == NULL))
	  {
            *saveptr = *saveptr + 1;
	  }

        if (**saveptr) 
	  {
            **saveptr = '\0';  
            *saveptr = *saveptr + 1;
	  }
        return start; 
    }
    return NULL;
}

#endif

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */
