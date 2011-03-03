
/**
 *  \file   ptty_dev.c
 *  \brief  serial ptty IO
 *  \author Antoine Fraboulet
 *  \date   2005
 **/

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#include "arch/common/hardware.h"
#include "libselect/libselect.h"
#include "src/options.h"
#include "ptty_dev.h"

/***************************************************/
/***************************************************/
/***************************************************/

#define DEBUG_PTTY

#ifdef DEBUG_PTTY
#    define HW_DMSG_PTTY(x...) HW_DMSG_DEV(x)
#else
#    define HW_DMSG_PTTY(x...) do { } while (0)
#endif

/***************************************************/
/***************************************************/
/***************************************************/

struct ptty_t {
  int            dev_id;    /* simulator device number */
  int            ser_id;    /* platform serial port number (Uart0, Uart1, ...) */
  libselect_id_t io;
};

#define PTTY_DATA     ((struct ptty_t*)(machine.device[dev].data))
#define PTTY_SERID    PTTY_DATA->ser_id
#define PTTY_DEVID    PTTY_DATA->dev_id
#define PTTY_IO       PTTY_DATA->io

#define PTTY_IN_FIFO_SIZE 5

int  ptty_reset       (int dev);
int  ptty_delete      (int dev);
int  ptty_power_up    (int dev);
int  ptty_power_down  (int dev);

void ptty_read        (int dev, uint32_t *mask, uint32_t *value);
void ptty_write       (int dev, uint32_t  mask, uint32_t  value);
void ptty_dummy_read  (int dev, uint32_t *mask, uint32_t *value);
void ptty_dummy_write (int dev, uint32_t  mask, uint32_t  value);
void ptty_skt_read    (int dev, uint32_t *mask, uint32_t *value);
void ptty_skt_write   (int dev, uint32_t  mask, uint32_t  value);

int  ptty_update      (int dev);
int  ptty_ui_draw     (int dev);
void ptty_ui_get_size (int dev, int *w, int *h);
void ptty_ui_set_pos  (int dev, int  x, int  y);
void ptty_ui_get_pos  (int dev, int *x, int *y);
void ptty_statdump    (int dev, int64_t UNUSED user_nanotime);

/***************************************************/
/***************************************************/
/***************************************************/

#define WSIM_MAX_PTTYS 4
#define MAXNAME        40

struct ptty_opt_t {
  int  dev_num;
  int  dev_id;
  char dev_name[MAXNAME];
  struct moption_t io;
};

static struct ptty_opt_t opt_array[WSIM_MAX_PTTYS];

char* str_build(const char* str, const char* name)
{
  char buff[MAXNAME];
  snprintf(buff,MAXNAME,str,name);
  return strdup(buff); /* create strings */
}

