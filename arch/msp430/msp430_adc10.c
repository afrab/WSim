/**
 *  \file   msp430_adc10.c
 *  \brief  MSP430 Adc10 controller
 *  \author Antoine Fraboulet
 *  \date   2006
 **/

#include "arch/common/hardware.h"
#include "msp430.h"

#if defined(__msp430_have_adc10)
/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void msp430_adc10_create()
{
  msp430_io_register_addr8 (ADC10AE  ,msp430_adc10_read8 ,msp430_adc10_write8);
  msp430_io_register_addr8 (ADC10DTC0,msp430_adc10_read8 ,msp430_adc10_write8);
  msp430_io_register_addr8 (ADC10DTC1,msp430_adc10_read8 ,msp430_adc10_write8);

  msp430_io_register_addr16(ADC10CTL0,msp430_adc10_read16,msp430_adc10_write16);
  msp430_io_register_addr16(ADC10CTL1,msp430_adc10_read16,msp430_adc10_write16);
  msp430_io_register_addr16(ADC10MEM ,msp430_adc10_read16,msp430_adc10_write16);
  msp430_io_register_addr16(ADC10SA  ,msp430_adc10_read16,msp430_adc10_write16);
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void msp430_adc10_reset()
{
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void msp430_adc10_update()
{
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

int16_t msp430_adc10_read16 (uint16_t addr)
{
  ERROR("msp430:adc10: read [0x%04x] block not implemented\n",addr);
  return 0;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void msp430_adc10_write16 (uint16_t addr, int16_t val)
{
  ERROR("msp430:adc10: write [0x%04x] = 0x%04x, block not implemented\n",addr,val);
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

int8_t msp430_adc10_read8  (uint16_t addr)
{
  ERROR("msp430:adc10: read [0x%04x] block not implemented\n",addr);
  return 0;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void msp430_adc10_write8 (uint16_t addr, int8_t val)
{
  ERROR("msp430:adc10: write [0x%04x] = 0x%02x, block not implemented\n",addr,val);
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

#endif
