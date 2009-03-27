
/**
 *  \file   spidev_dev.c
 *  \brief  SPI device example
 *  \author Antoine Fraboulet
 *  \date   2009
 **/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "arch/common/hardware.h"
#include "devices/spidev/spidev_dev.h"
#include "src/options.h"

/***************************************************/
/***************************************************/

#define NAME      "spidev"

tracer_id_t TRACER_SPIDEV_STATE;
tracer_id_t TRACER_SPIDEV_STROBE;

/***************************************************/
/***************************************************/

#ifdef DEBUG
#define MSG_DEVICES       2
#define DEBUG_ME_HARDER
#define HW_DMSG_SPI(x...) VERBOSE(MSG_DEVICES,x)
#else
#define HW_DMSG_SPI(x...) do {} while(0)
#endif

/***************************************************/
/***************************************************/
/***************************************************/

struct spidev_t 
{

  uint8_t select_bit;           /* chip select   */
  uint8_t hold_bit;             /* hold          */
  uint8_t write_protect_bit;    /* write protect */

  /* data just written */
  uint8_t data_buffer;
  uint8_t data_buffer_ok;

  /* data to be read */
  uint8_t data_ready;
  uint8_t data_val;
  
  /* busy timing */
  uint64_t end_of_busy_time;

  /* clock pin : unused */
  uint8_t clock;
};

#define SPIDEV_DATA        ((struct spidev_t*)(machine.device[dev].data))

/***************************************************/
/** Flash external entry points ********************/
/***************************************************/

int  spidev_reset       (int dev);
int  spidev_delete      (int dev);
int  spidev_power_up    (int dev);
int  spidev_power_down  (int dev);
void spidev_read        (int dev, uint32_t *mask, uint32_t *value);
void spidev_write       (int dev, uint32_t  mask, uint32_t  value);
int  spidev_update      (int dev);
int  spidev_ui_draw     (int dev);
void spidev_ui_get_size (int dev, int *w, int *h);
void spidev_ui_set_pos  (int dev, int  x, int  y);
void spidev_ui_get_pos  (int dev, int *x, int *y);

/***************************************************/
/***************************************************/
/***************************************************/

#define MAXNAME 1024

int spidev_add_options(int UNUSED dev_num, int dev_id, const char UNUSED *dev_name)
{
  if (dev_id >= 1)
    {
      ERROR("spidev: too much devices, please rewrite option handling\n");
      return -1;
    }

  return 0;
}

/***************************************************/
/***************************************************/
/***************************************************/

int spidev_device_size()
{
  return sizeof(struct spidev_t);
}

/***************************************************/
/***************************************************/
/***************************************************/

int spidev_device_create(int dev, int UNUSED id)
{
  machine.device[dev].reset         = spidev_reset;
  machine.device[dev].delete        = spidev_delete;
  machine.device[dev].power_up      = spidev_power_up;
  machine.device[dev].power_down    = spidev_power_down;

  machine.device[dev].read          = spidev_read;
  machine.device[dev].write         = spidev_write;
  machine.device[dev].update        = spidev_update;

  machine.device[dev].ui_draw       = spidev_ui_draw;
  machine.device[dev].ui_get_size   = spidev_ui_get_size;
  machine.device[dev].ui_set_pos    = spidev_ui_set_pos;

  machine.device[dev].state_size    = spidev_device_size();
  machine.device[dev].name          = NAME " example";

#if defined(DEBUG_ME_HARDER)
  HW_DMSG_SPI(NAME ": =================================== \n");
  HW_DMSG_SPI(NAME ": 0000 CHSW dddd dddd == MASK         \n");
  HW_DMSG_SPI(NAME ":      C              : Clock         \n");
  HW_DMSG_SPI(NAME ":       M             : MiSo          \n");
  HW_DMSG_SPI(NAME ":        S            : Chip Select   \n");
  HW_DMSG_SPI(NAME ":         W           : Write Protect \n");
  HW_DMSG_SPI(NAME ":           dddd dddd : SPI data      \n");
  HW_DMSG_SPI(NAME ": =================================== \n");
#endif

  TRACER_SPIDEV_STATE  = tracer_event_add_id(8, "state"    , NAME);
  TRACER_SPIDEV_STROBE = tracer_event_add_id(8, "function" , NAME);

  return 0;
}

/***************************************************/
/***************************************************/
/***************************************************/

int spidev_reset(int UNUSED dev)
{
  HW_DMSG_SPI(NAME ": device reset\n");
  return 0;
}

/***************************************************/
/***************************************************/
/***************************************************/

int spidev_delete(int UNUSED dev)
{
  HW_DMSG_SPI(NAME ": device delete\n");
  return 0;
}

/***************************************************/
/***************************************************/
/***************************************************/

int spidev_power_up(int UNUSED dev)
{
  HW_DMSG_SPI(NAME ": device power up\n");
  return 0;
}

int spidev_power_down(int UNUSED dev)
{
  HW_DMSG_SPI(NAME ": device power down\n");
  return 0;
}

/***************************************************/
/***************************************************/
/***************************************************/

/*
 * read is done only on a junk write
 *
 */

void spidev_read(int UNUSED dev, uint32_t *mask, uint32_t *value)
{
  *mask  = 0;
  *value = 0;
  
  if (*mask != 0)
    {
      HW_DMSG_SPI(NAME ": device write to mcu [val=0x%02x,mask=0x%04x] \n", *value, *mask);
    }
}

/***************************************************/
/***************************************************/
/***************************************************/

void spidev_write(int dev, uint32_t mask, uint32_t value)
{
  HW_DMSG_SPI(NAME ": device write from mcu value 0x%04x mask 0x%04x\n",value, mask);

  /***************************
   * Control pins. WRITE PROTECT
   ***************************/

  if (mask & SPIDEV_W) /* write protect netgated */
    {
      SPIDEV_DATA->write_protect_bit  = ! (value & SPIDEV_W);
      HW_DMSG_SPI(NAME ":    flash write protect W = %d\n",SPIDEV_DATA->write_protect_bit);
    }

  /***************************
   * Control pins. CLOCK
   ***************************/

  if (mask & SPIDEV_C) /* clock */
    {
      ERROR(NAME ":    clock pin should not be used during simulation\n");
      HW_DMSG_SPI(NAME ":    clock pin should not be used during simulation\n");
      SPIDEV_DATA->clock = (value >> SPIDEV_C_SHIFT) & 0x1;
    }
}

/***************************************************/
/***************************************************/
/***************************************************/

int spidev_update(int UNUSED dev)
{
  return 0;
}

/***************************************************/
/***************************************************/
/***************************************************/

int spidev_ui_draw      (int UNUSED dev)
{
  return 0;
}

void spidev_ui_get_size (int UNUSED dev, int *w, int *h)
{
  w = 0;
  h = 0;
}

void spidev_ui_set_pos  (int UNUSED dev, int UNUSED x, int UNUSED y)
{
  
}

void spidev_ui_get_pos  (int UNUSED dev, int *x, int *y)
{
  *x = 0;
  *y = 0;
}

/***************************************************/
/***************************************************/
/***************************************************/
