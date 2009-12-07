
/**
 *  \file   mcugen_jtag.c
 *  \brief  Generic MCU ALU emulation
 *  \author Antoine Fraboulet
 *  \date   2009
 **/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "arch/common/hardware.h"
#include "mcugen.h"


/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

uint8_t  mcu_jtag_read_byte(uint16_t addr)
{
  return MCU_RAM[addr];
}

uint16_t  mcu_jtag_read_word(uint16_t addr)
{
  return MCU_RAM[addr+1] << 8 | MCU_RAM[addr];
}

void mcu_jtag_write_byte(uint16_t addr, uint8_t val)
{
  MCU_RAM[addr] = val;
  mcu_ramctl_write(addr);
}

void mcu_jtag_write_word(uint16_t addr, uint16_t val)
{
  MCU_RAM[addr    ] = (val & 0xff);
  MCU_RAM[addr + 1] = (val >> 8) & 0xff;
  mcu_ramctl_write(addr);
  mcu_ramctl_write(addr+1);
}

int mcu_jtag_read_section(uint8_t *mem, uint16_t start, uint16_t size)
{
  long max_size;
  
  if (start + size > MAX_RAM_SIZE)
    max_size = (MAX_RAM_SIZE - start) - 1;
  else
    max_size = size;

  memcpy(mem, MCU_RAM + start, max_size);
  return max_size;
}

void mcu_jtag_write_section(uint8_t *mem, uint16_t start, uint16_t size)
{
  memcpy(MCU_RAM + start, mem, size);
  mcu_ramctl_write_block(start,size);
}

void mcu_jtag_write_zero(uint16_t start, uint16_t size)
{
  memset(MCU_RAM + start, 0, size);
  mcu_ramctl_write_block(start,size);
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */
