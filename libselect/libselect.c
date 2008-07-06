
/**
 * \file   libselect.c
 * \brief  Multiple client select library
 * \author Antoine Fraboulet
 * \date   2006
 **/


#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/time.h>
#include <inttypes.h>

#if defined(__MINGW32__)
  #include <winsock2.h>
#else
  #include <sys/select.h>
#endif

#include "arch/common/hardware.h"
#include "liblogger/logger.h"
#include "libselect_fifo.h"
#include "libselect.h"


/****************************************
 * DEBUG
 * 
 * DMSG is used for general tracer messages
 * while debugging select code
 ****************************************/

#undef DEBUG

#if defined(DEBUG)
#define DMSG(x...) fprintf(stderr,x)
#else
#define DMSG(x...) do {} while(0)
#endif

/****************************************
 * For performance purpose and because this
 * is an early version most of the libselect
 * dimensions are fixed
 ****************************************/

#define LIBSELECT_UPDATE_SKIP   200
#define LIBSELECT_MAX_ENTRY 20

/****************************************
 * libselect internal structure
 *
 ****************************************/
enum entry_type_t {
  ENTRY_NONE   = 0,
  ENTRY_FIFO   = 1,
  ENTRY_SIGNAL = 2
};

struct libselect_entry_t {
  enum entry_type_t  entry_type;
  libselect_fifo_t   fifo_input;
  libselect_fifo_t   fifo_output;
  unsigned int       fifo_size;
  unsigned int       signal;
  int                registered;
  libselect_callback callback;
  void               *cb_ptr;
};

struct libselect_t {
  struct libselect_entry_t entry[LIBSELECT_MAX_ENTRY];
  /* number of registered fd */
  int state;
  /* max number of registered fd + 1 */
  int fd_max;
};

static struct libselect_t libselect;
static int                libselect_init_done = 0;

/*****************************************
 * libselect update function pointer
 *
 *****************************************/

int (*libselect_update_ptr)      () = NULL;
int libselect_update_registered  ();

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

