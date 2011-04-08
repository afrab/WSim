/**
 *  \file   msp430_cmpa.h
 *  \brief  MSP430 Cmpa controller
 *  \author Antoine Fraboulet
 *  \date   2006
 **/

#ifndef MSP430_CMPA_H
#define MSP430_CMPA_H
#if defined(__msp430_have_cmpa)

#define CMPA_START  0x059
#define CMPA_END    0x05B

enum cmpa_addr_t {
  CMPACTL1   =  0x059,
  CMPACTL2   =  0x05A,
  CAPD       =  0x05B
};

struct msp430_cmpa_t {
};

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void   msp430_cmpa_create();
void   msp430_cmpa_reset ();
void   msp430_cmpa_update();
int8_t msp430_cmpa_read  (uint16_t addr);
void   msp430_cmpa_write (uint16_t addr, int8_t val);

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

#else
#define msp430_cmpa_create() do { } while (0)
#endif /* have_cmpa */
#endif
 
