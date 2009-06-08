/*
 *  cc1100.h
 *  
 *
 *  Created by Guillaume Chelius on 20/11/05.
 *  Modified by Antoine Fraboulet on 04/04/07
 *  Copyright 2005,2006,2007 __WorldSens__. All rights reserved.
 *
 */

#include "cc1100_wsn430.h"
#include "cc1100_spi.h"
#include "cc1100.h"


/* Initialize MSP430 - CC1100 connection */
void cc1100_wsn430_init(void) 
{
  cc1100_status_register = 0;
  
  /* Set PIN directions, funtionnality and interruptions */
  P1SEL &= ~(             BM(GDO0) | BM(GDO2)); // Select I/O functionality (GDO0, GDO2)
  P1DIR &= ~(             BM(GDO0) | BM(GDO2)); // Configure as inputs(GDO0, GDO2)
  P1IE  |=  (                        BM(GDO2)); // Enable interrupts on GDO2
  P1IE  &= ~(             BM(GDO0)           ); // Disable interrupts on GDO0
  P1IES &= ~(             BM(GDO0) | BM(GDO2)); // Interrupts on low-to-high transitions

  P3SEL |= (BM(SIMO) | BM(SCK) | BM(SOMI)); // Select Peripheral functionality (SPI: SIMO, SOI, SCK)
  P3DIR |= (BM(SIMO) | BM(SCK));            // Configure as outputs(SIMO,SCK)
  P3DIR &= ~BM(SOMI);                       // Configure as inputs(SOMI)
  
  P4SEL &= ~BM(CSN); // Select I/O functionality (CSn)
  P4DIR |=  BM(CSN); // Configure as outputs(CSn)
  
  /* Perform SPI module initialization */
  U0CTL  = CHAR + SYNC + MM + SWRST; // SPI mode, 8-bit transfer, Listen disabled, SPI master, SW reset (c[2]p14-14)
  U0TCTL = CKPH + SSEL1 + STC; // Data on Rising Edge, SMCLK, 3-wire SPI mode (c[2]p14-15)
  
  U0BR0  = 0x04; // SPICLK set baud (c[2]p14-17)
  U0BR1  = 0; // Dont need baud rate control register 2 - clear it (c[2]p14-15)
  U0MCTL = 0; // Dont need modulation control (c[2]p14-15)
  
  ME1 |= USPIE0; // Module 0 enabled (c[2]p14-19)
  
  U0CTL &= ~SWRST; // Remove RESET (c[2]p14-14)
  IE1 &= ~(UTXIE0); // no interrupt on SPI tx, active polling on U0TCTL.TXEPT
  IE1 &= ~(URXIE0); // toverify, active polling on IFG2.URXIFG0

  /* packet receive interrupt : software generated IRQ on P2.0 */
  P2SEL &= ~ 1; // io mode
  P2DIR &= ~ 1; // input
  P2IE  |=   1; // enable interrupt
  P2IES &= ~ 1; // low to high
  
  return;
}


#if defined(WSN430_SPI_FUNCTION)

void cc1100_wsn430_gdo0_interrupt_on_assert(void) 
{
  P1IES &= ~(BM(GDO0));
}

void cc1100_wsn430_gdo0_interrupt_on_deassert(void) 
{
  P1IES |= BM(GDO0);
}

void cc1100_wsn430_gdo0_eint(void)
{
  P1IE |= BM(GDO0);
}

void cc1100_wsn430_gdo0_dint(void)
{
  P1IE &= ~(BM(GDO0));
}



void cc1100_wsn430_gdo2_interrupt_on_assert(void) 
{
  P1IES &= ~(BM(GDO2));
}

void cc1100_wsn430_gdo2_interrupt_on_deassert(void) 
{
  P1IES |= BM(GDO2);
}

void cc1100_wsn430_gdo2_eint(void)
{
  P1IE |= BM(GDO2);
}

void cc1100_wsn430_gdo2_dint(void)
{
  P1IE &= ~(BM(GDO2));
}

#endif