int libselect_init(void)
{
  memset(&libselect, 0, sizeof(struct libselect_t));
  libselect_init_done  = 1;
  libselect_update_ptr = NULL;
  DMSG("libselect:init: done\n");
  return 0;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

static int libselect_find_fd_max()
{
  int i;
  for(i = (LIBSELECT_MAX_ENTRY - 1); i >= 0; i--)
    {
      if (libselect.entry[i].registered)
	{
	  return i;
	}
    }
  return -1; /* none */
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

#define libselect_max(a,b) ((a)<(b) ? (b):(a))

int libselect_register_fifo(int fd, unsigned int fifo_size)
{
  if (libselect_init_done == 0)
    {
      ERROR("libselect: registering fifo file descriptor before libselect_init()\n");
      return 1;
    }

  if (libselect.entry[fd].registered == 1)
    {
      ERROR("libselect: trying to register already registered file descriptor %d\n",fd);
      return 1;
    }

  DMSG("libselect: register handler %d, fifo size %d\n",fd,fifo_size);

  libselect.entry[fd].entry_type  = ENTRY_FIFO;
  libselect.entry[fd].fifo_input  = libselect_fifo_create(fifo_size);
  libselect.entry[fd].fifo_output = libselect_fifo_create(fifo_size);
  libselect.entry[fd].fifo_size   = fifo_size;
  libselect.entry[fd].signal      = 0;
  libselect.entry[fd].registered  = 1;

  libselect.state     += 1;
  libselect.fd_max     = libselect_find_fd_max() + 1; /* +1 is required here */
  libselect_update_ptr = libselect_update_registered;

  return 0;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

int libselect_register_signal(int fd, unsigned int signal)
{
  if (libselect_init_done == 0)
    {
      ERROR("libselect: registering signal file descriptor before libselect_init()\n");
      return 1;
    }

  if (libselect.entry[fd].registered == 1)
    {
      ERROR("libselect: trying to register already registered file descriptor %d\n",fd);
      return 1;
    }

  DMSG("libselect: register handler %d, signal 0x%08x\n",fd,signal);

  libselect.entry[fd].entry_type  = ENTRY_SIGNAL;
  libselect.entry[fd].fifo_input  = NULL;
  libselect.entry[fd].fifo_output = NULL;
  libselect.entry[fd].fifo_size   = 0;
  libselect.entry[fd].signal      = signal;
  libselect.entry[fd].registered  = 1;

  libselect.state     += 1;
  libselect.fd_max     = libselect_find_fd_max() + 1; /* +1 is required here */
  libselect_update_ptr = libselect_update_registered;

  return 0;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

int libselect_add_callback(int fd, libselect_callback callback, void *ptr)
{
  DMSG("libselect: add callback for fd %d\n",fd);
  libselect.entry[fd].callback = callback;
  libselect.entry[fd].cb_ptr   = ptr;
  return 1;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

int libselect_del_callback(int fd)
{
  libselect.entry[fd].callback = NULL;
  return 0;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

int libselect_unregister(int fd)
{
  if (libselect.state == 0)
    {
      ERROR("libselect: internal state error, want to unregister device %d \n",fd);
      return 1;
    }

  if (libselect.entry[fd].registered == 0)
    {
      ERROR("libselect: trying to un-register device %d that is not registered\n",fd);
      return 1;
    }

  if (libselect.entry[fd].entry_type == ENTRY_FIFO)
    {
      libselect_fifo_delete(libselect.entry[fd].fifo_input);
      libselect_fifo_delete(libselect.entry[fd].fifo_output);
      libselect.entry[fd].fifo_input  = NULL;
      libselect.entry[fd].fifo_output = NULL;
      libselect.entry[fd].fifo_size   = 0;
    }
  else
    {
      libselect.entry[fd].signal      = 0;
    }

  libselect.entry[fd].entry_type  = ENTRY_NONE;
  libselect.entry[fd].registered  = 0;

  libselect.fd_max   = libselect_find_fd_max() + 1; /* +1 is required here */
  libselect.state   -= 1;

  if (libselect.state == 0)
    {
      libselect_update_ptr = NULL;
    }
  return 0;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

#define BUFFER_MAX (64*1024) 

int libselect_update_registered()
{
  int res;
  int fd, n = 0;
  fd_set readfds;
  struct timeval timeout;
  unsigned char buffer[BUFFER_MAX];

  static int skippy = 0;
  if (skippy < LIBSELECT_UPDATE_SKIP)
    {
      skippy++;
      return 0;
    }
  skippy = 0;

  FD_ZERO(&readfds);
  for(fd=0; fd < libselect.fd_max; fd++)
    {
      if (libselect.entry[fd].registered)
	{
	  //DMSG("libselect: will read on fd %d\n",fd);
#if defined(__MINGW32__) 
	  /* comparison between signed and unsigned warning */
	  FD_SET((unsigned)fd,&readfds);
#else
	  FD_SET(fd,&readfds);
#endif
	}
    }

  timeout.tv_sec = 0;
  timeout.tv_usec = 0;
  switch (res = select(libselect.fd_max, &readfds, NULL, NULL, &timeout))
    {
    case -1: /* error */
      perror("libselect: error during select()\n");
      return 1;
    case 0:  /* timeout */
      // DMSG("libselect: nothing to read on timeout\n");
      return 0;
    default: /* something to read */
      break;
    }

  for(fd=0; fd<libselect.fd_max; fd++)
    {
      if (libselect.entry[fd].registered && FD_ISSET(fd,&readfds))
	{
	  switch (libselect.entry[fd].entry_type)
	    {
	    case ENTRY_NONE:
	      ERROR("libselect: select returns on fd entry type NONE (fd=%d)\n",fd);
	      break;
	    case ENTRY_FIFO:
	      if ((n = read(fd,buffer,BUFFER_MAX)) > 0)
		{
		  DMSG("libselect: something to read on fd %d = %d bytes\n",fd,n);
		  if (libselect_fifo_putblock(libselect.entry[fd].fifo_input,buffer,n))
		    {
		      ERROR("libselect: fifo overrun on descriptor %d\n",fd);
		    }
		}
	      else
		{
		  DMSG("libselect: read returned %d\n",n);
		  if (libselect.entry[fd].callback)
		    {
		      ERROR("libselect: fifo %d has been closed\n",fd);
		      libselect.entry[fd].callback(fd,LIBSELECT_CLOSE,libselect.entry[fd].cb_ptr);
		    }
		}
	      break;
	    case ENTRY_SIGNAL:
	      mcu_signal_add(libselect.entry[fd].signal);
	      DMSG("libselect: something to read on fd %d (signal)\n",fd);
	      break;
	    }
	}
    }
  
  return 0;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

uint32_t libselect_read(int fd, uint8_t *data, uint32_t size)
{
  return libselect_fifo_getblock(libselect.entry[fd].fifo_input,data,size);
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

uint32_t libselect_read_flush(int fd)
{
  uint32_t n = 0;

  if (libselect.entry[fd].entry_type == ENTRY_FIFO)
    {
      n += libselect_fifo_size(libselect.entry[fd].fifo_input);
      libselect_fifo_flush(libselect.entry[fd].fifo_input);
    }
  return n;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

int libselect_close (void)
{
  return 0;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */
