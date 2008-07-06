
/**
 *  \file   msp430_intr.h
 *  \brief  MSP430 MCU interrupt
 *  \author Antoine Fraboulet
 *  \date   2006
 **/

#ifndef MSP430_INTR_H
#define MSP430_INTR_H

void  msp430_interrupt_set          (uint16_t intr);
int   msp430_interrupt_checkifg     (void);
int   msp430_interrupt_start_if_any (void);

#endif
