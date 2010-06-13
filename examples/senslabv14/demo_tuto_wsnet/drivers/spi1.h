/*
 * Copyright  2008-2009 INRIA/SensTools
 * 
 * <dev-team@sentools.info>
 * 
 * This software is a set of libraries designed to develop applications
 * for the WSN430 embedded hardware platform.
 * 
 * This software is governed by the CeCILL license under French law and
 * abiding by the rules of distribution of free software.  You can  use, 
 * modify and/ or redistribute the software under the terms of the CeCILL
 * license as circulated by CEA, CNRS and INRIA at the following URL
 * "http://www.cecill.info". 
 * 
 * As a counterpart to the access to the source code and  rights to copy,
 * modify and redistribute granted by the license, users are provided only
 * with a limited warranty  and the software's author,  the holder of the
 * economic rights,  and the successive licensors  have only  limited
 * liability. 
 * 
 * In this respect, the user's attention is drawn to the risks associated
 * with loading,  using,  modifying and/or developing or reproducing the
 * software by the user in light of its specific status of free software,
 * that may mean  that it is complicated to manipulate,  and  that  also
 * therefore means  that it is reserved for developers  and  experienced
 * professionals having in-depth computer knowledge. Users are therefore
 * encouraged to load and test the software's suitability as regards their
 * requirements in conditions enabling the security of their systems and/or 
 * data to be ensured and,  more generally, to use and operate it in the 
 * same conditions as regards security. 
 * 
 * The fact that you are presently reading this means that you have had
 * knowledge of the CeCILL license and that you accept its terms.
 */


/**
 * \file spi1.h
 * \brief SPI_1 macro set
 * \author ClÃ©ment Burin des Roziers <clement.burin-des-roziers@inria.fr>
 * \date November 08
 */

#ifndef SPI1_H
#define SPI1_H

static uint8_t spi1_tx_return_value;

/* Local Macros */
/**
 * wait until a byte has been received on spi port
 */
#define SPI1_WAIT_EORX() while ( (IFG2 & URXIFG1) == 0){}

/**
 * wait until a byte has been sent on spi port
 */
#define SPI1_WAIT_EOTX() while ( (IFG2 & UTXIFG1) == 0){}

#define CC1100_CS_PIN (1<<2)
#define CC2420_CS_PIN (1<<2)
#define DS1722_CS_PIN (1<<3)
#define M25P80_CS_PIN (1<<4)

#define CC1100_ENABLE()  P4OUT &= ~CC1100_CS_PIN
#define CC1100_DISABLE() P4OUT |=  CC1100_CS_PIN

#define CC2420_ENABLE()  P4OUT &= ~CC2420_CS_PIN
#define CC2420_DISABLE() P4OUT |=  CC2420_CS_PIN

#define DS1722_ENABLE()  P4OUT |=  DS1722_CS_PIN
#define DS1722_DISABLE() P4OUT &= ~DS1722_CS_PIN

#define M25P80_ENABLE()  P4OUT &= ~M25P80_CS_PIN
#define M25P80_DISABLE() P4OUT |=  M25P80_CS_PIN

/**
 * SPI on USART1 initialization procedure
 **/
#define SPI1_INIT() do \
{ \
  P5DIR  |=   (1<<1) | (1<<3); /* output for CLK and SIMO */ \
  P5DIR  &=  ~(1<<2);   /* input for SOMI */ \
  P5SEL  |=   (1<<1) | (1<<2) | (1<<3); /* SPI for all three */ \
\
  U1CTL = SWRST; /* SPI 1 software reset */ \
  U1CTL = CHAR | SYNC | MM | SWRST;  /* 8bit SPI master */ \
\
  U1TCTL = CKPH | SSEL_2 | STC;    /* clock delay, SMCLK */ \
\
  U1RCTL = 0; /* clear errors */ \
\
  U1BR0 = 0x2; /* baudrate = SMCLK/2 */ \
  U1BR1 = 0x0; \
\
  ME2 |= USPIE1; /* enable SPI module */ \
\
  IE2 &= ~(UTXIE1 | URXIE1); /* disable SPI interrupt */ \
\
  U1CTL &= ~(SWRST); /* clear reset */ \
 \
 /* CS IO pins configuration */ \
  P4SEL &= ~(CC1100_CS_PIN | DS1722_CS_PIN | M25P80_CS_PIN); \
  P4DIR |=  (CC1100_CS_PIN | DS1722_CS_PIN | M25P80_CS_PIN); \
 \
 /* disable peripherals */ \
  M25P80_DISABLE(); \
  CC1100_DISABLE(); \
  DS1722_DISABLE(); \
} while(0)

/* enable/disable macros for SPI peripherals */

/**
 * Select CC1100 on SPI bus.
 **/
#define SPI1_CC1100_ENABLE() do \
{ \
  M25P80_DISABLE(); \
  DS1722_DISABLE(); \
  CC1100_ENABLE(); \
} while (0)

/**
 * Deselect CC1100.
 **/
#define SPI1_CC1100_DISABLE() do \
{ \
  CC1100_DISABLE(); \
} while (0)


/**
 * Select CC2420 on SPI bus.
 **/
#define SPI1_CC2420_ENABLE() do \
{ \
  M25P80_DISABLE(); \
  DS1722_DISABLE(); \
  CC2420_ENABLE(); \
} while (0)

/**
 * Deselect CC2420.
 **/
#define SPI1_CC2420_DISABLE() do \
{ \
  CC2420_DISABLE(); \
} while (0)


/**
 * select DS1722
 */
#define SPI1_DS1722_ENABLE() do \
{ \
  M25P80_DISABLE(); \
  CC1100_DISABLE(); \
  DS1722_DISABLE(); \
  U1CTL |= SWRST; \
  U1TCTL &= ~(CKPH); \
  U1CTL &= ~(SWRST); \
  DS1722_ENABLE(); \
} while (0)

/**
 * deselect DS1722
 */
#define SPI1_DS1722_DISABLE() do \
{ \
  DS1722_DISABLE(); \
  U1CTL |= SWRST; \
  U1TCTL |= CKPH; \
  U1CTL &= ~(SWRST); \
} while (0)


/**
 * select M25P80
 */
#define SPI1_M25P80_ENABLE() do \
{ \
    CC1100_DISABLE(); \
    DS1722_DISABLE(); \
    M25P80_ENABLE(); \
} while (0)
/**
 * deselect M25P80
 */
#define SPI1_M25P80_DISABLE() do \
{ \
  M25P80_DISABLE(); \
} while (0)

// tx/rx procedures
/**
 * send one byte on SPI port 1
 */
#define SPI1_TX(value) do  \
{ \
  U1TXBUF = (value); \
  SPI1_WAIT_EORX(); \
  spi1_tx_return_value = U1RXBUF; \
} while (0)

/**
 * receive one byte on SPI port 1
 */
#define SPI1_RX(value) do \
{ \
  U1TXBUF = 0; \
  SPI1_WAIT_EORX(); \
  (value) = U1RXBUF; \
} while (0)

/**
 * read SOMI line
 */
#define SPI1_READ_SOMI() (P5IN & (1<<2))

#endif
