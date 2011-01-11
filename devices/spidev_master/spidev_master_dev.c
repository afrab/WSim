
/**
 *  \file   spidev_master_dev.c
 *  \brief  SPI master device example
 *  \author Loic Lemaitre
 *  \date   2009
 **/


/* This device is inspired from the spidev template to test and 
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
#include "devices/spidev_master/spidev_master_dev.h"
#include "src/options.h"

/***************************************************/
/***************************************************/

#define SPIDEV_MODE_SLAVE  0
#define SPIDEV_MODE_MASTER 1

#define SPIDEV_MASTER_BAUDRATE 38400

#define NAME      "spidevmaster"

tracer_id_t TRACER_SPIDEV_STATE;
tracer_id_t TRACER_SPIDEV_STROBE;

/***************************************************/
/***************************************************/

#undef DEBUG

#ifdef DEBUG
#define DEBUG_ME_HARDER 0
#define HW_DMSG_SPI(x...) HW_DMSG_DEV(x)
#else
#define HW_DMSG_SPI(x...) do {} while(0)
#endif

/***************************************************/
/***************************************************/
/***************************************************/
#define TAB_MAX 200
#define SPIDEV_WAIT_FOR_TX 10

struct spidev_t 
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
  uint8_t  spidev_data[TAB_MAX];
  int32_t  spidev_index;
  int32_t  spidev_index_max;

  /* current spi mode */
  uint8_t  spidev_mode;          /* boolean     */

  /* spi master time handling */
  uint64_t spidev_tx_start_time; /* nano second */
  uint64_t spidev_tx_lag_time;   /* nano second */
  uint8_t  spidev_tx_processing; /* boolean     */
  int      spidev_wait_for_tx;   /* number of update call to drop before sending */

};

#define SPIDEVMAST      ((struct spidev_t*)(machine.device[dev].data))

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

#if DEBUG_ME_HARDER != 0
  HW_DMSG_SPI(NAME ": =================================== \n");
  HW_DMSG_SPI(NAME ": 0000 CMSW dddd dddd == MASK         \n");
  HW_DMSG_SPI(NAME ":      C              : Clock         \n");
  HW_DMSG_SPI(NAME ":       M             : SPI Mode      \n");
  HW_DMSG_SPI(NAME ":        S            : Chip Select   \n");
  HW_DMSG_SPI(NAME ":         W           : Write Protect \n");
  HW_DMSG_SPI(NAME ":           dddd dddd : SPI data      \n");
  HW_DMSG_SPI(NAME ": =================================== \n");
#endif

  TRACER_SPIDEV_STATE  = tracer_event_add_id(8, "state"  , NAME);
  TRACER_SPIDEV_STROBE = tracer_event_add_id(8, "strobe" , NAME);

  SPIDEVMAST->spidev_index         = 0;
  SPIDEVMAST->spidev_index_max     = 0;
  SPIDEVMAST->spidev_mode          = SPIDEV_MODE_SLAVE;
  SPIDEVMAST->spidev_tx_processing = 0;
  SPIDEVMAST->spidev_tx_lag_time   = 7 * (1000000000 / SPIDEV_MASTER_BAUDRATE); /* in nano second */
  HW_DMSG_SPI(NAME ": lag time = %lld\n", SPIDEVMAST->spidev_tx_lag_time);
  SPIDEVMAST->spidev_wait_for_tx = SPIDEV_WAIT_FOR_TX;

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
  *value = 0;
  *mask  = SPIDEVMAST->data_r_mask;

  /***************************
   * DATA pins.
   ***************************/

  if (*mask && SPIDEV_D)
    {
      *value = SPIDEVMAST->data_r_val;
    }

  if (*mask != 0)
    {
      HW_DMSG_SPI(NAME ": device write to mcu [val=0x%02x,mask=0x%04x] \n", *value, *mask);
    }

  SPIDEVMAST->data_r_mask = 0;
}

/***************************************************/
/***************************************************/
/***************************************************/

void spidev_fill_tab(int dev, uint8_t val)
{
  if (SPIDEVMAST->spidev_mode == SPIDEV_MODE_SLAVE)  
    {
      SPIDEVMAST->data_w_val = val;
      SPIDEVMAST->data_r_val = val;
      SPIDEVMAST->data_r_mask = SPIDEV_D;
      SPIDEVMAST->spidev_data[SPIDEVMAST->spidev_index] = val;
      HW_DMSG_SPI(NAME ": value from SOMI put in the table at index %d [0x%02x]\n", SPIDEVMAST->spidev_index, val);
      SPIDEVMAST->spidev_index++;
    }
  else
    SPIDEVMAST->data_w_val = val;
}


void spidev_write(int dev, uint32_t mask, uint32_t value)
{
  HW_DMSG_SPI(NAME ": device write from mcu value 0x%04x mask 0x%04x\n",value, mask);


  /***************************
   * Control pins. WRITE PROTECT
   ***************************/

  if (mask & SPIDEV_W) /* write protect netgated */
    {
      //SPIDEVMAST->write_protect_bit  = ! (value & SPIDEV_W);
      //HW_DMSG_SPI(NAME ":    flash write protect W = %d\n",SPIDEVMAST->write_protect_bit);
    }


  /***************************
   * Control pins. CLOCK
   ***************************/

  if (mask & SPIDEV_C) /* clock */
    {
      ERROR(NAME ":    clock pin should not be used during simulation\n");
      HW_DMSG_SPI(NAME ":    clock pin should not be used during simulation\n");
      // SPIDEV_DATA->clock = (value >> SPIDEV_C_SHIFT) & 0x1;
    }


  /***************************
   * Control pins. SPI MODE
   ***************************/

  if (mask & SPIDEV_M)
    {
      if (value)
	{
	  SPIDEVMAST->spidev_mode = SPIDEV_MODE_MASTER;
	  SPIDEVMAST->spidev_index_max = SPIDEVMAST->spidev_index;
	  HW_DMSG_SPI(NAME ": spi mode master, index max = %d\n", SPIDEVMAST->spidev_index_max); 
	}
      else
	{
	  SPIDEVMAST->spidev_mode = SPIDEV_MODE_SLAVE;
	  SPIDEVMAST->spidev_index_max = 0;
	  SPIDEVMAST->spidev_index = 0;
	  HW_DMSG_SPI(NAME ": spi mode master\n"); 
	}
    }


  /***************************
   * DATA pins.
   ***************************/

  if (mask & SPIDEV_D)
    { 
      spidev_fill_tab(dev, (uint8_t)(value & SPIDEV_D));
    }
}

/***************************************************/
/***************************************************/
/***************************************************/

void spidev_tx_end(int UNUSED dev)
{
  SPIDEVMAST->data_r_val = SPIDEVMAST->data_r_val_temp;
  SPIDEVMAST->data_r_mask     = SPIDEV_D;
  SPIDEVMAST->spidev_tx_processing = 0;
  HW_DMSG_SPI(NAME ": Sending to mcu: value %d of the table [0x%02x], time=%lldns\n", 
	      SPIDEVMAST->spidev_index_max - SPIDEVMAST->spidev_index, SPIDEVMAST->data_r_val, MACHINE_TIME_GET_NANO());
  SPIDEVMAST->spidev_wait_for_tx = SPIDEV_WAIT_FOR_TX;
}


void spidev_tx_start(int UNUSED dev)
{
  SPIDEVMAST->data_r_val_temp      = SPIDEVMAST->spidev_data[SPIDEVMAST->spidev_index_max - SPIDEVMAST->spidev_index];
  SPIDEVMAST->spidev_tx_start_time = MACHINE_TIME_GET_NANO();
  SPIDEVMAST->spidev_tx_processing = 1;

  HW_DMSG_SPI(NAME ": start of wait before sending to mcu: value %d of the table [0x%02x], time=%lldns\n", SPIDEVMAST->spidev_index_max - SPIDEVMAST->spidev_index, SPIDEVMAST->data_r_val_temp, MACHINE_TIME_GET_NANO());

  SPIDEVMAST->spidev_index--;

  if (SPIDEVMAST->spidev_index < 0)
    {
      SPIDEVMAST->spidev_index = 0;
    }
}


int spidev_update(int UNUSED dev)
{
  SPIDEVMAST->spidev_wait_for_tx--;

  if (SPIDEVMAST->spidev_tx_processing && SPIDEVMAST->spidev_mode == SPIDEV_MODE_MASTER)
    {
      if(SPIDEVMAST->spidev_tx_start_time + SPIDEVMAST->spidev_tx_lag_time <= MACHINE_TIME_GET_NANO())
	{
	  spidev_tx_end(dev);	  
	}
      return 0;
    }
	 

  if (SPIDEVMAST->spidev_mode == SPIDEV_MODE_MASTER && 
      SPIDEVMAST->spidev_wait_for_tx <= 0)
    {
      spidev_tx_start(dev);
    }

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
