/**
 *  \file   msp430_lcdb.c
 *  \brief  MSP430 LCD_B definition based upon msp430_lcd.c
 *  \author Bernhard Dick
 *  \date   2011
 **/

#include <stdio.h>

#include "arch/common/hardware.h"
#include "msp430.h"

#if defined(__msp430_have_lcdb)
/* ************************************************** */
/* ************************************************** */

/* ************************************************** */

void msp430_lcdb_create()
{
  msp430_io_register_range8(LCDB_IOMEM_BEGIN, LCDB_IOMEM_END, msp430_lcdb_read8, msp430_lcdb_write8);
  msp430_io_register_range16(LCDB_IOMEM_BEGIN, LCDB_IOMEM_END, msp430_lcdb_read, msp430_lcdb_write);
}

/* ************************************************** */
/* ************************************************** */

/* ************************************************** */

int msp430_lcdb_reset()
{
  MCU.lcdb.lcdbctl0.s = 0x00;
  MCU.lcdb.lcdbctl1.s = 0x00;
  MCU.lcdb.lcdbblkctl.s = 0x00;
  return 0;
}

/* ************************************************** */
/* ************************************************** */

/* ************************************************** */

void msp430_lcdb_update()
{
  //always set the LCD framing INT.
  MCU.lcdb.lcdbctl1.b.lcdfrmifg = 1;
}

/* ************************************************** */
/* ************************************************** */

/* ************************************************** */

int16_t msp430_lcdb_read(uint16_t addr)
{
  int16_t res;
  if (addr == LCDB_LCDBCTL0) {
    res = MCU.lcdb.lcdbctl0.s;
  } else if (addr == LCDB_LCDBCTL1) {
    res = MCU.lcdb.lcdbctl1.s;
  } else if (addr == LCDB_LCDBBLKCTL) {
    res = MCU.lcdb.lcdbblkctl.s;
  } else if ((addr >= LCDB_LCDM_START) && (addr <= LCDB_LCDM_STOP)) {
    res = MCU.lcdb.mem[addr - LCDB_LCDM_START];
  } else if ((addr >= LCDB_LCDBM_START) && (addr <= LCDB_LCDBM_STOP)) {
    res = MCU.lcdb.bmem[addr - LCDB_LCDBM_START];
  } else {
    ERROR("msp430:lcdb: bad read8 address 0x%04x\n", addr);
    res = 0;
  }
  return res;
}

/* ************************************************** */
/* ************************************************** */

/* ************************************************** */

int8_t msp430_lcdb_read8(uint16_t addr)
{
  int8_t res;
  if ((addr >= LCDB_LCDM_START) && (addr <= LCDB_LCDM_STOP)) {
    res = MCU.lcdb.mem[addr - LCDB_LCDM_START];
  } else if ((addr >= LCDB_LCDBM_START) && (addr <= LCDB_LCDBM_STOP)) {
    res = MCU.lcdb.bmem[addr - LCDB_LCDBM_START];
  } else {
    ERROR("msp430:lcdb: bad read address 0x%04x\n", addr);
    res = 0;
  }
  return res;
}

/* ************************************************** */
/* ************************************************** */

/* ************************************************** */

void msp430_lcdb_write(uint16_t addr, int16_t val)
{
  if (addr == LCDB_LCDBCTL0) {
    MCU.lcdb.lcdbctl0.s = val;
  } else if (addr == LCDB_LCDBCTL1) {
    MCU.lcdb.lcdbctl1.s = val;
  } else if (addr == LCDB_LCDBBLKCTL) {
    MCU.lcdb.lcdbblkctl.s = val;
  } else if ((addr >= LCDB_LCDM_START) && (addr <= LCDB_LCDM_STOP)) {
    MCU.lcdb.mem[addr - LCDB_LCDM_START] = val;
  } else if ((addr >= LCDB_LCDBM_START) && (addr <= LCDB_LCDBM_STOP)) {
    MCU.lcdb.bmem[addr - LCDB_LCDBM_START] = val;
  } else {
    ERROR("msp430:lcd: bad write address 0x%04x = 0x%02x\n", addr, val & 0xff);
  }
}

/* ************************************************** */
/* ************************************************** */

/* ************************************************** */

void msp430_lcdb_write8(uint16_t addr, int8_t val)
{
  if ((addr >= LCDB_LCDM_START) && (addr <= LCDB_LCDM_STOP)) {
    MCU.lcdb.mem[addr - LCDB_LCDM_START] = val;
  } else if ((addr >= LCDB_LCDBM_START) && (addr <= LCDB_LCDBM_STOP)) {
    MCU.lcdb.bmem[addr - LCDB_LCDBM_START] = val;
  } else {
    ERROR("msp430:lcd: bad write address 0x%04x = 0x%02x\n", addr, val & 0xff);
  }
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

#endif // _have_lcdb
