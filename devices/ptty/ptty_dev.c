
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
#include "src/options.h"
#include "ptty_dev.h"

/***************************************************/
/***************************************************/
/***************************************************/

#define DEBUG_PTTY

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
  int   need_close_in;
  int   need_close_out;

  char  fifo_in_local  [MAX_FILENAME];
  char  fifo_in_remote [MAX_FILENAME];
  char  fifo_out_local [MAX_FILENAME];
  char  fifo_out_remote[MAX_FILENAME];
};

#define PTTY_DATA     ((struct ptty_t*)(machine.device[dev].data))
#define PTTY_ID       PTTY_DATA->id
#define PTTY_DEVID    PTTY_DATA->dev_id
#define PTTY_FD_IN    PTTY_DATA->fd_in
#define PTTY_FD_OUT   PTTY_DATA->fd_out

#define PTTY_IN_FIFO_SIZE 512

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

#define MAXNAME 40

struct ptty_opt_t {
  int  dev_num;
  int  dev_id;
  char dev_name[MAXNAME];
  struct moption_t io;
  struct moption_t in; 
  struct moption_t out;
};

static struct ptty_opt_t opt_array[2];

char* str_build(const char* str, const char* name)
{
  char buff[MAXNAME];
  snprintf(buff,MAXNAME,str,name);
  return strdup(buff);
}

int ptty_add_options(int dev_num, int dev_id, const char *dev_name)
{
  if (dev_id >= 2)
    {
      ERROR("ptty: too much devices, please rewrite option handling\n");
      return -1;
    }

  opt_array[dev_id].dev_num = dev_num;
  opt_array[dev_id].dev_id  = dev_id;
  strncpy(opt_array[dev_id].dev_name, dev_name, MAXNAME);

  opt_array[dev_id].io.longname    = str_build("%s_io", dev_name);
  opt_array[dev_id].io.type        = required_argument;
  opt_array[dev_id].io.helpstring  = str_build("%s fifo in/out", dev_name);
  opt_array[dev_id].io.value       = NULL;

  opt_array[dev_id].in.longname    = str_build("%s_in", dev_name);
  opt_array[dev_id].in.type        = required_argument;
  opt_array[dev_id].in.helpstring  = str_build("%s fifo in", dev_name);
  opt_array[dev_id].in.value       = NULL;

  opt_array[dev_id].out.longname   = str_build("%s_out", dev_name);
  opt_array[dev_id].out.type       = required_argument;
  opt_array[dev_id].out.helpstring = str_build("%s fifo out", dev_name);
  opt_array[dev_id].out.value      = NULL;

  options_add( & opt_array[dev_id].io);
  options_add( & opt_array[dev_id].in);
  options_add( & opt_array[dev_id].out);
  return 0;
}

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

