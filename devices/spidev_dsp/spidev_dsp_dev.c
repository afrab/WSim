
/**
 *  \file   spidev_dsp_dev.c
 *  \brief  SPI DSP device example
 *  \author Loic Lemaitre, Antoine Fraboulet
 *  \date   2009
 **/


/* This device is inspired from the spidev_master template to test and 
 * demonstrate how to connect an SPI device in master mode (MSP
 * in slave mode).
 * In addition to the SPI ports, this device uses a SPI mode pin.
 * It enables the device to know if the MSP is in slave or master
 * mode to ajust its own SPI mode.
 *
 * The device behaviour is very simple and only intented to test
 * the right function of the SPI in slave mode : first it saves
 * data coming from the MSP430 (MSP in master mode), and when it
 * receives a SPI master mode signal, goes in SPI master mode, 
 * and starts to send received data through SPI.
 *
 * Note that when a SPI device is in master mode, it has to
 * evaluate the time lag to send a byte through the SPI before
 * sending it, as data is send per byte and as the USART model of
 * the MSP430 implementation in WSim puts in UXRXBUF immediately.
 * 
 * Concerning data in UXTXBUF of the MSP430, a delay is computed
 * only when the SPI is in master mode (MSP430 side), but not in
 * slave mode (as it is already taken into consideration by the 
 * device).
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "arch/common/hardware.h"
#include "devices/spidev_dsp/spidev_dsp_dev.h"
#include "src/options.h"

/***************************************************/
/***************************************************/

#define SPIDEV_DSP_MODE_SLAVE  0
#define SPIDEV_DSP_MODE_MASTER 1

#define SPIDEV_DSP_MASTER_BAUDRATE 38400

#define NAME      "spidsp"

tracer_id_t TRACER_SPIDEV_DSP_STATE;
tracer_id_t TRACER_SPIDEV_DSP_STROBE;

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
#define TAB_MAX 200
#define SPIDEV_DSP_WAIT_FOR_TX 10

struct spidev_dsp_t 
{

  uint8_t select_bit;           /* chip select   */
  uint8_t write_protect_bit;    /* write protect */

  /* data just written */
  uint32_t data_w_val;
  uint32_t data_w_mask;
  uint8_t  data_w_ready;

  /* data to be read */
  uint8_t  data_r_ready;        /* boolean */
  uint32_t data_r_mask;         /* mask    */
  uint32_t data_r_val;          /* value   */

  uint32_t data_r_val_temp;      /* temp value */
  
  /* busy timing */
  uint64_t end_of_busy_time;    /* nano second */

  /* table for test */
  uint8_t  spidev_dsp_data[TAB_MAX];
  int32_t  spidev_dsp_index;
  int32_t  spidev_dsp_index_max;

  /* current spi mode */
  uint8_t  spidev_dsp_mode;          /* boolean     */

  /* spi master time handling */
  uint64_t spidev_dsp_tx_start_time; /* nano second */
  uint64_t spidev_dsp_tx_lag_time;   /* nano second */
  uint8_t  spidev_dsp_tx_processing; /* boolean     */
  int      spidev_dsp_wait_for_tx;   /* number of update call to drop before sending */

};

#define SPIDEV_DSPMAST      ((struct spidev_dsp_t*)(machine.device[dev].data))

/***************************************************/
/** Flash external entry points ********************/
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
      ERROR("spidev_dsp: too much devices, please rewrite option handling\n");
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

int spidev_dsp_device_create(int dev, int UNUSED id)
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
  machine.device[dev].name          = NAME " example";

#if defined(DEBUG_ME_HARDER)
  HW_DMSG_SPI(NAME ": =================================== \n");
  HW_DMSG_SPI(NAME ": 0000 CMSW dddd dddd == MASK         \n");
  HW_DMSG_SPI(NAME ":      C              : Clock         \n");
  HW_DMSG_SPI(NAME ":       M             : SPI Mode      \n");
  HW_DMSG_SPI(NAME ":        S            : Chip Select   \n");
  HW_DMSG_SPI(NAME ":         W           : Write Protect \n");
  HW_DMSG_SPI(NAME ":           dddd dddd : SPI data      \n");
  HW_DMSG_SPI(NAME ": =================================== \n");
#endif

  TRACER_SPIDEV_DSP_STATE  = tracer_event_add_id(8, "state"  , NAME);
  TRACER_SPIDEV_DSP_STROBE = tracer_event_add_id(8, "strobe" , NAME);

  SPIDEV_DSPMAST->spidev_dsp_index         = 0;
  SPIDEV_DSPMAST->spidev_dsp_index_max     = 0;
  SPIDEV_DSPMAST->spidev_dsp_mode          = SPIDEV_DSP_MODE_SLAVE;
  SPIDEV_DSPMAST->spidev_dsp_tx_processing = 0;
  SPIDEV_DSPMAST->spidev_dsp_tx_lag_time   = 7 * (1000000000 / SPIDEV_DSP_MASTER_BAUDRATE); /* in nano second */
  HW_DMSG_SPI(NAME ": lag time = %lld\n", SPIDEV_DSPMAST->spidev_dsp_tx_lag_time);
  SPIDEV_DSPMAST->spidev_dsp_wait_for_tx = SPIDEV_DSP_WAIT_FOR_TX;

  return 0;
}

/***************************************************/
/***************************************************/
/***************************************************/

int spidev_dsp_reset(int UNUSED dev)
{
  HW_DMSG_SPI(NAME ": device reset\n");
  return 0;
}

/***************************************************/
/***************************************************/
/***************************************************/

int spidev_dsp_delete(int UNUSED dev)
{
  HW_DMSG_SPI(NAME ": device delete\n");
  return 0;
}

/***************************************************/
/***************************************************/
/***************************************************/

int spidev_dsp_power_up(int UNUSED dev)
{
  HW_DMSG_SPI(NAME ": device power up\n");

  return 0;
}

int spidev_dsp_power_down(int UNUSED dev)
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

void spidev_dsp_read(int UNUSED dev, uint32_t *mask, uint32_t *value)
{
  *value = 0;
  *mask  = SPIDEV_DSPMAST->data_r_mask;

  /***************************
   * DATA pins.
   ***************************/

  if (*mask && SPIDEV_DSP_D)
    {
      *value = SPIDEV_DSPMAST->data_r_val;
    }

  if (*mask != 0)
    {
      HW_DMSG_SPI(NAME ": device write to mcu [val=0x%02x,mask=0x%04x] \n", *value, *mask);
    }

  SPIDEV_DSPMAST->data_r_mask = 0;
}

/***************************************************/
/***************************************************/
/***************************************************/

void spidev_dsp_fill_tab(int dev, uint8_t val)
{
  if (SPIDEV_DSPMAST->spidev_dsp_mode == SPIDEV_DSP_MODE_SLAVE)  
    {
      SPIDEV_DSPMAST->data_w_val = val;
      SPIDEV_DSPMAST->data_r_val = val;
      SPIDEV_DSPMAST->data_r_mask = SPIDEV_DSP_D;
      SPIDEV_DSPMAST->spidev_dsp_data[SPIDEV_DSPMAST->spidev_dsp_index] = val;
      HW_DMSG_SPI(NAME ": value from SOMI put in the table at index %d [0x%02x]\n", SPIDEV_DSPMAST->spidev_dsp_index, val);
      SPIDEV_DSPMAST->spidev_dsp_index++;
    }
  else
    SPIDEV_DSPMAST->data_w_val = val;
}


