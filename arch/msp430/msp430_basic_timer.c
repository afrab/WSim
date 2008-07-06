
/**
 *  \file   msp430_basic_timer.c
 *  \brief  MSP430 Basic timer definition
 *  \author Antoine Fraboulet
 *  \date   2005
 **/

#include <stdio.h> 
#include <string.h>

#include "arch/common/hardware.h"
#include "msp430.h"

#if defined(__msp430_have_basic_timer)
/* ************************************************** */
/* ************************************************** */
/* ************************************************** */
void msp430_basic_timer_reset()
{
  memset(&MCU.bt,0,sizeof(struct msp430_basic_timer_t));
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void msp430_basic_timer_update()
{
  /* do something ? */
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

int8_t msp430_basic_timer_read (uint16_t addr)
{
  int8_t res = 0;
  switch (addr)
    {
    case BT_CTL:
      break;
    case BT_CNT1:
      break;
    case BT_CNT2:
      break;
    default:
      ERROR("msp430:basic_timer: bad read address [0x%04x]\n",addr);
      break;
    }
  ERROR("msp430:basic_timer: read [0x%04x] = 0x%02x, block not implemented\n",addr,res);
  return res;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void msp430_basic_timer_write(uint16_t addr, int8_t val)
{
  ERROR("msp430:basic_timer: write [0x%04x] = 0x%02x, block not implemented\n",addr,val);
  switch (addr)
    {
    case BT_CTL:
      break;
    case BT_CNT1:
      break;
    case BT_CNT2:
      break;
    default:
      ERROR("msp430:basic_timer: bad write address [0x%04x] = 0x%02x\n",addr,val);
      break;
    }
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

int msp430_basic_timer_chkifg()
{
  int ret = 0;
  return ret;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */
#endif
