
/**
 *  \file    fifo.c
 *  \brief   Terminal for wsim serial line emulation (device fifo)
 *  \author  Antoine Fraboulet
 *  \date    2006
 *  \license GPLv2
 **/

#if !defined(_GNU_SOURCE)
#define _GNU_SOURCE 
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <strings.h>

#include <sys/ioctl.h>

#if defined(SOLARIS)
#   include <stropts.h>
#endif

#include "fifo.h"

/* ************************************************** */     
/* ************************************************** */     
/* ************************************************** */     

#if defined(__PTTY)
#  define __FIFO_EVENT_SKIP 1
#endif

/* ************************************************** */     
/* ************************************************** */     
/* ************************************************** */     

#define ERROR(x...) fprintf(stderr,x)

#undef DEBUG
#if defined(DEBUG)
#define DBG_MSG(x...) fprintf(stderr,x)
#else
#define DBG_MSG(x...) do { } while(0)
#endif

/* ************************************************** */     
/* ************************************************** */     
/* ************************************************** */     

#if defined(__USE_UNIX98) || defined(LINUX) || defined(SOLARIS) || defined(MACOSX)

#if !defined(__GLIBC__)
int getpt(void)
{
  return posix_openpt(O_RDWR | O_NOCTTY);
  //return open("/dev/ptmx",O_RDWR | O_NOCTTY);
}
#endif

static int
get_system_fifo(char local_name[MAX_FILENAME], char remote_name[MAX_FILENAME])
{
  int   fd        = -1;

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

/* ************************************************** */     
#else
/* ************************************************** */     

#warning "get_system_fifo is not implemented"

static int
get_system_fifo(char local_name[MAX_FILENAME], char remote_name[MAX_FILENAME])
{
  strcpy(local_name,"");
  strcpy(remote_name,"");
  return -1;
}

#endif

/* ************************************************** */     
/* ************************************************** */     
/* ************************************************** */     

static struct fifo_t*
fifo_open   (const char *filename)
{
  int fd;
  struct fifo_t *fifo = NULL;
  if ((fifo = malloc(sizeof(struct fifo_t))) == NULL)
    {
      ERROR("FIFO: malloc error [%s]",strerror(errno));
      return NULL;
    }
  
  memset(fifo,0,sizeof(struct fifo_t));

  if ((fd = open(filename,O_RDWR)) == -1)
    {
      ERROR("FIFO: open error [%s]\n",strerror(errno));
      free(fifo);
      return NULL;
    }
  
#if defined(I_PUSH) && !defined(linux)
 if (isastream(fd))
   {
     if (ioctl(fd, I_PUSH, "ptem") == -1) /* push ptem */
       {
	 perror("ptem");
       }
     if (ioctl(fd, I_PUSH, "ldterm") == -1) /* push ldterm*/
       {
	 perror("ldterm");
       }
   }
#endif

  fifo->fd_in  = fd;
  fifo->fd_out = fd; 
  strncpy(fifo->local_name,filename,MAX_FILENAME);
  strncpy(fifo->remote_name,"unknown",MAX_FILENAME);
  return fifo;
}

/* ************************************************** */     
/* ************************************************** */     
/* ************************************************** */     

static struct fifo_t*
fifo_create()
{
  int fd;
  struct fifo_t *fifo = NULL;
  if ((fifo = malloc(sizeof(struct fifo_t))) == NULL)
    {
      ERROR("FIFO: malloc error [%s]\n",strerror(errno));
      return NULL;
    }

  memset(fifo,0,sizeof(struct fifo_t));

  if ((fd = get_system_fifo(fifo->local_name,fifo->remote_name)) == -1)
    {
      ERROR("FIFO: cannot create system fifo\n");
      free(fifo);
      return NULL;
    }

  fifo->fd_in  = fd;
  fifo->fd_out = fd;
  return fifo;
}

/* ************************************************** */     
/* ************************************************** */     
/* ************************************************** */     

void           
fifo_delete (struct fifo_t *fifo)
{
  if (fifo)
    {
      if (fifo->fd_in != -1)
	close(fifo->fd_in);

      if (fifo->fd_out != -1)
	close(fifo->fd_out);

      free(fifo);
    }
}

/* ************************************************** */     
/* ************************************************** */     
/* ************************************************** */     

void
fifo_putchar(struct fifo_t *fifo, char c)
{
  if (fifo && (fifo->fd_out != -1))
    {
      /* TODO: non blocking ? */ 
      DBG_MSG("FIFO: write to dev [0x%02x]\n",c);
      if (write(fifo->fd_out, &c, 1) == -1)
	{
	  DBG_MSG("FIFO: write error %d [%s]\n",errno,strerror(errno));
	}
      /* cannot use fsync and fdatasync */
    }
}

/* ************************************************** */     
/* ************************************************** */     
/* ************************************************** */     

#define MAX(a,b) (a<b ? b:a)

#define ADD_FD_SET(n,fd,set)  \
  if (fd > 0)                 \
    {                         \
      FD_SET(fd,set);         \
      n = MAX(n,fd);          \
    }

/* ************************************************** */     
/* ************************************************** */     
/* ************************************************** */     

static inline int fifo_input_ready(struct fifo_t *fifo)
{
  return fifo->input_state != 0;
}

static inline unsigned char fifo_input_read(struct fifo_t *fifo)
{
  if (fifo->input_state == 0)
    {
      ERROR("FIFO: input fifo IO input underrun\n");
      return 0;
    }
  else
    {
      char v = fifo->input_val[fifo->input_read_ptr];
      fifo->input_state    =  fifo->input_state    - 1;
      fifo->input_read_ptr = (fifo->input_read_ptr + 1) % INPUT_FIFO_SIZE;
      DBG_MSG("FIFO: input fifo input read 0x%02x\n",v & 0xffu);
      return v;
    }
}

static inline void fifo_input_write(struct fifo_t *fifo, unsigned char val)
{
  if (fifo->input_state == INPUT_FIFO_SIZE)
    {
      ERROR("FIFO: input fifo serial IO input overrun\n");
    }
  else
    {
      fifo->input_val[fifo->input_write_ptr] = val;
      fifo->input_state     =  fifo->input_state     + 1;
      fifo->input_write_ptr = (fifo->input_write_ptr + 1) % INPUT_FIFO_SIZE;
      DBG_MSG("FIFO: input fifo input write 0x%02x\n",val & 0xffu);
    }
}

/* ************************************************** */     
/* ************************************************** */     
/* ************************************************** */     

#define READ_BUF_SIZE 512

int
fifo_event(struct fifo_t *fifo, int blocking)
{
  int i;
  int c = FIFO_NOEVENT;
  int n = 0;
  int retval;
  fd_set rdset;
  struct timeval timeout;
  char read_buffer[READ_BUF_SIZE];

#if defined(FIFO_EVENT_SKIP)
  static int fifo_event_skip = FIFO_EVENT_SKIP;
#endif

  if ((fifo == NULL) || (fifo->fd_in == -1)) 
    {
      return FIFO_QUIT;
    }

  if (fifo_input_ready(fifo))
    {
      c = fifo_input_read(fifo);
    }
  else
    {
#if defined(FIFO_EVENT_SKIP)
      if ((fifo_event_skip--) == 0)
	{
	  fifo_event_skip = FIFO_EVENT_SKIP;
#endif
	  FD_ZERO(&rdset);
	  ADD_FD_SET(n,fifo->fd_in,&rdset);


	  if (blocking == FIFO_BLOCKING)
	    {
	      timeout.tv_sec  = 0;
	      timeout.tv_usec = 500000; /* 0.5 second */
	      retval = select(n + 1, &rdset, NULL, NULL, &timeout);
	    }
	  else
	    {
	      timeout.tv_sec  = 0;
	      timeout.tv_usec = 0; /* non blocking */
	      retval = select(n + 1, &rdset, NULL, NULL, &timeout);
	    }


	  switch (retval)
	    {
	    case -1:
	      ERROR("FIFO: select error [%s]\n",strerror(errno));
	      return FIFO_SELECT_ERROR;
	    case 0: /* time out for non blocking, error on blocking */
	      return FIFO_NOEVENT;
	    default:
	      if (FD_ISSET(fifo->fd_in,&rdset))
		{
		  switch (retval = read(fifo->fd_in,read_buffer,READ_BUF_SIZE))
		    {
		    case -1:
		      ERROR("FIFO: read error in fifo_event [%s]\n",strerror(errno));
		      c = FIFO_IO_ERROR;
		      break;
		    case 0: /* EOF */
		      ERROR("FIFO: read end of stream in fifo_event : quit\n");
		      c = FIFO_QUIT;
		      break;
		    default:
		      for(i=0; i < retval; i++)
			{
			  DBG_MSG("FIFO: read from dev [0x%02x]\n",read_buffer[i]);
			  fifo_input_write(fifo,read_buffer[i]);
			}
		      if (fifo_input_ready(fifo))
			{
			  c = fifo_input_read(fifo);
			}
		      else
			{
			  ERROR("FIFO: read returned ok but nothing to read ?\n");
			}
		      break;
		    }
		}
	      else
		{
		  ERROR("FIFO: select released on another fd\n");
		}
	      break;
	    }
#if defined(FIFO_EVENT_SKIP)
	} /* skip event */
#endif
    } /* else fifo_input_ready */

  return c;
}

/* ************************************************** */     
/* ************************************************** */     
/* ************************************************** */     

static int
fifo_raw(int fd, struct termios *termios_backup)
{      
    struct termios  buf;

    if (tcgetattr(fd, termios_backup) < 0) /* get the original state */
        return -1;

    memcpy(&buf,termios_backup,sizeof(struct termios));
    
    /*
      echo off, 
      canonical mode off, 
      extended input processing off, 
    */
    //    buf.c_lflag &= ~(ECHO | ICANON | IEXTEN);
    buf.c_lflag &= ~(ECHO | ICANON | IEXTEN);

    /* no SIGINT on BREAK, CR-toNL off, input parity
       check off, don't strip the 8th bit on input,
       ouput flow control off */
    // buf.c_iflag &= ~(BRKINT | ICRNL | ISTRIP | IXON);

    /* clear size bits, parity checking off */
    buf.c_cflag &= ~(CSIZE | PARENB);

    /* set 8 bits/char */
    buf.c_cflag |= CS8;

    /* output processing off */
    buf.c_oflag &= ~(OPOST);

    buf.c_cc[VMIN] = 1;  /* 1 byte at a time */
    buf.c_cc[VTIME] = 0; /* no timer on input */

    if (tcsetattr(fd, TCSAFLUSH, &buf) < 0)
        return -1;

    return 0;
}

/* ************************************************** */     
/* ************************************************** */     
/* ************************************************** */     

struct fifo_t*
fifo_init(const char* filename)
{  
  struct fifo_t *fifo = NULL;

  if (filename)
    {
      fifo = fifo_open(filename);
    }
  else
    {
      fifo = fifo_create();
    }

  if (fifo != NULL)
    {
      if (fifo_raw(fifo->fd_in,&(fifo->in_termios)) < 0)
	{
	  fprintf(stderr,"Cannot set device_in raw mode\n");
	}
      
      if (fifo_raw(fifo->fd_out,&(fifo->out_termios)) < 0)
	{
	  fprintf(stderr,"Cannot set device_out raw mode\n");
	}
    }

  return fifo;
}

/* ************************************************** */     
/* ************************************************** */     
/* ************************************************** */     

void fifo_post_quit(struct fifo_t *fifo)
{
  if (fifo)
    {
      if (fifo->fd_in != -1)
	{
	  close(fifo->fd_in);
	  fifo->fd_in = -1;
	}
    }  
}

/* ************************************************** */     
/* ************************************************** */     
/* ************************************************** */     
