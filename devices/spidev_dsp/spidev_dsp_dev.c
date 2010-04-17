
/**
 *  \file   spidev_dsp_dev.c
 *  \brief  SPI DSP device example
 *  \author Loic Lemaitre, Antoine Fraboulet
 *  \date   2009
 **/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "arch/common/hardware.h"
#include "devices/spidev_dsp/spidev_dsp_dev.h"
#include "src/options.h"

/***************************************************/
/***************************************************/

#define DSP_MODE_SLAVE  0
#define DSP_MODE_MASTER 1

#define DSP_MASTER_BAUDRATE 1000000

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

#define DSP_MEM_SIZE 200

#define DSP_WAIT_FOR_TX 10

struct spidev_dsp_t 
{

  uint8_t select_bit;           /* chip select   */
  uint8_t write_protect_bit;    /* write protect */
  
  /*********************/
  /* SPI Slave port    */
  /*********************/

  /* data just written */
  uint32_t slave_data_w_val;
  uint32_t slave_data_w_mask;   
  uint32_t slave_data_r_val;
  uint32_t slave_data_r_nextval; /* becomes r_val on update */

  /*********************/
  /* SPI Master port   */
  /*********************/
  
  /* spi master time handling */
  uint64_t spidev_dsp_tx_start_time; /* nano second */
  uint64_t spidev_dsp_tx_lag_time;   /* nano second */
  uint8_t  spidev_dsp_tx_processing; /* boolean     */
  int      spidev_dsp_wait_for_tx;   /* number of update call to drop before sending */

  /*********************/
  /* Internals         */
  /*********************/

  /* busy timing */
  uint64_t end_of_busy_time;         /* nano second */

  /* table for test */
  uint8_t  dsp_data[DSP_MEM_SIZE];
  int32_t  dsp_index;
  int32_t  dsp_index_max;

  /* current spi mode */
  uint8_t  dsp_mode;          /* boolean     */

};


#define DSP_NAME   machine.device[dev].name
#define DSP_DATA   ((struct spidev_dsp_t*)(machine.device[dev].data))
#define DSP_MODE   DSP_DATA->dsp_mode

/***************************************************/
/***************************************************/
/***************************************************/

tracer_id_t TRACER_SPIDEV_DSP_STATE;
tracer_id_t TRACER_SPIDEV_DSP_STROBE;

/***************************************************/
/** DSP external entry points **********************/
/***************************************************/

int  spidev_dsp_reset       (int dev);
int  spidev_dsp_delete      (int dev);
int  spidev_dsp_power_up    (int dev);
int  spidev_dsp_power_down  (int dev);
void spidev_dsp_read        (int dev, uint32_t *mask, uint32_t *value);
void spidev_dsp_write       (int dev, uint32_t  mask, uint32_t  value);
int  spidev_dsp_update      (int dev);
int  spidev_dsp_ui_draw     (int dev);
void spidev_dsp_ui_get_size (int dev, int *w, int *h);
void spidev_dsp_ui_set_pos  (int dev, int  x, int  y);
void spidev_dsp_ui_get_pos  (int dev, int *x, int *y);

/***************************************************/
/***************************************************/
/***************************************************/

#define MAXNAME 1024

int spidev_dsp_add_options(int UNUSED dev_num, int dev_id, const char UNUSED *dev_name)
{
  if (dev_id >= 1)
    {
      ERROR("spidsp: too much devices, please rewrite option handling\n");
      return -1;
    }

  return 0;
}

/***************************************************/
/***************************************************/
/***************************************************/

int spidev_dsp_device_size()
{
  return sizeof(struct spidev_dsp_t);
}

/***************************************************/
/***************************************************/
/***************************************************/

#define DSP_NAME_MAX  15

int spidev_dsp_device_create(int dev, int UNUSED id, const char *dev_name)
{
  machine.device[dev].reset         = spidev_dsp_reset;
  machine.device[dev].delete        = spidev_dsp_delete;
  machine.device[dev].power_up      = spidev_dsp_power_up;
  machine.device[dev].power_down    = spidev_dsp_power_down;

  machine.device[dev].read          = spidev_dsp_read;
  machine.device[dev].write         = spidev_dsp_write;
  machine.device[dev].update        = spidev_dsp_update;

  machine.device[dev].ui_draw       = spidev_dsp_ui_draw;
  machine.device[dev].ui_get_size   = spidev_dsp_ui_get_size;
  machine.device[dev].ui_set_pos    = spidev_dsp_ui_set_pos;

  machine.device[dev].state_size    = spidev_dsp_device_size();
  machine.device[dev].name          = malloc(DSP_NAME_MAX);
  strncpy(machine.device[dev].name, dev_name, DSP_NAME_MAX);

  TRACER_SPIDEV_DSP_STATE  = tracer_event_add_id(8, "state"  , dev_name);
  TRACER_SPIDEV_DSP_STROBE = tracer_event_add_id(8, "strobe" , dev_name);

  spidev_dsp_reset(dev);
  HW_DMSG_SPI("%s: device create done\n", DSP_NAME);
  return 0;
}

