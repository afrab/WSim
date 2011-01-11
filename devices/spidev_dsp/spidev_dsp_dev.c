
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
#include "mydsp.h"

/***************************************************/
/***************************************************/

#define DSP_MODE_SLAVE  0
#define DSP_MODE_MASTER 1

#define DSP_MASTER_BAUDRATE 1000000

/***************************************************/
/***************************************************/

#undef DEBUG


#ifdef DEBUG
#define HW_DMSG_SPI(x...) HW_DMSG_DEV(x)
#else
#define HW_DMSG_SPI(x...) do {} while(0)
#endif

/***************************************************/
/***************************************************/
/***************************************************/

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
  uint32_t master_data_w_val;
  uint32_t master_data_w_mask;
  
  uint32_t master_data_w_nextval;
  uint64_t master_data_tx_lag_time;
  uint64_t master_data_tx_time;       /* nano second */
  uint8_t  master_data_tx_processing; /* boolean     */

  /*********************/
  /* Internals         */
  /*********************/
  /* current spi mode */
  uint8_t  dsp_mode;          /* boolean     */
  struct dsp_internal_state_t dsp_state;
};


#define DSP_NAME   machine.device[dev].name
#define DSP_DATA   ((struct spidev_dsp_t*)(machine.device[dev].data))
#define DSP_MODE   DSP_DATA->dsp_mode
#define DSP_STATE  DSP_DATA->dsp_state

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
  strncpyz(machine.device[dev].name, dev_name, DSP_NAME_MAX);

  TRACER_SPIDEV_DSP_STATE  = tracer_event_add_id(8, "state"  , dev_name);
  TRACER_SPIDEV_DSP_STROBE = tracer_event_add_id(8, "strobe" , dev_name);

  mydsp_create(& DSP_STATE);
  spidev_dsp_reset(dev);
  HW_DMSG_SPI("%s: device create done\n", DSP_NAME);
  return 0;
}

/***************************************************/
/***************************************************/
/***************************************************/

int spidev_dsp_reset(int dev)
{
  DSP_DATA->dsp_mode                 = DSP_MODE_SLAVE;
  DSP_DATA->slave_data_r_val         = 0;
  DSP_DATA->slave_data_r_nextval     = 0;

  DSP_DATA->master_data_tx_processing = 0;
  DSP_DATA->master_data_tx_lag_time   = 7 * (1000000000 / DSP_MASTER_BAUDRATE); /* in nano second */
  mydsp_reset(& DSP_STATE);
  mydsp_mode(& DSP_STATE, MYDSP_PASSIVE);
  HW_DMSG_SPI("%s: device reset lag=%d\n", DSP_NAME, DSP_DATA->master_data_tx_lag_time);
  return 0;
}

/***************************************************/
/***************************************************/
/***************************************************/

int spidev_dsp_delete(int dev)
{
  HW_DMSG_SPI("%s: device delete\n", DSP_NAME);
  mydsp_delete(& DSP_STATE);
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

void spidev_dsp_read(int dev, uint32_t *mask, uint32_t *value)
{
  *value  = 0;
  *mask   = 0;

  /***************************
   * Slave data.
   ***************************/
  if (DSP_DATA->slave_data_w_mask)
    {
      *mask  |=  DSP_DATA->slave_data_w_mask; 
      *value |= (DSP_DATA->slave_data_r_val << SPIDEV_DSP_D_SHIFT);
    }

  /***************************
   * Master data.
   ***************************/
  if (DSP_DATA->master_data_w_mask)
    {
      *mask  |=  DSP_DATA->master_data_w_mask;
      *value |= (DSP_DATA->master_data_w_val << SPIDEV_DSP2_D_SHIFT);
      DSP_DATA->master_data_w_mask = 0;
    }


  if (*mask != 0)
    {
      HW_DMSG_SPI("%s: device write to mcu [val=0x%08x,mask=0x%08x] \n", DSP_NAME, *value, *mask);
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
	  mydsp_mode(& DSP_STATE, MYDSP_ACTIVE);
	  HW_DMSG_SPI("%s: spi mode master\n", DSP_NAME);
	}
      else
	{
	  DSP_DATA->dsp_mode      = DSP_MODE_SLAVE;
	  mydsp_mode(& DSP_STATE, MYDSP_PASSIVE);
	  HW_DMSG_SPI("%s: spi mode slave\n", DSP_NAME); 
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

int spidev_dsp_update(int dev)
{
  int dspret;
  /* update current state */
  switch (DSP_MODE) 
    {
    case DSP_MODE_SLAVE:
      if (DSP_DATA->slave_data_w_mask == SPIDEV_DSP_D)
	{
	  mydsp_write( & DSP_STATE, DSP_DATA->slave_data_w_val);
	}
      break;

    case DSP_MODE_MASTER:
      /* current tx */
      if (DSP_DATA->master_data_tx_processing)
	{
	  if (MACHINE_TIME_GET_NANO() >= DSP_DATA->master_data_tx_time)
	    {
	      DSP_DATA->master_data_w_val         = DSP_DATA->master_data_w_nextval;
	      DSP_DATA->master_data_w_mask        = SPIDEV_DSP2_D;
	      DSP_DATA->master_data_tx_processing = 0;
	      // data will be read on next write/read/update cycle 
	      // DSP_DATA->master_dsp_w_mask = 0; in dsp_read() 
	    }
	}
      /* current work */
      dspret = mydsp_update(& DSP_STATE, 
			    & DSP_DATA->master_data_w_nextval, 
			      DSP_DATA->master_data_tx_processing);
      switch (dspret)
	{
	case 0: /* no output */
	  break;
	case 1: /* nextval is ok */
	  DSP_DATA->master_data_tx_processing = 1;
	  DSP_DATA->master_data_tx_time = MACHINE_TIME_GET_NANO() + DSP_DATA->master_data_tx_lag_time;
	  HW_DMSG_SPI("%s: %"PRId64" next byte at %"PRId64"\n", DSP_NAME, MACHINE_TIME_GET_NANO(), DSP_DATA->master_data_tx_time);
	  break;
	default: /* no output */
	  break;
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
