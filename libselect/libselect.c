
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
#include <sys/types.h>
#include <sys/stat.h>
#include <inttypes.h>
#include <fcntl.h>
#include <ctype.h>

#if defined(_WIN32)
#define ORIG_WIN32 1
#endif

#if defined(_WIN32) || defined(__CYGWIN__)
#include <windows.h>
#define WINPIPES 1 /* convenience macro to enable these features
		      and keep the decision logic in one place */
#endif

#if defined(ORIG_WIN32)
#define snprintf _snprintf
#endif

#if defined(__MINGW32__)
  #include <winsock2.h>
#else
  #include <sys/select.h>
#endif

#include "arch/common/hardware.h"
#include "liblogger/logger.h"
#include "libselect_fifomem.h"
#include "libselect_socket.h"
#include "libselect_file.h"
#include "libselect.h"


/****************************************
 * DEBUG
 * 
 * DMSG is used for general tracer messages
 * while debugging select code
 ****************************************/

// #undef DEBUG

#if defined(DEBUG)
#define DMSG(x...) VERBOSE(3,x)
#define DMSG_BK(x...) VERBOSE(3,x)
#else
#define DMSG(x...) do {} while(0)
#define DMSG_BK(x...) do {} while(0)
#endif

/****************************************
 * For performance purpose and because this
 * is an early version most of the libselect
 * dimensions are fixed
 ****************************************/

#define SELECT_SKIP_UPDATES_e
#if defined(SELECT_SKIP_UPDATES)
#define LIBSELECT_UPDATE_SKIP   200
#endif

#define DEFAULT_FIFO_SIZE       5120
#define LIBSELECT_MAX_ENTRY     20
#define BUFFER_MAX              DEFAULT_FIFO_SIZE /* max 64ko == IP datagram max size */

/****************************************
 * libselect internal structure
 *
 ****************************************/

/*
  NONE         : error id
  FILE         : input/output data from a file descriptor
  TCP_LISTEN   : tcp listen socket
  TCP_SERV     : tcp socket after accept has been called
  FD_ONLY      : I/O data is not handled by libselect
*/

enum entry_type_t {
  ENTRY_NONE       = 0,
  ENTRY_FILE       = 1,
  ENTRY_TCP        = 2,
  ENTRY_TCP_SRV    = 3,
  ENTRY_UDP        = 4,
  ENTRY_FD_ONLY    = 5,
  ENTRY_WIN32_PIPE,
};

struct libselect_entry_t {
  enum entry_type_t         entry_type;    /* type of Entry                      */
  int                       registered;    /* in use ?                           */

  int                       fd_in;         /* from fd to wsim                    */
  int                       fd_out;        /* from wsim to fd                    */
  struct libselect_socket_t skt;           /* unix socket                        */

  unsigned int              fifo_size;     /* i/o fifo size                      */
  libselect_fifo_t          fifo_input;    /* input data fifo : from fd to wsim  */
  libselect_fifo_t          fifo_output;   /* output data fifo : from wsim to fd */

  libselect_callback        callback;      /* callback function()                */
  void                     *cb_ptr;        /* registered data for callback func. */
  unsigned int              signal;        /* signal associated with fifo events */

  int                       backtrack;     /* should we commit on save           */
};

