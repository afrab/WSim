
/**
 *  \file   ptty_dev.c
 *  \brief  serial ptty IO
 *  \author Antoine Fraboulet
 *  \date   2005
 **/

#if !defined(_GNU_SOURCE)
#define _GNU_SOURCE 
#endif

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "arch/common/hardware.h"
#include "libselect/libselect.h"
#include "ptty_dev.h"

/***************************************************/
/***************************************************/
/***************************************************/

// #define DEBUG_PTTY

#ifdef DEBUG_PTTY
#    define HW_DMSG_PTTY(x...) HW_DMSG(x)
#else
#    define HW_DMSG_PTTY(x...) do { } while (0)
#endif

/***************************************************/
/***************************************************/
/***************************************************/

#define MAX_FILENAME 256

struct ptty_t {
  int   dev_id;    /* simulator device number */
  int   id;        /* platform serial port number (Uart0, Uart1, ...) */

  int   fd_in;
  int   fd_out;
  int   need_close;

  char  fifo_local[MAX_FILENAME];
  char  fifo_remote[MAX_FILENAME];
};

#define PTTY_DATA     ((struct ptty_t*)(machine.device[dev].data))
#define PTTY_ID       PTTY_DATA->id
#define PTTY_DEVID    PTTY_DATA->dev_id
#define PTTY_FD_IN    PTTY_DATA->fd_in
#define PTTY_FD_OUT   PTTY_DATA->fd_out

#define PTTY_IN_FIFO   10

int  ptty_reset       (int dev);
int  ptty_delete      (int dev);
int  ptty_power_up    (int dev);
int  ptty_power_down  (int dev);

void ptty_read        (int dev, uint32_t *mask, uint32_t *value);
void ptty_write       (int dev, uint32_t  mask, uint32_t  value);

void ptty_dummy_read  (int dev, uint32_t *mask, uint32_t *value);
void ptty_dummy_write (int dev, uint32_t  mask, uint32_t  value);

int  ptty_update      (int dev);
int  ptty_ui_draw     (int dev);
void ptty_ui_get_size (int dev, int *w, int *h);
void ptty_ui_set_pos  (int dev, int  x, int  y);
void ptty_ui_get_pos  (int dev, int *x, int *y);

/***************************************************/
/***************************************************/
/***************************************************/

int ptty_device_size()
{
  return sizeof(struct ptty_t);
}

/***************************************************/
/***************************************************/
/***************************************************/

void ptty_libselect_callback(int fd, uint64_t flags, void *ptr)
{
  ERROR("ptty: libselect callback has been called with flags %"PRIx64"\n",flags);
  if (flags & LIBSELECT_CLOSE)
    {
      ERROR("ptty: input has been close during simulation\n");
      ERROR("ptty: stopping serial I/O simulation now\n");
      libselect_unregister(fd);
      machine.device[((struct ptty_t*)ptr)->dev_id].read  = ptty_dummy_read;
      machine.device[((struct ptty_t*)ptr)->dev_id].write = ptty_dummy_write;
    }
}

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

static int
ptty_get_system_fifo(char local_name[MAX_FILENAME], char remote_name[MAX_FILENAME])
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

static int
ptty_get_system_fifo(char local_name[MAX_FILENAME], char remote_name[MAX_FILENAME])
{
  strcpy(local_name,"");
  strcpy(remote_name,"");
  return -1;
}

#endif

/***************************************************/
/***************************************************/
/***************************************************/

int ptty_device_create(int dev, int id, const char *filename)
{

  PTTY_FD_IN            = -1;
  PTTY_FD_OUT           = -1;
  PTTY_DATA->need_close =  0;

  if (filename == NULL)
    {
      OUTPUT("PTTY%d: ptty not activated\n",id);
      machine.device[dev].read          = ptty_dummy_read;
      machine.device[dev].write         = ptty_dummy_write;
    }
  else if (strcmp(filename,"stdout") == 0)
    {
      PTTY_FD_OUT                       = 1;
      machine.device[dev].read          = ptty_dummy_read;
      machine.device[dev].write         = ptty_write;
    }
  else if (strcmp(filename,"stdin") == 0)
    {
      PTTY_FD_IN                        = 0;
      machine.device[dev].read          = ptty_read;
      machine.device[dev].write         = ptty_dummy_write;
    }
  else if (strcmp(filename,"stdio") == 0)
    {
      PTTY_FD_IN                        = 0;
      PTTY_FD_OUT                       = 1;
      machine.device[dev].read          = ptty_read;
      machine.device[dev].write         = ptty_write;
    }
  else if (strcmp(filename,"create") == 0)
    {
      int fd;
      if ((fd = ptty_get_system_fifo(PTTY_DATA->fifo_local,PTTY_DATA->fifo_remote)) == -1)
	{
	  ERROR("PTTY%d: Cannot create system fifo\n",id);
	  OUTPUT("PTTY%d: ptty not activated\n",id);
	  machine.device[dev].read          = ptty_dummy_read;
	  machine.device[dev].write         = ptty_dummy_write;
	}
      else
	{
	  REAL_STDOUT("PTTY%d: local  system fifo %s\n",id,PTTY_DATA->fifo_local);
	  REAL_STDOUT("PTTY%d: remote system fifo %s\n",id,PTTY_DATA->fifo_remote);
	  PTTY_FD_IN                        = fd;
	  PTTY_FD_OUT                       = fd;
	  PTTY_DATA->need_close             = 1;
	  machine.device[dev].read          = ptty_read;
	  machine.device[dev].write         = ptty_write;
	}
    }
  else
    {
      int fd;
      if ((fd = open(filename,O_RDWR)) == -1)
	{
	  ERROR("PTTY%d: Cannot open file %s\n",id,filename);
	  return 1;
	}
      PTTY_FD_IN                        = fd;
      PTTY_FD_OUT                       = fd;
      PTTY_DATA->need_close             = 1;
      machine.device[dev].read          = ptty_read;
      machine.device[dev].write         = ptty_write;
    }


  if (machine.device[dev].read != ptty_dummy_read)
    {
      if (libselect_register_fifo(PTTY_FD_IN, PTTY_IN_FIFO))
	{
	  ERROR("PTTY%d: cannot register input handler %d in libselect\n",PTTY_FD_IN);
	  return 1;
	}
      libselect_add_callback(PTTY_FD_IN, ptty_libselect_callback, machine.device[dev].data);
    }

  PTTY_ID    = id;
  PTTY_DEVID = dev;

  machine.device[dev].reset         = ptty_reset;
  machine.device[dev].delete        = ptty_delete;
  machine.device[dev].power_up      = ptty_power_up;
  machine.device[dev].power_down    = ptty_power_down;

  machine.device[dev].update        = ptty_update;

  machine.device[dev].ui_draw       = ptty_ui_draw;
  machine.device[dev].ui_get_size   = ptty_ui_get_size;
  machine.device[dev].ui_set_pos    = ptty_ui_set_pos;
  machine.device[dev].ui_get_pos    = ptty_ui_get_pos;

  machine.device[dev].state_size    = ptty_device_size();
  machine.device[dev].name          = "ptty serial I/O";

  return 0;
}

/***************************************************/
/***************************************************/
/***************************************************/

int ptty_reset(int UNUSED dev)
{
  return 0;
}

/***************************************************/
/***************************************************/
/***************************************************/

int ptty_delete(int dev)
{
  if (PTTY_FD_IN != -1)
    {
      libselect_unregister(PTTY_FD_IN);
    }
  if (PTTY_DATA->need_close)
    {
      close(PTTY_FD_OUT);
    }
  return 0;
}

/***************************************************/
/***************************************************/
/***************************************************/

int ptty_power_up    (int UNUSED dev)
{
  return 0;
}

/***************************************************/
/***************************************************/
/***************************************************/

int ptty_power_down  (int UNUSED dev)
{
  return 0;
}

/***************************************************/
/***************************************************/
/***************************************************/

/* these functions are used when there is no fifo I/O available */

void ptty_dummy_read(int UNUSED dev, uint32_t *mask, uint32_t *value)
{
  *mask  = 0;
  *value = 0;
}

void ptty_dummy_write (int UNUSED dev, uint32_t UNUSED mask, uint32_t UNUSED value)
{
}

/***************************************************/
/***************************************************/
/***************************************************/

/** 
 * ptty_read
 * read from fifo to MCU
 */

void ptty_read(int dev, uint32_t *addr, uint32_t *val)
{
  unsigned char c;

  if (libselect_read(PTTY_FD_IN,&c,1))
    {
      *addr = 0;
      *val  = 0;
    }
  else
    {
      *addr = PTTY_D;
      *val  = c & 0xffu;
#if defined(DEBUG_PTTY)      
      if (isprint(*val))
	HW_DMSG_PTTY("PTTY%d <- read data 0x%02x [%c]\n",PTTY_ID,*val,*val);
      else
	HW_DMSG_PTTY("PTTY%d <- read data 0x%02x\n",PTTY_ID,*val);
#endif
    }
}

/***************************************************/
/***************************************************/
/***************************************************/

/**
 * ptty_write
 * write from MCU to serial line
 */

void ptty_write(int dev, uint32_t addr, uint32_t val)
{
  uint8_t v;
  if ((addr & PTTY_D) == PTTY_D)
    {
      v = val & PTTY_D;
#if defined(DEBUG_PTTY)      
      if (isprint(v))
	HW_DMSG_PTTY("PTTY%d -> write data 0x%02x [%c]\n",PTTY_ID,v,v);
      else
	HW_DMSG_PTTY("PTTY%d -> write data 0x%02x\n",PTTY_ID,v);
#endif
      write(PTTY_FD_OUT,&v,1);
    }
}

/***************************************************/
/***************************************************/
/***************************************************/

int  ptty_update      (int UNUSED dev)      { return 0;       }

/***************************************************/
/***************************************************/
/***************************************************/

int  ptty_ui_draw     (int UNUSED dev)
{ 
  return 0;       
}

void ptty_ui_get_size (int UNUSED dev, int *w, int *h) 
{ 
  *w = 0; *h = 0; 
}

void ptty_ui_set_pos  (int UNUSED dev, int UNUSED x, int UNUSED y) 
{
}

void ptty_ui_get_pos  (int UNUSED dev, int *x, int *y) 
{ 
  *x = 0; *y = 0; 
}

/***************************************************/
/***************************************************/
/***************************************************/