/***************************************************/
/***************************************************/
/***************************************************/

int spidev_dsp_reset(int dev)
{
  HW_DMSG_SPI("%s: device reset\n", DSP_NAME);
  DSP_DATA->dsp_index                = 0;
  DSP_DATA->dsp_index_max            = 0;
  DSP_DATA->dsp_mode                 = DSP_MODE_SLAVE;
  DSP_DATA->slave_data_r_val         = 0;
  DSP_DATA->slave_data_r_nextval     = 0;

  DSP_DATA->spidev_dsp_tx_processing = 0;
  DSP_DATA->spidev_dsp_tx_lag_time   = 7 * (1000000000 / DSP_MASTER_BAUDRATE); /* in nano second */
  DSP_DATA->spidev_dsp_wait_for_tx   = DSP_WAIT_FOR_TX;
  return 0;
}

/***************************************************/
/***************************************************/
/***************************************************/

int spidev_dsp_delete(int dev)
{
  HW_DMSG_SPI("%s: device delete\n", DSP_NAME);
  free(DSP_NAME);
  return 0;
}

/***************************************************/
/***************************************************/
/***************************************************/

int spidev_dsp_power_up(int UNUSED dev)
{
  HW_DMSG_SPI("%s: device power up\n", DSP_NAME);
  return 0;
}

int spidev_dsp_power_down(int UNUSED dev)
{
  HW_DMSG_SPI("%s: device power down\n", DSP_NAME);
  return 0;
}

/***************************************************/
/***************************************************/
/***************************************************/

void spidev_dsp_read(int UNUSED dev, uint32_t *mask, uint32_t *value)
{
  *value = 0;
  *mask  = DSP_DATA->slave_data_w_mask; /* do wa follow a slave write ? */

  /***************************
   * DATA pins.
   ***************************/

  if (*mask && SPIDEV_DSP_D)
    {
      *value = DSP_DATA->slave_data_r_val;
    }

  if (*mask != 0)
    {
      HW_DMSG_SPI("%s: device write to mcu [val=0x%02x,mask=0x%04x] \n", DSP_NAME, *value, *mask);
    }
}

/***************************************************/
/***************************************************/
/***************************************************/

void spidev_dsp_write(int dev, uint32_t mask, uint32_t value)
{
  HW_DMSG_SPI("%s: device write from mcu value 0x%08x mask 0x%08x\n",DSP_NAME, value, mask);

  /***************************
   * Control pins. WRITE PROTECT
   ***************************/
  if (mask & SPIDEV_DSP_W) /* write protect negated */
    {
      DSP_DATA->write_protect_bit  = ! (value & SPIDEV_DSP_W);
      HW_DMSG_SPI("%s:    write protect W = %d\n", DSP_NAME, DSP_DATA->write_protect_bit);
    }

  /***************************
   * Control pins. CLOCK
   ***************************/
  if (mask & SPIDEV_DSP_C) /* clock */
    {
      ERROR("%s:    clock pin should not be used during simulation\n", DSP_NAME);
      HW_DMSG_SPI("%s:    clock pin should not be used during simulation\n", DSP_NAME);
      // DSP_DATA->clock = (value >> SPIDEV_DSP_C_SHIFT) & 0x1;
    }

  /***************************
   * Control pins. MODE
   ***************************/
  if (mask & SPIDEV_DSP_M)
    {
      if ((value & SPIDEV_DSP_M) == SPIDEV_DSP_M)
	{
	  DSP_DATA->dsp_mode      = DSP_MODE_MASTER;
	  DSP_DATA->dsp_index_max = DSP_DATA->dsp_index;
	  HW_DMSG_SPI("%s: spi mode master, index max = %d\n", DSP_NAME, DSP_DATA->dsp_index_max); 
	}
      else
	{
	  DSP_DATA->dsp_mode      = DSP_MODE_SLAVE;
	  DSP_DATA->dsp_index     = 0;
	  DSP_DATA->dsp_index_max = 0;
	  HW_DMSG_SPI("%s: spi mode master\n", DSP_NAME); 
	}
    }

  /***************************
   * DATA pins.
   ***************************/
  if (mask & SPIDEV_DSP_D)
    { 
      DSP_DATA->slave_data_w_val  = (uint8_t)(value & SPIDEV_DSP_D);
      DSP_DATA->slave_data_w_mask = SPIDEV_DSP_D;
    }
}

