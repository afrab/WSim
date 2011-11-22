
/**
 *  \file   cma3000_spi_dev.c
 *  \brief  CMA3000 Accel sensor in SPI Mode
 *  \author Bernhard Dick
 *  \date   2011
 **/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "arch/common/hardware.h"
#include "devices/cma3000_spi/cma3000_spi_dev.h"
#include "src/options.h"

/***************************************************/
/***************************************************/

#define NAME      "cma3000_spi"

tracer_id_t TRACER_CMA3000_SPI_STATE;
tracer_id_t TRACER_CMA3000_SPI_STROBE;

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

struct cma3000_spi_t {
  uint8_t select_bit; /* chip select   */
  uint8_t write_protect_bit; /* write protect */

  uint8_t got_data; /* did we get anything? */
  /* data just written */
  uint32_t data_w_val;
  uint32_t data_w_mask;
  uint8_t data_w_ready;

  /* data to be read */
  uint8_t data_r_ready; /* boolean */
  uint32_t data_r_mask; /* mask    */
  uint32_t data_r_val; /* value   */

  /* busy timing */
  uint64_t end_of_busy_time; /* nano second */

  uint8_t registers[25]; /* registers */
  uint8_t addr; /* address to read */
  uint8_t state; /* 0 first byte, 1 second byte*/
  uint8_t write;

  uint32_t cycle_count; /* cycle count */
  uint8_t INT_send; /* INT Pin */
};

#define CMA3000_SPI_DATA        ((struct cma3000_spi_t*)(machine.device[dev].data))

/***************************************************/
/** Flash external entry points ********************/
/***************************************************/

int cma3000_spi_reset(int dev);
int cma3000_spi_delete(int dev);
int cma3000_spi_power_up(int dev);
int cma3000_spi_power_down(int dev);
void cma3000_spi_read(int dev, uint32_t *mask, uint32_t *value);
void cma3000_spi_write(int dev, uint32_t mask, uint32_t value);
int cma3000_spi_update(int dev);
int cma3000_spi_ui_draw(int dev);
void cma3000_spi_ui_get_size(int dev, int *w, int *h);
void cma3000_spi_ui_set_pos(int dev, int x, int y);
void cma3000_spi_ui_get_pos(int dev, int *x, int *y);

/***************************************************/
/***************************************************/
/***************************************************/

#define MAXNAME 1024

int cma3000_spi_add_options(int UNUSED dev_num, int dev_id, const char UNUSED *dev_name)
{
  if (dev_id >= 1) {
    ERROR("spidev: too much devices, please rewrite option handling\n");
    return -1;
  }

  return 0;
}

/***************************************************/
/***************************************************/

/***************************************************/

int cma3000_spi_device_size()
{
  return sizeof (struct cma3000_spi_t);
}

/***************************************************/
/***************************************************/

/***************************************************/

int cma3000_spi_device_create(int dev, int UNUSED id)
{
  machine.device[dev].reset = cma3000_spi_reset;
  machine.device[dev].delete = cma3000_spi_delete;
  machine.device[dev].power_up = cma3000_spi_power_up;
  machine.device[dev].power_down = cma3000_spi_power_down;

  machine.device[dev].read = cma3000_spi_read;
  machine.device[dev].write = cma3000_spi_write;
  machine.device[dev].update = cma3000_spi_update;

  machine.device[dev].ui_draw = cma3000_spi_ui_draw;
  machine.device[dev].ui_get_size = cma3000_spi_ui_get_size;
  machine.device[dev].ui_set_pos = cma3000_spi_ui_set_pos;

  machine.device[dev].state_size = cma3000_spi_device_size();
  machine.device[dev].name = NAME;

  CMA3000_SPI_DATA->registers[CMA3000_SPI_WHO_AM_I] = 0x00;
  CMA3000_SPI_DATA->registers[CMA3000_SPI_REVID] = 0xF0;
  CMA3000_SPI_DATA->registers[CMA3000_SPI_CTRL] = 0x00;
  CMA3000_SPI_DATA->registers[CMA3000_SPI_STATUS] = 0x00;
  CMA3000_SPI_DATA->registers[CMA3000_SPI_RSTR] = 0x00;
  CMA3000_SPI_DATA->registers[CMA3000_SPI_INT_STATUS] = 0x00;
  CMA3000_SPI_DATA->registers[CMA3000_SPI_DOUTX] = 0x23;
  CMA3000_SPI_DATA->registers[CMA3000_SPI_DOUTY] = 0x37;
  CMA3000_SPI_DATA->registers[CMA3000_SPI_DOUTZ] = 0x42;
  CMA3000_SPI_DATA->registers[CMA3000_SPI_MDTHR] = 0x08;
  CMA3000_SPI_DATA->registers[CMA3000_SPI_MDFFTMR] = 0x33;
  CMA3000_SPI_DATA->registers[CMA3000_SPI_FFTHR] = 0x08;
  CMA3000_SPI_DATA->registers[CMA3000_SPI_I2C_ADDR] = 0x1C;


  CMA3000_SPI_DATA->addr = 0x00;
  CMA3000_SPI_DATA->state = 0;
  CMA3000_SPI_DATA->write = 0;
  CMA3000_SPI_DATA->select_bit = 1;
  CMA3000_SPI_DATA->got_data = 0;

  CMA3000_SPI_DATA->cycle_count = 0;
  CMA3000_SPI_DATA->INT_send = 0;

#if DEBUG_ME_HARDER != 0
  HW_DMSG_SPI(NAME ": =================================== \n");
  HW_DMSG_SPI(NAME ": 0000 CHSW dddd dddd == MASK         \n");
  HW_DMSG_SPI(NAME ":      C              : Clock         \n");
  HW_DMSG_SPI(NAME ":       M             : MiSo          \n");
  HW_DMSG_SPI(NAME ":        S            : CSb           \n");
  HW_DMSG_SPI(NAME ":         W           : Write Protect \n");
  HW_DMSG_SPI(NAME ":           dddd dddd : SPI data      \n");
  HW_DMSG_SPI(NAME ": =================================== \n");
#endif

  TRACER_CMA3000_SPI_STATE = tracer_event_add_id(8, "state", NAME);
  TRACER_CMA3000_SPI_STROBE = tracer_event_add_id(8, "strobe", NAME);

  return 0;
}