void spidev_dsp_write(int dev, uint32_t mask, uint32_t value)
{
  HW_DMSG_SPI(NAME ": device write from mcu value 0x%04x mask 0x%04x\n",value, mask);


  /***************************
   * Control pins. WRITE PROTECT
   ***************************/

  if (mask & SPIDEV_DSP_W) /* write protect netgated */
    {
      //SPIDEV_DSPMAST->write_protect_bit  = ! (value & SPIDEV_DSP_W);
      //HW_DMSG_SPI(NAME ":    flash write protect W = %d\n",SPIDEV_DSPMAST->write_protect_bit);
    }


  /***************************
   * Control pins. CLOCK
   ***************************/

  if (mask & SPIDEV_DSP_C) /* clock */
    {
      ERROR(NAME ":    clock pin should not be used during simulation\n");
      HW_DMSG_SPI(NAME ":    clock pin should not be used during simulation\n");
      // SPIDEV_DSP_DATA->clock = (value >> SPIDEV_DSP_C_SHIFT) & 0x1;
    }


  /***************************
   * Control pins. SPI MODE
   ***************************/

  if (mask & SPIDEV_DSP_M)
    {
      if (value)
	{
	  SPIDEV_DSPMAST->spidev_dsp_mode = SPIDEV_DSP_MODE_MASTER;
	  SPIDEV_DSPMAST->spidev_dsp_index_max = SPIDEV_DSPMAST->spidev_dsp_index;
	  HW_DMSG_SPI(NAME ": spi mode master, index max = %d\n", SPIDEV_DSPMAST->spidev_dsp_index_max); 
	}
      else
	{
	  SPIDEV_DSPMAST->spidev_dsp_mode = SPIDEV_DSP_MODE_SLAVE;
	  SPIDEV_DSPMAST->spidev_dsp_index_max = 0;
	  SPIDEV_DSPMAST->spidev_dsp_index = 0;
	  HW_DMSG_SPI(NAME ": spi mode master\n"); 
	}
    }


  /***************************
   * DATA pins.
   ***************************/

  if (mask & SPIDEV_DSP_D)
    { 
      spidev_dsp_fill_tab(dev, (uint8_t)(value & SPIDEV_DSP_D));
    }
}

/***************************************************/
/***************************************************/
/***************************************************/

void spidev_dsp_tx_end(int UNUSED dev)
{
  SPIDEV_DSPMAST->data_r_val = SPIDEV_DSPMAST->data_r_val_temp;
  SPIDEV_DSPMAST->data_r_mask     = SPIDEV_DSP_D;
  SPIDEV_DSPMAST->spidev_dsp_tx_processing = 0;
  HW_DMSG_SPI(NAME ": Sending to mcu: value %d of the table [0x%02x], time=%lldns\n", 
	      SPIDEV_DSPMAST->spidev_dsp_index_max - SPIDEV_DSPMAST->spidev_dsp_index, SPIDEV_DSPMAST->data_r_val, MACHINE_TIME_GET_NANO());
  SPIDEV_DSPMAST->spidev_dsp_wait_for_tx = SPIDEV_DSP_WAIT_FOR_TX;
}


void spidev_dsp_tx_start(int UNUSED dev)
{
  SPIDEV_DSPMAST->data_r_val_temp      = SPIDEV_DSPMAST->spidev_dsp_data[SPIDEV_DSPMAST->spidev_dsp_index_max - SPIDEV_DSPMAST->spidev_dsp_index];
  SPIDEV_DSPMAST->spidev_dsp_tx_start_time = MACHINE_TIME_GET_NANO();
  SPIDEV_DSPMAST->spidev_dsp_tx_processing = 1;

  HW_DMSG_SPI(NAME ": start of wait before sending to mcu: value %d of the table [0x%02x], time=%lldns\n", SPIDEV_DSPMAST->spidev_dsp_index_max - SPIDEV_DSPMAST->spidev_dsp_index, SPIDEV_DSPMAST->data_r_val_temp, MACHINE_TIME_GET_NANO());

  SPIDEV_DSPMAST->spidev_dsp_index--;

  if (SPIDEV_DSPMAST->spidev_dsp_index < 0)
    {
      SPIDEV_DSPMAST->spidev_dsp_index = 0;
    }
}


int spidev_dsp_update(int UNUSED dev)
{
  SPIDEV_DSPMAST->spidev_dsp_wait_for_tx--;

  if (SPIDEV_DSPMAST->spidev_dsp_tx_processing && SPIDEV_DSPMAST->spidev_dsp_mode == SPIDEV_DSP_MODE_MASTER)
    {
      if(SPIDEV_DSPMAST->spidev_dsp_tx_start_time + SPIDEV_DSPMAST->spidev_dsp_tx_lag_time <= MACHINE_TIME_GET_NANO())
	{
	  spidev_dsp_tx_end(dev);	  
	}
      return 0;
    }
	 

  if (SPIDEV_DSPMAST->spidev_dsp_mode == SPIDEV_DSP_MODE_MASTER && 
      SPIDEV_DSPMAST->spidev_dsp_wait_for_tx <= 0)
    {
      spidev_dsp_tx_start(dev);
    }

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