int ptty_device_create(int dev, int id)
{
  char* file_in;
  char* file_out;

  PTTY_FD_IN                 = -1;
  PTTY_FD_OUT                = -1;
  PTTY_DATA->need_close_in   =  0;
  PTTY_DATA->need_close_out  =  0;
  machine.device[dev].read   =  ptty_dummy_read;
  machine.device[dev].write  =  ptty_dummy_write;

  REAL_STDOUT("ptty:options: %s %s %s\n",
	      opt_array[id].io.value,
	      opt_array[id].in.value,
	      opt_array[id].out.value);

  if (opt_array[id].io.value && (opt_array[id].in.value || opt_array[id].out.value))
    {
      ERROR("ptty:options: must define unique io or or separated in/out devices\n");
      return 1;
    }

  if (opt_array[id].io.value != NULL)
    {
      file_in  = opt_array[id].io.value;
      file_out = opt_array[id].io.value;
      
      if      (strcmp(file_in,"stdio") == 0)
	{
	  PTTY_FD_IN                        = 0;
	  PTTY_FD_OUT                       = 1;
	  machine.device[dev].read          = ptty_read;
	  machine.device[dev].write         = ptty_write;
	}
      else if (strcmp(file_in,"stdout") == 0)
	{
	  PTTY_FD_OUT                       = 1;
	  machine.device[dev].write         = ptty_write;
	}
      else if (strcmp(file_in,"stdin") == 0)
	{
	  PTTY_FD_OUT                       = 0;
	  machine.device[dev].read          = ptty_read;
	}
      else if (strcmp(file_in,"create") == 0)
	{
	  int fd;
	  if ((fd = ptty_get_system_fifo(PTTY_DATA->fifo_in_local,PTTY_DATA->fifo_in_remote)) == -1)
	    {
	      ERROR("PTTY%d: Cannot create system fifo\n",id);
	      OUTPUT("PTTY%d: ptty not activated\n",id);
	    }
	  else
	    {
	      REAL_STDOUT("PTTY%d: local  system fifo %s\n",id,PTTY_DATA->fifo_in_local);
	      REAL_STDOUT("PTTY%d: remote system fifo %s\n",id,PTTY_DATA->fifo_in_remote);
	      PTTY_FD_IN                        = fd;
	      PTTY_FD_OUT                       = fd;
	      PTTY_DATA->need_close_in          = 1;
	      machine.device[dev].read          = ptty_read;
	      machine.device[dev].write         = ptty_write;
	    }
	}
      else /* fifo name */
	{
	  int fd;
	  if ((fd = open(file_in,O_RDWR)) == -1)
	    {
	      ERROR("PTTY%d: Cannot open file %s\n",id,file_in);
	      return 1;
	    }
	  PTTY_FD_IN                        = fd;
	  PTTY_FD_OUT                       = fd;
	  PTTY_DATA->need_close_out         = 1;
	  strncpy(PTTY_DATA->fifo_in_local,file_in,MAX_FILENAME);
	  machine.device[dev].read          = ptty_read;
	  machine.device[dev].write         = ptty_write;
	}
    }
  else /* separate input/output devices */
    {
      file_in  = opt_array[id].in.value;
      file_out = opt_array[id].out.value;

      /* output */
      if (file_out == NULL )
	{
	  OUTPUT("PTTY%d: ptty OUT not activated\n",id);
	}
      else if (strcmp(file_out,"stdout") == 0)
	{
	  PTTY_FD_OUT                       = 1;
	  machine.device[dev].write         = ptty_write;
	}
      else if (strcmp(file_out,"create") == 0)
	{
	  int fd;
	  if ((fd = ptty_get_system_fifo(PTTY_DATA->fifo_out_local,PTTY_DATA->fifo_out_remote)) == -1)
	    {
	      ERROR("PTTY%d: Cannot create system fifo for output\n",id);
	    }
	  else
	    {
	      REAL_STDOUT("PTTY%d: remote system fifo %s output\n",id,PTTY_DATA->fifo_out_remote);
	      PTTY_FD_OUT                       = fd;
	      PTTY_DATA->need_close_out         = 1;
	      machine.device[dev].write         = ptty_write;
	    }
	}
      else
	{
	  int fd;
	  if ((fd = open(file_out,O_WRONLY)) == -1)
	    {
	      ERROR("PTTY%d:output: Cannot open file %s\n",id,file_out);
	      return 1;
	    }
	  REAL_STDOUT("PTTY%d:open:output %15s = %d\n",id,file_out,fd);
	  PTTY_FD_OUT                       = fd;
	  PTTY_DATA->need_close_out         = 1;
	  strncpy(PTTY_DATA->fifo_out_local,file_out,MAX_FILENAME);
	  machine.device[dev].write         = ptty_write;
	}

      /* input */
      if (file_in == NULL )
	{
	  OUTPUT("PTTY%d: ptty IN not activated\n",id);
	}
      else if (strcmp(file_in,"stdin") == 0)
	{
	  PTTY_FD_IN                        = 0;
	  machine.device[dev].read          = ptty_read;
	}
      else if (strcmp(file_in,"create") == 0)
	{
	  int fd;
	  if ((fd = ptty_get_system_fifo(PTTY_DATA->fifo_in_local,PTTY_DATA->fifo_in_remote)) == -1)
	    {
	      ERROR("PTTY%d: Cannot create system fifo for input\n",id);
	    }
	  else
	    {
	      REAL_STDOUT("PTTY%d: remote system fifo %s input\n",id,PTTY_DATA->fifo_in_remote);
	      PTTY_FD_IN                        = fd;
	      PTTY_DATA->need_close_in          = 1;
	      machine.device[dev].read          = ptty_read;
	    }
	}
      else
	{
	  int fd;
	  if ((fd = open(file_in,O_RDONLY)) == -1)
	    {
	      ERROR("PTTY%d:input: Cannot open file %s\n",id,file_in);
	      return 1;
	    }
	  REAL_STDOUT("PTTY%d:open:input  %15s = %d\n",id,file_in,fd);
	  PTTY_FD_IN                        = fd;
	  PTTY_DATA->need_close_in          = 1;
	  strncpy(PTTY_DATA->fifo_in_local,file_in,MAX_FILENAME);
	  machine.device[dev].read          = ptty_read;
	}

      
    }
  

  /* ******* */

  if (machine.device[dev].read != ptty_dummy_read)
    {
      if (libselect_register_fifo(PTTY_FD_IN, PTTY_IN_FIFO_SIZE))
	{
	  ERROR("PTTY%d: cannot register input handler %d in libselect\n",PTTY_FD_IN);
	  return 1;
	}
      libselect_add_callback(PTTY_FD_IN, ptty_libselect_callback, machine.device[dev].data);
    }

  /* ******* */

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
  if ((PTTY_FD_IN != -1) && (PTTY_DATA->need_close_in))
    {
      close(PTTY_FD_IN);
    }
  if ((PTTY_FD_OUT != -1) && (PTTY_DATA->need_close_out))
    {
      close(PTTY_FD_OUT);
    }

  /* add: free memory used for options */
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
      {
	char buff[1024];
	if (isprint(*val))
	  snprintf(buff,1024,"PTTY%d <- MCU read  data 0x%02x [%c] from OUTSIDE\n",PTTY_ID,*val,*val);
	else
	  snprintf(buff,1024,"PTTY%d <- MCU read  data 0x%02x      from OUTSIDE\n",PTTY_ID,*val);
	HW_DMSG_PTTY(buff);
	REAL_STDOUT(buff);
      }
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
      {
	char buff[1024];
	if (isprint(v))
	  snprintf(buff,1024,"PTTY%d -> MCU write data 0x%02x [%c]  to  OUTSIDE\n",PTTY_ID,v,v);
	else
	  snprintf(buff,1024,"PTTY%d -> MCU write data 0x%02x       to  OUTSIDE\n",PTTY_ID,v);
	HW_DMSG_PTTY(buff);
	REAL_STDOUT(buff);
      }
#endif
      write(PTTY_FD_OUT,&v,1);
    }
}

/***************************************************/
/***************************************************/
/***************************************************/

int  ptty_update      (int UNUSED dev)      
{ 
  return 0;       
}

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