/***************************************************/
/***************************************************/

/***************************************************/

int cma3000_spi_reset(int UNUSED dev)
{
  HW_DMSG_SPI(NAME ": device reset\n");
  return 0;
}

/***************************************************/
/***************************************************/

/***************************************************/

int cma3000_spi_delete(int UNUSED dev)
{
  HW_DMSG_SPI(NAME ": device delete\n");
  return 0;
}

/***************************************************/
/***************************************************/

/***************************************************/

int cma3000_spi_power_up(int UNUSED dev)
{
  HW_DMSG_SPI(NAME ": device power up\n");
  return 0;
}

int cma3000_spi_power_down(int UNUSED dev)
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

void cma3000_spi_read(int dev, uint32_t *mask, uint32_t *value)
{

  *mask = 0;
  *value = 0;

  uint8_t porst = ((CMA3000_SPI_DATA->registers[CMA3000_SPI_STATUS] & 0x08) == 0x08);
  if (CMA3000_SPI_DATA->select_bit == 0 && CMA3000_SPI_DATA->got_data == 1) {
    *mask |= CMA3000_SPI_D;
    if (CMA3000_SPI_DATA->state == 0) {
      *value |= 0x02 | (porst << 5) | (porst << 4) | (porst << 3);
    } else {
      *value |= CMA3000_SPI_DATA->registers[CMA3000_SPI_DATA->addr];
      if (CMA3000_SPI_DATA->addr == 0x06 || CMA3000_SPI_DATA->addr == 0x07 || CMA3000_SPI_DATA->addr == 0x08) { // data read
        CMA3000_SPI_DATA->cycle_count = 0;
        CMA3000_SPI_DATA->INT_send = 0;
      }
    }
  }

  *mask |= CMA3000_SPI_INT_MASK;
  *value |= (CMA3000_SPI_DATA->INT_send << CMA3000_SPI_INT_SHIFT);

  if (*mask != 0) {
    HW_DMSG_SPI(NAME ": device write to mcu [val=0x%02x,mask=0x%04x] \n", *value, *mask);
  }
}

/***************************************************/
/***************************************************/

/***************************************************/

void cma3000_spi_write(int dev, uint32_t mask, uint32_t value)
{
  HW_DMSG_SPI(NAME ": device write from mcu value 0x%04x mask 0x%04x\n", value, mask);

  /***************************
   * Control pins. CSB
   ***************************/

  if (mask & CMA3000_SPI_CSb) /* write protect netgated */ {
    if (value & CMA3000_SPI_CSb) {
      CMA3000_SPI_DATA->select_bit = 1;
    } else {
      CMA3000_SPI_DATA->select_bit = 0;
    }
  }

  /***************************
   * Control pins. CLOCK
   ***************************/

  if (mask & CMA3000_SPI_C) /* clock */ {
    ERROR(NAME ":    clock pin should not be used during simulation\n");
  }

  /***************************
   * DATA pins.
   ***************************/
  if (mask & CMA3000_SPI_D) {
    CMA3000_SPI_DATA->got_data = 1;
    uint8_t data = (uint8_t) (value & CMA3000_SPI_D);
    if (CMA3000_SPI_DATA->state == 0) { //Control
      CMA3000_SPI_DATA->addr = (data & 0xfC) >> 2;
      CMA3000_SPI_DATA->write = (data & 0x02) == 0x02;
    } else { //second part
      if (CMA3000_SPI_DATA->write == 0x02) {
        CMA3000_SPI_DATA->registers[CMA3000_SPI_DATA->addr] = data;
      }
    }


  }
}

/***************************************************/
/***************************************************/

/***************************************************/

int cma3000_spi_update(int dev)
{
  if (CMA3000_SPI_DATA->got_data == 1) {
    if (CMA3000_SPI_DATA->state == 0) {
      CMA3000_SPI_DATA->state = 1;
    } else {
      CMA3000_SPI_DATA->state = 0;
    }
    CMA3000_SPI_DATA->got_data = 0;
  }

  CMA3000_SPI_DATA->cycle_count++;
  if (CMA3000_SPI_DATA->cycle_count == 100001) {
    CMA3000_SPI_DATA->INT_send = 1;
  }
  return 0;
}

/***************************************************/
/***************************************************/

/***************************************************/

int cma3000_spi_ui_draw(int UNUSED dev)
{
  return 0;
}

void cma3000_spi_ui_get_size(int UNUSED dev, int *w, int *h)
{
  w = 0;
  h = 0;
}

void cma3000_spi_ui_set_pos(int UNUSED dev, int UNUSED x, int UNUSED y)
{

}

void cma3000_spi_ui_get_pos(int UNUSED dev, int *x, int *y)
{
  *x = 0;
  *y = 0;
}

/***************************************************/
/***************************************************/
/***************************************************/
