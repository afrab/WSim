
/**
 *  \file   mcugen_mac.c
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

#if defined(ENABLE_RAM_CONTROL)

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

uint8_t MCU_RAMCTL  [MAX_RAM_SIZE];

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void mcu_ramctl_init(void)
{
  int i;
  for(i=0; i<MAX_RAM_SIZE; i++)
    {
      MCU_RAMCTL[i] = MAC_MUST_WRITE_FIRST;
    }
}

void mcu_ramctl_tst_read(uint16_t addr)
{
  uint8_t b = MCU_RAMCTL[addr];
  if ((b & MAC_WATCH_READ) != 0)
    {
      mcu_signal_add( SIG_MAC | MAC_TO_SIG(MAC_WATCH_READ) );
    }
}

void mcu_ramctl_tst_write(uint16_t addr)
{
  uint8_t b = MCU_RAMCTL[addr];
  if ((b & MAC_WATCH_WRITE) != 0)
    {
      mcu_signal_add( SIG_MAC | MAC_TO_SIG(MAC_WATCH_WRITE) );
    }
} 

void mcu_ramctl_tst_fetch(uint16_t addr)
{
  uint8_t b = MCU_RAMCTL[addr];
  if ((b & MAC_BREAK_WATCH_FETCH) != 0)
    {
      mcu_signal_add( SIG_MAC | MAC_TO_SIG(b & MAC_BREAK_WATCH_FETCH) );
    }
}

void mcu_ramctl_set_bp(uint16_t addr, int type)
{
  MCU_RAMCTL[addr] |= type;
}

void mcu_ramctl_unset_bp(uint16_t addr, int type)
{
  MCU_RAMCTL[addr] &= ~type;
}

void mcu_ramctl_read(uint16_t addr)
{
  mcu_ramctl_tst_read(addr);
}

void mcu_ramctl_read_block(uint16_t addr, int size)
{
  int i;
  for(i=0; i<size; i++)
    {
      mcu_ramctl_read(addr+i);
    }
}

void mcu_ramctl_write(uint16_t addr)
{
  mcu_ramctl_tst_write(addr);
  MCU_RAMCTL[addr] &= ~MAC_MUST_WRITE_FIRST;
}

void mcu_ramctl_write_block(uint16_t addr, int size)
{
  int i;
  for(i=0; i<size; i++)
    {
      mcu_ramctl_write(addr + i);
    }
}

uint8_t mcu_ramctl_read_ctl(uint16_t addr)
{
  uint8_t ret = 0;
  /* if (ret < MAX_RAM_SIZE) */
    {
      ret = MCU_RAMCTL[addr];
    }
  return ret;
}

#endif

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */
