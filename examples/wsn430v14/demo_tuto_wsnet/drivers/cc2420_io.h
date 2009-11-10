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
 * \file cc2420_io.h
 * \brief CC2420 IO PIN hardware abstraction
 * \author Clément Burin des Roziers <clement.burin-des-roziers@inria.fr>
 * \date January 09
 */

#ifndef _CC2420_IO_H
#define _CC2420_IO_H

#define FIFO_PIN    (1<<3)
#define FIFOP_PIN   (1<<4)
#define SFD_PIN     (1<<5)
#define CCA_PIN     (1<<6)
#define RESETn_PIN  (1<<7)

#define VREGEN_PIN  (1<<0)

/**
 * Initialize IO PORTs for CC 2420 connectivity
 **/
#define CC2420_IO_INIT() do \
{ \
    P1SEL &= ~(FIFOP_PIN | FIFO_PIN | SFD_PIN | CCA_PIN | RESETn_PIN); \
    P1DIR &= ~(FIFOP_PIN | FIFO_PIN | SFD_PIN | CCA_PIN); \
    P1DIR |= RESETn_PIN; \
    P1OUT &= ~RESETn_PIN; \
    P1IE  &= ~(FIFOP_PIN | FIFO_PIN | SFD_PIN | CCA_PIN | RESETn_PIN); \
    \
    P3SEL &= ~VREGEN_PIN; \
    P3DIR |= VREGEN_PIN; \
    P3OUT &= ~VREGEN_PIN; \
} while (0)


#define RESETn_SET(val) do \
{ \
    if (val) P1OUT |= RESETn_PIN; \
    else P1OUT &= ~RESETn_PIN; \
} while (0)

#define VREGEN_SET(val) do \
{ \
    if (val) P3OUT |= VREGEN_PIN; \
    else P3OUT &= ~VREGEN_PIN; \
} while (0)


/**
 * Enable Interrupt for FIFOP pin
 **/
#define FIFOP_INT_ENABLE() P1IE |= FIFOP_PIN
/**
 * Enable Interrupt for FIFO pin
 **/
#define FIFO_INT_ENABLE() P1IE |= FIFO_PIN
/**
 * Enable Interrupt for SFD pin
 **/
#define SFD_INT_ENABLE() P1IE |= SFD_PIN
/**
 * Enable Interrupt for CCA pin
 **/
#define CCA_INT_ENABLE() P1IE |= CCA_PIN


/**
 * Disable Interrupt for FIFOP pin
 **/
#define FIFOP_INT_DISABLE() P1IE &= ~FIFOP_PIN
/**
 * Disable Interrupt for FIFO pin
 **/
#define FIFO_INT_DISABLE() P1IE &= ~FIFO_PIN
/**
 * Disable Interrupt for SFD pin
 **/
#define SFD_INT_DISABLE() P1IE &= ~SFD_PIN
/**
 * Disable Interrupt for CCA pin
 **/
#define CCA_INT_DISABLE() P1IE &= ~CCA_PIN


/**
 * Clear interrupt flag for FIFOP pin
 **/
#define FIFOP_INT_CLEAR() P1IFG &= ~FIFOP_PIN
/**
 * Clear interrupt flag for FIFO pin
 **/
#define FIFO_INT_CLEAR() P1IFG &= ~FIFO_PIN
/**
 * Clear interrupt flag for SFD pin
 **/
#define SFD_INT_CLEAR() P1IFG &= ~SFD_PIN
/**
 * Clear interrupt flag for CCA pin
 **/
#define CCA_INT_CLEAR() P1IFG &= ~CCA_PIN



/**
 * Set interrupt on rising edge for FIFOP pin
 **/
#define FIFOP_INT_SET_RISING()  P1IES &= ~FIFOP_PIN
/**
 * Set interrupt on rising edge for FIFO pin
 **/
#define FIFO_INT_SET_RISING()  P1IES &= ~FIFO_PIN
/**
 * Set interrupt on rising edge for SFD pin
 **/
#define SFD_INT_SET_RISING()  P1IES &= ~SFD_PIN
/**
 * Set interrupt on rising edge for CCA pin
 **/
#define CCA_INT_SET_RISING()  P1IES &= ~CCA_PIN


/**
 * Set interrupt on falling edge for FIFOP pin
 **/
#define FIFOP_INT_SET_FALLING()  P1IES |= FIFOP_PIN
/**
 * Set interrupt on falling edge for FIFO pin
 **/
#define FIFO_INT_SET_FALLING()  P1IES |= FIFO_PIN
/**
 * Set interrupt on falling edge for SFD pin
 **/
#define SFD_INT_SET_FALLING()  P1IES |= SFD_PIN
/**
 * Set interrupt on falling edge for CCA pin
 **/
#define CCA_INT_SET_FALLING()  P1IES |= CCA_PIN



/**
 * Read FIFOP pin value
 **/
#define FIFOP_READ() (P1IN & FIFOP_PIN)
/**
 * Read FIFO pin value
 **/
#define FIFO_READ() (P1IN & FIFO_PIN)
/**
 * Read SFD pin value
 **/
#define SFD_READ() (P1IN & SFD_PIN)
/**
 * Read CCA pin value
 **/
#define CCA_READ() (P1IN & CCA_PIN)

#endif