struct libselect_t {
  struct libselect_entry_t entry[LIBSELECT_MAX_ENTRY];
  int state;   /* number of id                    */
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
  int id;
  memset(&libselect, 0, sizeof(struct libselect_t));
  libselect_init_done  = 1;
  libselect_update_ptr = NULL;
  for(id=0; id < LIBSELECT_MAX_ENTRY; id++)
    {
      libselect.entry[id].entry_type = ENTRY_NONE;
      libselect.entry[id].fd_in      = -1;
      libselect.entry[id].fd_out     = -1;
      libselect.entry[id].registered = 0;
      libselect.entry[id].signal     = 0;
      libselect.entry[id].backtrack  = 0;
      libselect.entry[id].callback   = NULL;
    }
  return 0;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

int libselect_close (void)
{
  int id;
  for (id=0; id < LIBSELECT_MAX_ENTRY; id++)
    {
      if (libselect.entry[id].entry_type != ENTRY_NONE &&
	  libselect.entry[id].entry_type != ENTRY_FD_ONLY)
	{
	  libselect_id_close(id);
	}
    }
  return 0;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

char* entry_type_str(int type)
{
  switch (type)
    {
    case ENTRY_NONE:
      return "NONE";
    case ENTRY_FILE:
      return "FILE";
    case ENTRY_UDP:
      return "UDP";
    case ENTRY_TCP:
      return "TCP";
    default:
      return "Unknown";
    }
}

static inline int libselect_max(int a, int b) { return ((a)<(b) ? (b):(a)); }

int libselect_update_registered()
{
  int res;
  int id, n = 0;
  int fd_max;
  fd_set readfds;
  struct timeval timeout;
  unsigned char buffer[BUFFER_MAX];

#if defined(SELECT_SKIP_UPDATES)
  static int skippy = 0;
  if (skippy < LIBSELECT_UPDATE_SKIP)
    {
      skippy++;
      return 0;
    }
  skippy = 0;
#endif

#if defined(__MINGW32__) 
	  /* comparison between signed and unsigned warning */
#define MINGWMOD (unsigned)
#else
#define MINGWMOD
#endif

  fd_max=0;
  FD_ZERO(&readfds);
  for(id=0; id < LIBSELECT_MAX_ENTRY; id++)
    {
      switch (libselect.entry[id].entry_type)
	{
	case ENTRY_NONE:
	  break;
	case ENTRY_WIN32_PIPE:
	  break;
	case ENTRY_FILE:
	case ENTRY_TCP:
	case ENTRY_TCP_SRV:
	case ENTRY_UDP:
	case ENTRY_FD_ONLY:
	  if (libselect.entry[id].registered)
	    {
	      /* *DMSG("wsim:libselect:update: add id=%d fd=%d\n",id,libselect.entry[id].fd_in); */
  	      FD_SET(MINGWMOD libselect.entry[id].fd_in, &readfds);
	      fd_max = libselect_max(libselect.entry[id].fd_in , fd_max);
	    }
	  break;
	}
    }

  timeout.tv_sec = 0;
  timeout.tv_usec = 0;
  switch (res = select(fd_max + 1, &readfds, NULL, NULL, &timeout))
    {
    case -1: /* error */
      perror("wsim:libselect:update: error during select(), host interrupt\n");
      mcu_signal_set(SIG_HOST);
      return 1;
    case 0:  /* timeout */
      // DMSG("wsim:libselect: nothing to read on timeout\n");
      return 0;
    default: /* something to read */
      break;
    }

  for(id=0; id < LIBSELECT_MAX_ENTRY; id++)
    {
      int fd_in = libselect.entry[id].fd_in;
      if (libselect.entry[id].registered && FD_ISSET(fd_in,&readfds))
	{
	  switch (libselect.entry[id].entry_type)
	    {
	    case ENTRY_NONE:
	      ERROR("wsim:libselect:update: select returns on fd entry type NONE (id=%d)\n",id);
	      break;
	      
	    case ENTRY_FILE:
	    case ENTRY_UDP:
	    case ENTRY_TCP:
	      switch (n = read(fd_in,buffer,BUFFER_MAX)) 
		{
		case -1:
		  ERROR("wsim:libselect:update: error on descriptor (id=%d:%d) type %s\n",id,fd_in,
			entry_type_str(libselect.entry[id].entry_type) );
		case 0:
		  if (libselect.entry[id].callback)
		    {
		      WARNING("wsim:libselect:update: fifo id %d has been closed\n",id);
		      libselect.entry[id].callback(id,LIBSELECT_EVT_CLOSE,libselect.entry[id].cb_ptr);
		    }
		  libselect_id_unregister(id);
		  break;
		default:
		  DMSG("wsim:libselect:update: something to read on id %d = %d bytes\n",id,n);
		  if (libselect_fifo_putblock(libselect.entry[id].fifo_input,buffer,n) < n)
		    {
		      ERROR("wsim:libselect:update: overrun on descriptor %d\n",id);
		    }
		  break;
		}
	      break;
	      
	    case ENTRY_TCP_SRV:
	      if (fd_in == libselect.entry[id].skt.socket_listen)
		{
		  if (libselect_skt_accept( & libselect.entry[id].skt )) 
		    {
		      libselect.entry[id].skt.socket = -1;
		      libselect.entry[id].fd_in      = libselect.entry[id].skt.socket_listen;
		      libselect.entry[id].fd_out     = -1;
		    }
		  else
		    {
		      DMSG("wsim:libselect:update:accepted connection on port %d\n",libselect.entry[id].skt.port);
		      libselect.entry[id].fd_in  = libselect.entry[id].skt.socket;
		      libselect.entry[id].fd_out = libselect.entry[id].skt.socket;
		    }
		}
	      else 
		{
		  if ((n = read(fd_in,buffer,BUFFER_MAX)) > 0)
		    {
		      DMSG("wsim:libselect:update: something to read on id %d = %d bytes\n",id,n);
		      if (libselect_fifo_putblock(libselect.entry[id].fifo_input,buffer,n) < n)
			{
			  ERROR("wsim:libselect:update: fifo overrun on descriptor %d\n",id);
			}
		    }
		  else
		    {
		      libselect.entry[id].skt.socket = -1;
		      libselect.entry[id].fd_in      = libselect.entry[id].skt.socket_listen;
		      libselect.entry[id].fd_out     = -1;
		      DMSG("wsim:libselect:update: read id %d returned %d\n",id,n);
		      if (libselect.entry[id].callback)
			{
			  WARNING("wsim:libselect:update: fifo id %d has been closed\n",id);
			  libselect.entry[id].callback(id,LIBSELECT_EVT_CLOSE,libselect.entry[id].cb_ptr);
			}
		    }
		}
	      break;

	    case ENTRY_WIN32_PIPE:
	      break;

	    case ENTRY_FD_ONLY:
	      mcu_signal_add(libselect.entry[id].signal);
	      DMSG("wsim:libselect: something to read on id %d (signal)\n",id);
	      break;
	    } /* switch */
	} /* if registered and isset */
    } /* for (id) */
  return 0;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

libselect_id_t libselect_id_create(char *argname, int UNUSED flags)
{
  int  id;
  char *cmdline;

  if (libselect_init_done == 0)
    {
      ERROR("wsim:libselect: id_create before libselect_init()\n");
      return -1;
    }

  /* find the first available id */
  for(id=0; id < LIBSELECT_MAX_ENTRY; id++)
    {
      if (libselect.entry[id].entry_type == ENTRY_NONE)
	{
	  break;
	}
    }

  if (id == LIBSELECT_MAX_ENTRY)
    {
      return -1;
    }

  cmdline = argname;
  if (strstr(cmdline,"bk:") == cmdline)
    {
      cmdline += 3;
      DMSG("wsim:libselect: open file %s with backtrack buffer on output\n",cmdline);
      libselect.entry[id].backtrack = 1;
    }
  else
    {
      libselect.entry[id].backtrack = 0;
    }

  libselect.entry[id].signal      = 0;
  libselect.entry[id].callback    = NULL;
  libselect.entry[id].cb_ptr      = NULL;
  libselect.entry[id].fifo_size   = DEFAULT_FIFO_SIZE;
  libselect.entry[id].fifo_input  = NULL;
  libselect.entry[id].fifo_output = NULL;
  
  if (strstr(cmdline,"tcp:s:") == cmdline)
    {
      if (libselect_skt_init(& libselect.entry[id].skt, cmdline) == -1)
	{
	  ERROR("wsim:libselect: Cannot open TCP SRV socket %s\n",cmdline);
	  return -1;
	}
      libselect.entry[id].entry_type = ENTRY_TCP_SRV;
      libselect.entry[id].fd_in      = libselect.entry[id].skt.socket_listen;
      libselect.entry[id].fd_out     = -1;
    }
  else if (strstr(cmdline,"tcp:c:") == cmdline)
    {
      if (libselect_skt_init(& libselect.entry[id].skt, cmdline) == -1)
	{
	  ERROR("wsim:libselect: Cannot open TCP socket %s\n",cmdline);
	  return -1;
	}
      libselect.entry[id].entry_type = ENTRY_TCP;
      libselect.entry[id].fd_in      = libselect.entry[id].skt.socket;
      libselect.entry[id].fd_out     = libselect.entry[id].skt.socket;
    }
  else if (strstr(cmdline,"udp:") == cmdline)
    {
      if (libselect_skt_init(& libselect.entry[id].skt, cmdline) == -1)
	{
	  ERROR("wsim:libselect: Cannot open UDP socket %s\n",id,cmdline);
	  return -1;
	}
      libselect.entry[id].entry_type = ENTRY_UDP;
      libselect.entry[id].fd_in      = libselect.entry[id].skt.socket;
      libselect.entry[id].fd_out     = libselect.entry[id].skt.socket;
    }
#ifdef WINPIPES
  else if (strstr(cmdline,"pipe:") == cmdline)
    {
      char szPipe[MAX_PATH] = {0,};
      snprintf(szPipe, sizeof(szPipe), "\\\\.\\pipe\\%s", cmdline + 5);
      HANDLE hPipe = CreateNamedPipe(szPipe, PIPE_ACCESS_DUPLEX, PIPE_TYPE_BYTE, 1, 32768, 32768, INFINITE, NULL);
      if (hPipe == INVALID_HANDLE_VALUE)
	{
	  ERROR("wsim:libselect: Cannot create named pipe %s\n", szPipe);
	  return -1;
	}
      ConnectNamedPipe(hPipe, NULL);
      
      libselect.entry[id].entry_type = ENTRY_WIN32_PIPE;
      //libselect.entry[id].fd_in      = (int)hPipe;
      libselect.entry[id].fd_out     = (int)hPipe;
    }
#endif
  else if (strcmp(cmdline,"stdio") == 0)
    {
      libselect.entry[id].entry_type = ENTRY_FILE;
      libselect.entry[id].fd_in      = 0;
      libselect.entry[id].fd_out     = 1;
    }
  else if (strcmp(cmdline,"stdin") == 0)
    {
      libselect.entry[id].entry_type = ENTRY_FILE;
      libselect.entry[id].fd_in      = 0;
      libselect.entry[id].fd_out     = -1;
    }
  else if (strcmp(cmdline,"stdout") == 0)
    {
      libselect.entry[id].entry_type = ENTRY_FILE;
      libselect.entry[id].fd_in      = -1;
      libselect.entry[id].fd_out     = 1;
    }
  else if (strcmp(cmdline,"create") == 0)
    {
      int fd;
      char f_local [MAX_FILENAME];
      char f_remote[MAX_FILENAME];
      if ((fd = libselect_get_system_fifo(f_local, f_remote)) == -1)
	{
	  ERROR("wsim:libselect: Cannot create system fifo\n");
	  return -1;
	}
      WARNING("wsim:libselect: opening fifo in write only mode\n");
      libselect.entry[id].entry_type = ENTRY_FILE;
      libselect.entry[id].fd_in      = -1;
      libselect.entry[id].fd_out     = fd;
    }
  else 
    {
      int fd;
      if ((fd = open(cmdline,O_WRONLY)) == -1)
	{
	  ERROR("wsim:libselect: Cannot open file %s\n",cmdline);
	  return -1;
	}
      WARNING("wsim:libselect: opening fifo in write only mode\n");
      libselect.entry[id].entry_type = ENTRY_FILE;
      libselect.entry[id].fd_in      = -1;
      libselect.entry[id].fd_out     = fd;
    }

  /* allocate fifo memory if needed */
  if (libselect.entry[id].fifo_size > 0)
    {
      libselect.entry[id].fifo_input  = libselect_fifo_create( libselect.entry[id].fifo_size );
      libselect.entry[id].fifo_output = libselect_fifo_create( libselect.entry[id].fifo_size );
    }
  else
    {
      libselect.entry[id].fifo_input  = NULL;
      libselect.entry[id].fifo_output = NULL;
    }

  DMSG("wsim:libselect:create: %s, id=%d, fd_in=%d\n",cmdline,id,libselect.entry[id].fd_in);
  return id;
}

inline int libselect_id_is_valid(libselect_id_t id)
{
  int ret = 0;
  ret += (libselect.entry[id].entry_type != ENTRY_NONE);
  return ret;
}

int libselect_id_close(libselect_id_t id)
{
  switch (libselect.entry[id].entry_type)
    {
    case ENTRY_NONE:
      /* ERROR("wsim:libselect:close: error cannot close id %d of type NONE\n",id); */
      return 1;
    case ENTRY_FILE:
      break;
    case ENTRY_TCP:
      libselect_skt_close_client (& libselect.entry[id].skt);
      break;
    case ENTRY_TCP_SRV:
      libselect_skt_close_client (& libselect.entry[id].skt);
      libselect_skt_close        (& libselect.entry[id].skt);
      break;
    case ENTRY_UDP:
      libselect_skt_close_client (& libselect.entry[id].skt);
      break;
    case ENTRY_FD_ONLY:
      ERROR("wsim:libselect:close: cannot close id %d of type FD_ONLY\n",id);
      return 1;
    case ENTRY_WIN32_PIPE:
#ifdef WINPIPES
      CloseHandle((HANDLE)libselect.entry[id].fd_out);
#endif
      return 1;
    }

  libselect_fifo_delete(libselect.entry[id].fifo_input);
  libselect_fifo_delete(libselect.entry[id].fifo_output);
  libselect.entry[id].entry_type = ENTRY_NONE;
  libselect.entry[id].registered = 0;
  return 0;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

int libselect_id_register(libselect_id_t id)
{
  if (libselect_init_done == 0)
    {
      ERROR("wsim:libselect: registering fifo file descriptor before libselect_init()\n");
      return 1;
    }

  if (libselect_id_is_valid(id) == 0)
    {
      ERROR("wsim:libselect: trying to register invalid id %d\n",id);
      return 1;
    }

  if (libselect.entry[id].registered == 1)
    {
      ERROR("wsim:libselect: trying to register already registered file descriptor %d\n",id);
      return 1;
    }

  if (libselect.entry[id].fd_in == -1)
    {
      WARNING("wsim:libselect: trying to register closed IN descriptor %d\n",id);
      return 1;
    }

  DMSG("wsim:libselect:register: id=%d, fd_in=%d\n",id,libselect.entry[id].fd_in);
  libselect.entry[id].registered = 1;
  libselect.state               += 1;
  libselect_update_ptr           = libselect_update_registered;

  return 0;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

int libselect_id_unregister(libselect_id_t id)
{
  if (libselect.state == 0)
    {
      ERROR("libselect: internal state error, want to unregister id %d \n",id);
      return 1;
    }

  if (libselect_id_is_valid(id) == 0)
    {
      ERROR("libselect: trying to register invalid id %d\n",id);
      return 1;
    }

  if (libselect.entry[id].registered == 0)
    {
      ERROR("libselect: trying to un-register id %d that is not registered\n",id);
      return 1;
    }

  libselect.entry[id].registered  = 0;
  libselect.state                -= 1;
  if (libselect.state == 0)
    {
      libselect_update_ptr        = NULL;
    }
  return 0;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

int libselect_id_add_callback(libselect_id_t id, libselect_callback callback, void *ptr)
{
  if (libselect_id_is_valid(id) == 0)
    {
      ERROR("libselect: trying to add callback to invalid id %d\n",id);
      return 1;
    }

  DMSG("libselect: add callback for id %d\n",id);
  libselect.entry[id].callback = callback;
  libselect.entry[id].cb_ptr   = ptr;
  return 0;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

uint32_t libselect_id_read(libselect_id_t id, uint8_t *data, uint32_t size)
{
  uint32_t ret = 0;
  if (libselect.entry[id].fifo_input)
    {
      ret = libselect_fifo_getblock(libselect.entry[id].fifo_input,data,size);
    }
  return ret;
}

uint32_t libselect_id_write(libselect_id_t id, uint8_t *data, uint32_t size)
{
  uint32_t ret = -1;
  if (libselect.entry[id].fd_out != -1)
    {
      if (libselect.entry[id].backtrack)
	{
	  ret = libselect_fifo_putblock (libselect.entry[id].fifo_output, data, size);
	  DMSG_BK("wsim:libselect:bk: WRITE %d bytes to id=%d, fd=%d, fifo=%04d\n",
		  size, id, libselect.entry[id].fd_out,
		  libselect_fifo_size ( libselect.entry[id].fifo_output ) );
	}
      else
	{
#ifdef WINPIPES
	  if (libselect.entry[id].entry_type == ENTRY_WIN32_PIPE) {
		  DWORD r;
		  if (WriteFile((HANDLE)(libselect.entry[id].fd_out), data, size > UINT32_MAX - 1 ? UINT32_MAX - 1 : size, &r, NULL))
			  ret = r; /* safe as we limit the number of bytes */
	  else
			  ret = -1;
	  } else
#endif
	    ret = write(libselect.entry[id].fd_out, data, size);
	  DMSG("wsim:libselect: WRITE %d bytes to id=%d, fd=%d, val=%c\n",
		  size, id, libselect.entry[id].fd_out,
		  isprint(data[0]) ? data[0] : '.');
	}
    }
  return ret;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

int libselect_fd_register(int fd, unsigned int signal)
{
  int id;
  if (libselect_init_done == 0)
    {
      ERROR("libselect: registering signal file descriptor before libselect_init()\n");
      return 1;
    }

  for(id=0; id < LIBSELECT_MAX_ENTRY; id++)
    {
      if (libselect.entry[id].entry_type == ENTRY_NONE)
	{
	  libselect.entry[id].entry_type = ENTRY_FD_ONLY;
	  libselect.entry[id].fd_in      = fd;
	  libselect.entry[id].registered = 1;
	  libselect.entry[id].signal     = signal;
	  libselect.entry[id].backtrack  = 0;
	  libselect.state               += 1;
	  libselect_update_ptr           = libselect_update_registered;
	  return id;
	}
    }
  return -1;
}

int libselect_fd_unregister(int fd)
{
  int id;
  for(id=0; id < LIBSELECT_MAX_ENTRY; id++)
    {
      if (libselect.entry[id].entry_type == ENTRY_FD_ONLY &&
	  libselect.entry[id].fd_in      == fd            &&
	  libselect.entry[id].registered == 1 )
	{
	  libselect.entry[id].entry_type = ENTRY_NONE;
	  libselect.entry[id].fd_in      = -1;
	  libselect.entry[id].fd_out     = -1;
	  libselect.entry[id].registered = 0;
	  libselect.entry[id].signal     = 0;
	  libselect.entry[id].backtrack  = 0;
	  libselect.state               -= 1;
	  if (libselect.state == 0)
	    {
	      libselect_update_ptr       = NULL;
	    }
	  return id;
	}
    }
  ERROR("libselect:fd: trying to un-register fd %d that is not registered\n",fd);
  return -1;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void libselect_state_save(void)
{
  int size;
  uint8_t data[BUFFER_MAX];
  libselect_id_t id;
  for(id=0; id < LIBSELECT_MAX_ENTRY; id++)
    {
      if (libselect_id_is_valid(id) && 
	  libselect.entry[id].backtrack)
	{
	  size = libselect_fifo_size ( libselect.entry[id].fifo_output );
	  libselect_fifo_getblock    ( libselect.entry[id].fifo_output, data, size );
	  libselect_fifo_flush       ( libselect.entry[id].fifo_output) ;
	  if (size > 0)
	    {
	      if (write(libselect.entry[id].fd_out, data, size) != size)
		{
		  ERROR("wsim:libselect:bk: error on write id=%d, fd=%d\n",
			id,libselect.entry[id].fd_out);
		}
	      else
		{
		  DMSG_BK("wsim:libselect:bk: SAVE+WRITE id=%d, fd=%d, write %d bytes\n",
			  id,libselect.entry[id].fd_out,size);
		}
	    }
	  else
	    {
	      DMSG_BK("wsim:libselect:bk: SAVE id=%d, fd=%d, size %d bytes\n",
		      id,libselect.entry[id].fd_out,size);
	    }
	}      
    }
}

void libselect_state_restore(void)
{
  int size;
  libselect_id_t id;
  for(id=0; id < LIBSELECT_MAX_ENTRY; id++)
    {
      if (libselect_id_is_valid(id) &&
	  libselect.entry[id].backtrack)
	{
	  size = libselect_fifo_size ( libselect.entry[id].fifo_output );
	  libselect_fifo_flush(libselect.entry[id].fifo_output);
	  DMSG_BK("wsim:libselect:bk: RESTORE id=%d, fd=%d, cancel %d bytes\n",
		  id,libselect.entry[id].fd_out,size);
	}      
    }
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */
