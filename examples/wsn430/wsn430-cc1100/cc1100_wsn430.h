/*
 *  cc1100_wsn430.h
 *  
 *
 *  Created by Guillaume Chelius on 20/11/05.
 *  Modified by Antoine Fraboulet on 04/04/07
 *  Copyright 2005,2006,2007 __WorldSens__. All rights reserved.
 *
 */
#ifndef CC1100_WSN430_H
#define CC1100_WSN430_H

#include <io.h>
#include <signal.h>
#include <iomacros.h>


/**************************************
 * WSN430 port mapping
 **************************************/

/* Port 1 */
#define GDO0    3 /* P1.3 - Input: General Digital Output 0 (GDOO)   */
#define GDO2    4 /* P1.4 - Input: General Digital Output 2 (GDO2)   */

#define BM(x) (0x01 << x)
#define GDO0BITMASK   BM(GDO0)
#define GDO2BITMASK   BM(GDO2)

/* Port 3 */
#define SIMO 1 /* P3.1 - SPI Master out - slave in   (MOSI) */
#define SOMI 2 /* P3.2 - SPI Master in  - slave out  (MISO) */
#define SCK  3 /* P3.3 - SPI Serial Clock - slave in (SCLK) */

/* Port 4 */
#define CSN  2 /* P4.2 - SPI Chip Select             (CS_N) */


/* Enables/disables the SPI interface */
#define CC1100_SPI_ENABLE()    (P4OUT &= ~BM(CSN)) /* ENABLE CSn (active low)  */
#define CC1100_SPI_DISABLE()   (P4OUT |=  BM(CSN)) /* DISABLE CSn (active low) */

#define CC1100_HW_CHECK_MISO_HIGH() (P3IN & BM(SOMI))

/* **************************************************
 * SPI low level functions for MSP430 serial port 0
 * **************************************************/

#define NO_EINTDINT
#if defined(EINTDINT)
#define CC1100_HW_GLOBAL_EINT() if (!cc1100_intr) eint()
#define CC1100_HW_GLOBAL_DINT() if (!cc1100_intr) dint()
#else
#define CC1100_HW_GLOBAL_EINT() do { } while (0)
#define CC1100_HW_GLOBAL_DINT() do { } while (0)
#endif

#define	CC1100_SPI_WAITFOREORx() while ((IFG1 & URXIFG0) == 0)  /* USART0 Rx buffer ready? */


#define CC1100_SPI_TX(x)                    \
do {                                        \
	U0TXBUF = x;                        \
	CC1100_SPI_WAITFOREORx();           \
	cc1100_status_register = U0RXBUF;   \
} while(0)


#define CC1100_SPI_RX(x)                    \
do {                                        \
	U0TXBUF = 0;                        \
	CC1100_SPI_WAITFOREORx();           \
	x = U0RXBUF;                        \
} while(0)

/* **************************************************
 * Low level cc1100 port mapping functions 
 * **************************************************/

/* real functions prototypes */
void cc1100_wsn430_init(void);
void cc1100_wsn430_irq_packet_assert(void);
void cc1100_wsn430_irq_packet_deassert(void);

#if defined(WSN430_SPI_FUNCTION)

void cc1100_wsn430_gdo0_interrupt_on_assert(void);
void cc1100_wsn430_gdo0_interrupt_on_deassert(void);
void cc1100_wsn430_gdo0_eint(void);
void cc1100_wsn430_gdo0_dint(void);
void cc1100_wsn430_gdo2_interrupt_on_assert(void);
void cc1100_wsn430_gdo2_interrupt_on_deassert(void);
void cc1100_wsn430_gdo2_eint(void);
void cc1100_wsn430_gdo2_dint(void);

#else /* MACROS */

#define cc1100_wsn430_gdo0_interrupt_on_assert()    do { P1IES &= ~GDO0BITMASK; } while (0)
#define cc1100_wsn430_gdo0_interrupt_on_deassert()  do { P1IES |=  GDO0BITMASK; } while (0)
#define cc1100_wsn430_gdo0_eint()                   do { P1IE  |=  GDO0BITMASK; } while (0)
#define cc1100_wsn430_gdo0_dint()                   do { P1IE  &= ~GDO0BITMASK; } while (0)

#define cc1100_wsn430_gdo2_interrupt_on_assert()    do { P1IES &= ~GDO2BITMASK; } while (0)
#define cc1100_wsn430_gdo2_interrupt_on_deassert()  do { P1IES |=  GDO2BITMASK; } while (0)
#define cc1100_wsn430_gdo2_eint()                   do { P1IE  |=  GDO2BITMASK; } while (0)
#define cc1100_wsn430_gdo2_dint()                   do { P1IE  &= ~GDO2BITMASK; } while (0) 

#endif /* FUNCTIONS / MACROS */

/* **************************************************
 * Low level names to Generic names
 * **************************************************/

#define CC100_SPI_HW_INIT()              cc1100_wsn430_init()

#define CC1100_HW_GDO0_READ()            ((P1IN & GDO0BITMASK) >> GDO0)
#define CC1100_HW_GDO0_CLEAR_FLAG()      do { P1IFG &= ~(GDO0BITMASK); } while (0)

#define CC1100_HW_GDO2_READ()            ((P1IN & GDO2BITMASK) >> GDO2)
#define CC1100_HW_GDO2_CLEAR_FLAG()      do { P1IFG &= ~(GDO2BITMASK); } while (0)

#define CC1100_HW_GDO0_IRQ_ON_ASSERT()   cc1100_wsn430_gdo0_interrupt_on_assert()
#define CC1100_HW_GDO0_IRQ_ON_DEASSERT() cc1100_wsn430_gdo0_interrupt_on_deassert()
#define CC1100_HW_GDO0_EINT()            cc1100_wsn430_gdo0_eint()
#define CC1100_HW_GDO0_DINT()            cc1100_wsn430_gdo0_dint()

#define CC1100_HW_GDO2_IRQ_ON_ASSERT()   cc1100_wsn430_gdo2_interrupt_on_assert()
#define CC1100_HW_GDO2_IRQ_ON_DEASSERT() cc1100_wsn430_gdo2_interrupt_on_deassert()
#define CC1100_HW_GDO2_EINT()            cc1100_wsn430_gdo2_eint()
#define CC1100_HW_GDO2_DINT()            cc1100_wsn430_gdo2_dint()

#endif