int ptty_add_options(int dev_num, int dev_id, const char *dev_name)
{
  if (dev_id >= WSIM_MAX_PTTYS)
    {
      ERROR("ptty: too much devices, please rewrite option handling\n");
      return -1;
    }

  opt_array[dev_id].dev_num = dev_num;
  opt_array[dev_id].dev_id  = dev_id;
  strncpyz(opt_array[dev_id].dev_name, dev_name, MAXNAME);

  opt_array[dev_id].io.longname    = str_build("%s_io", dev_name);
  opt_array[dev_id].io.type        = required_argument;
  opt_array[dev_id].io.helpstring  = str_build("%s in/out", dev_name);
  opt_array[dev_id].io.value       = NULL;

  options_add( & opt_array[dev_id].io);
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

void ptty_libselect_callback(int UNUSED fd, uint64_t flags, void *ptr)
{
  ERROR("ptty: libselect callback has been called with flags %"PRIx64"\n",flags);
  if (flags & LIBSELECT_EVT_CLOSE)
    {
      ERROR("ptty: input has been close during simulation\n");
      ERROR("ptty: stopping serial I/O simulation now\n");
      machine.device[((struct ptty_t*)ptr)->dev_id].read  = ptty_dummy_read;
      machine.device[((struct ptty_t*)ptr)->dev_id].write = ptty_dummy_write;
    }
}

/***************************************************/
/***************************************************/
/***************************************************/

int ptty_device_create(int dev, int id)
{

  PTTY_SERID = id;
  PTTY_DEVID = dev;

  machine.device[dev].read          = ptty_dummy_read;
  machine.device[dev].write         = ptty_dummy_write;
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
  machine.device[dev].statdump      = ptty_statdump;

  if (opt_array[id].io.value != NULL)
    {
      PTTY_IO = libselect_id_create(opt_array[id].io.value, 0);
      libselect_id_register(PTTY_IO);
    }

  if (libselect_id_is_valid(PTTY_IO))
    {
      machine.device[dev].read          = ptty_read;
      machine.device[dev].write         = ptty_write;
    }

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

void ptty_statdump(int dev, int64_t UNUSED user_nanotime)
{
  if (opt_array[PTTY_SERID].io.value)
    {
      OUTPUT("     + opt: %s\n",  opt_array[PTTY_SERID].io.value);
    }
}

/***************************************************/
/***************************************************/
/***************************************************/

int ptty_delete(int dev)
{
  libselect_id_close(PTTY_IO);

  if (opt_array[PTTY_SERID].io.longname)
    free(opt_array[PTTY_SERID].io.longname);

  if (opt_array[PTTY_SERID].io.helpstring)
    free(opt_array[PTTY_SERID].io.helpstring);

  return 0;
}

/***************************************************/
/***************************************************/
/***************************************************/

int ptty_power_up    (int UNUSED dev)
{
  return 0;
}

int ptty_power_down  (int UNUSED dev)
{
  return 0;
}

/***************************************************/
/***************************************************/
/***************************************************/

/** 
 * ptty_read
 * read from fifo to MCU
 */

static void DMSG_PRINT_VAL(int dev , unsigned char val, char *str)
{
  char buff[1024];
  snprintf(buff,1024,"PTTY%d: MCU data 0x%02x [%c] %s\n",
	   PTTY_SERID, val, isprint(val) ? val : '.', str);
  HW_DMSG_PTTY(buff);
}

void ptty_dummy_read (int UNUSED dev, uint32_t *mask, uint32_t *value)  
{ 
  *mask  = 0; 
  *value = 0; 
}

void ptty_dummy_write(int dev, uint32_t  mask, uint32_t  value)  
{  
  mask  = 0;  
  value = 0; 
  uint8_t v;
  if ((mask & PTTY_D) == PTTY_D)
    {
      v = value & PTTY_D;
      DMSG_PRINT_VAL(dev,v,"to /dev/null");
    }
}


void ptty_read(int dev, uint32_t *addr, uint32_t *val)
{
  unsigned char c;
  *addr = 0;
  *val  = 0;

  if (libselect_id_read(PTTY_IO,&c,1) == 1)
    {
      *addr = PTTY_D;
      *val  = c & 0xffu;
      DMSG_PRINT_VAL(dev,c,"read from serial");
    }
}

void ptty_write(int dev, uint32_t addr, uint32_t val)
{
  uint8_t v;
  if ((addr & PTTY_D) == PTTY_D)
    {
      v = val & PTTY_D;
      libselect_id_write(PTTY_IO,&v,1);
      DMSG_PRINT_VAL(dev,v,"write to serial");
    }
}

/***************************************************/
/***************************************************/
/***************************************************/

int  ptty_update      (int UNUSED dev)      
{ 
  return 0;       
}

int  ptty_ui_draw     (int UNUSED dev)
{ 
  return 0;       
}

void ptty_ui_get_size (int UNUSED dev, int *w, int *h) 
{ 
  *w = 0; 
  *h = 0; 
}

void ptty_ui_set_pos  (int UNUSED dev, int UNUSED x, int UNUSED y) 
{
}

void ptty_ui_get_pos  (int UNUSED dev, int *x, int *y) 
{ 
  *x = 0; 
  *y = 0; 
}

/***************************************************/
/***************************************************/
/***************************************************/
