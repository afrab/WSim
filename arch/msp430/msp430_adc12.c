/**
 *  \file   msp430_adc12.c
 *  \brief  MSP430 Adc12 controller
 *  \author Antoine Fraboulet
 *  \date   2006
 **/

#include "arch/common/hardware.h"
#include "msp430.h"

#if defined(__msp430_have_adc12)
/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void msp430_adc12_reset(void)
{
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void msp430_adc12_update(void)
{
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

int16_t msp430_adc12_read16(uint16_t addr)
{
  static int tinyos_read_this_too_many_times = 0;
  if (tinyos_read_this_too_many_times == 0)
    {
      ERROR("msp430:adc12: read [0x%04x] block not implemented\n",addr);
      tinyos_read_this_too_many_times = 1;
    }
  return 0;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void msp430_adc12_write16(uint16_t addr, int16_t val)
{
  ERROR("msp430:adc12: write [0x%04x] = 0x%04x, block not implemented\n",addr,val & 0xffff);
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

int8_t msp430_adc12_read8(uint16_t addr)
{
  static int tinyos_read_this_too_many_times = 0;
  if (tinyos_read_this_too_many_times == 0)
    {
      ERROR("msp430:adc12: read [0x%04x] block not implemented\n",addr);
      tinyos_read_this_too_many_times = 1;
    }
  return 0;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void msp430_adc12_write8(uint16_t addr, int8_t val)
{
  ERROR("msp430:adc12: write [0x%04x] = 0x%02x, block not implemented\n",addr,val & 0xff);
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

#endif
