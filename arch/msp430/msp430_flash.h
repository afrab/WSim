/**
 *  \file   msp430_flash.h
 *  \brief  MSP430 Flash controller
 *  \author Antoine Fraboulet
 *  \date   2006
 **/

#ifndef MSP430_FLASH_H
#define MSP430_FLASH_H
#if defined(__msp430_have_flash)

#define FLASH_START  0x128
#define FLASH_END    0x12C

enum flash_addr_t {
  FCTL1     = 0x0128,
  FCTL2     = 0x012A,
  FCTL3     = 0x012C
};

struct msp430_flash_t {
};

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void    msp430_flash_reset ();
void    msp430_flash_update();
int16_t msp430_flash_read  (uint16_t addr);
void    msp430_flash_write (uint16_t addr, int16_t val);
#define msp430_flash_chkifg() 0

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

#endif /* have_flash */
#endif
