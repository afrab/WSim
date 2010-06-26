/**
 *  \file   missing.h
 *  \brief  Missing functions
 *  \author Antoine Fraboulet
 *  \date   2009
 **/

#ifndef __MISSING__H
#define __MISSING__H

#include "config.h"

#include <signal.h>
#if !defined(SIGPIPE)
#define SIGPIPE 13
#endif

#if defined(HAVE_STDINT_H)
#include <stdint.h>
#else
#include "arch/common/wsim_stdint.h"
#endif

#if !defined(FUNC_STRTOK_R_DEFINED)
char *strtok_r(char *str, const char *delim, char **saveptr);
#endif

#if !defined(strncpyz)
#define strncpyz(dst,src,size)			\
  do {						\
    strncpy(dst,src,size);			\
    dst[size - 1] = '\0';			\
  } while (0)
#endif

#endif
