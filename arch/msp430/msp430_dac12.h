/**
 *  \file   msp430_dac12.h
 *  \brief  MSP430 DAC12 controller
 *  \author Antoine Fraboulet
 *  \date   2006
 **/

#ifndef MSP430_DAC12_H
#define MSP430_DAC12_H
#if defined(__msp430_have_dac12)

enum dac12_addr_t {
  DAC12_0CTL = 0x01C0,
  DAC12_1CTL = 0x01C2,
  DAC12_0DAT = 0x01C8,
  DAC12_1DAT = 0x01CA
};

struct msp430_dac12_t {
};

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void    msp430_dac12_reset ();
void    msp430_dac12_update();
int16_t msp430_dac12_read  (uint16_t addr);
void    msp430_dac12_write (uint16_t addr, int16_t val);

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

#endif /* have_dac12 */
#endif
