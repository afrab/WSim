/**
 *  \file   libselect_file.c
 *  \brief  libselect file API
 *  \author Antoine Fraboulet
 *  \date   2008
 **/

#if !defined(_GNU_SOURCE)
#define _GNU_SOURCE 
#endif

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
 
#include "arch/common/hardware.h"
#include "libselect/libselect.h"
#include "libselect_file.h"

/***************************************************/
/***************************************************/
/***************************************************/

#if defined(__USE_UNIX98) || defined(LINUX) || defined(SOLARIS) || defined(MACOSX)

#if !defined(__GLIBC__)
int getpt(void)
{
  return posix_openpt(O_RDWR | O_NOCTTY);
}
#endif

int libselect_get_system_fifo(char local_name[MAX_FILENAME], char remote_name[MAX_FILENAME])
{
  int fd = -1;

  if ((fd = getpt()) != -1)
    {
      if (grantpt(fd) != -1)
	{
	  if (unlockpt(fd) != -1)
	    {
	      int oflags = fcntl(fd,F_GETFL,0);
	      if (fcntl(fd,F_SETFL,oflags | O_SYNC | O_NDELAY) != -1)
		{
		  if (ttyname(fd) != NULL && ptsname(fd) != NULL)
		    {
		      strncpy(local_name ,ttyname(fd),MAX_FILENAME);
		      strncpy(remote_name,ptsname(fd),MAX_FILENAME);
		      return fd;
		    }
		}
	    }
	}

      perror("get_system_fifo:");
      close(fd);
      fd = -1;
    }
  return fd;
}

#else /* __USE_UNIX98 || ... */

int libselect_get_system_fifo(char local_name[MAX_FILENAME], char remote_name[MAX_FILENAME])
{
  strcpy(local_name,"");
  strcpy(remote_name,"");
  return -1;
}

#endif

/***************************************************/
/***************************************************/
/***************************************************/
