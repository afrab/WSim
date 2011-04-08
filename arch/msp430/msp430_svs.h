/**
 *  \file   msp430_svs.h
 *  \brief  MSP430 SVS controller
 *  \author Antoine Fraboulet
 *  \date   2006
 **/

#ifndef MSP430_SVS_H
#define MSP430_SVS_H
#if defined(__msp430_have_svs_at_0x55)

#define SVS_START  0x55
#define SVS_END    0x55

enum svs_addr_t {
  SVSCTL   = 0x055
};

struct msp430_svs_t {
};

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void   msp430_svs_create();
void   msp430_svs_reset ();
void   msp430_svs_update();
int8_t msp430_svs_read  (uint16_t addr);
void   msp430_svs_write (uint16_t addr, int8_t val);

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */
#else
#define msp430_svs_create() do { } while (0)
#endif /* have_svs */
#endif