/***************************************************/
/***************************************************/
/***************************************************/

void spidev_dsp_tx_end(int UNUSED dev)
{
  /*
  DSP_DATA->data_r_val               = DSP_DATA->data_r_val_temp;
  DSP_DATA->spidev_dsp_tx_processing = 0;
  HW_DMSG_SPI("%s: Sending to mcu: value %d of the table [0x%02x], time=%lldns\n", 
	      DSP_NAME,
	      DSP_DATA->spidev_dsp_index_max - DSP_DATA->spidev_dsp_index, 
	      DSP_DATA->data_r_val, MACHINE_TIME_GET_NANO());
  DSP_DATA->spidev_dsp_wait_for_tx = DSP_WAIT_FOR_TX;
  */
}


void spidev_dsp_tx_start(int UNUSED dev)
{
  /*
  DSP_DATA->data_r_val_temp          = DSP_DATA->spidev_dsp_data[DSP_DATA->spidev_dsp_index_max - DSP_DATA->spidev_dsp_index];
  DSP_DATA->spidev_dsp_tx_start_time = MACHINE_TIME_GET_NANO();
  DSP_DATA->spidev_dsp_tx_processing = 1;

  HW_DMSG_SPI("%s: start of wait before sending to mcu: value %d of the table [0x%02x], time=%lldns\n", 
	      DSP_NAME,
	      DSP_DATA->spidev_dsp_index_max - DSP_DATA->spidev_dsp_index, 
	      DSP_DATA->data_r_val_temp, MACHINE_TIME_GET_NANO());

  DSP_DATA->spidev_dsp_index--;

  if (DSP_DATA->spidev_dsp_index < 0)
    {
      DSP_DATA->spidev_dsp_index = 0;
    }
  */
}


int spidev_dsp_update(int UNUSED dev)
{
  /* update current state */
  switch (DSP_MODE) 
    {
    case DSP_MODE_SLAVE:
      if (DSP_DATA->slave_data_w_mask == SPIDEV_DSP_D)
	{
	  DSP_DATA->dsp_data[DSP_DATA->dsp_index] = DSP_DATA->slave_data_w_val;
	  DSP_DATA->dsp_index++;

	  HW_DMSG_SPI("%s: value put in the table at index %d [0x%02x]\n", 
		      DSP_NAME, DSP_DATA->dsp_index - 1, DSP_DATA->slave_data_w_val);
	}
      break;

    case DSP_MODE_MASTER:
      if (DSP_DATA->spidev_dsp_tx_processing)
	{
	  if (DSP_DATA->spidev_dsp_tx_start_time + 
	      DSP_DATA->spidev_dsp_tx_lag_time <= MACHINE_TIME_GET_NANO())
	    {
	      spidev_dsp_tx_end(dev);	  
	    }
	  return 0;
	}
      
      if (DSP_DATA->spidev_dsp_wait_for_tx <= 0)
	{
	  spidev_dsp_tx_start(dev);
	}
      break;
      
    default:
      break;
    }

  /* prepare next data to read on SPI slave port */
  if (DSP_DATA->slave_data_w_mask)
    {
      DSP_DATA->slave_data_r_val  = DSP_DATA->slave_data_w_val;
    }
  DSP_DATA->slave_data_w_mask = 0;
 
  return 0;
}


/***************************************************/
/***************************************************/
/***************************************************/

int spidev_dsp_ui_draw      (int UNUSED dev)
{
  return 0;
}

void spidev_dsp_ui_get_size (int UNUSED dev, int *w, int *h)
{
  w = 0;
  h = 0;
}

void spidev_dsp_ui_set_pos  (int UNUSED dev, int UNUSED x, int UNUSED y)
{
  
}

void spidev_dsp_ui_get_pos  (int UNUSED dev, int *x, int *y)
{
  *x = 0;
  *y = 0;
}

/***************************************************/
/***************************************************/
/***************************************************/
