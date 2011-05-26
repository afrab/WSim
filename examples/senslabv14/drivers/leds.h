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
 * \file leds.h
 * \brief LEDs macro set
 * \author ClÃ©ment Burin des Roziers <clement.burin-des-roziers@inria.fr>
 * \date November 08
 */

#ifndef _LEDS_H
#define _LEDS_H

/**********************
* Leds 
**********************/

#define LED_OUT   P5OUT

#define BIT_BLUE     (1 << 6)
#define BIT_GREEN    (1 << 5)
#define BIT_RED      (1 << 4)

/**
 * Turn the blue LED on.
 */
#define LED_BLUE_ON()      LED_OUT &= ~BIT_BLUE
/**
 * Turn the blue LED off.
 */
#define LED_BLUE_OFF()     LED_OUT |=  BIT_BLUE
/**
 * Toggle the blue LED.
 */
#define LED_BLUE_TOGGLE()  LED_OUT ^=  BIT_BLUE
/**
 * Turn the green LED on.
 */
#define LED_GREEN_ON()     LED_OUT &= ~BIT_GREEN
/**
 * Turn the green LED off.
 */
#define LED_GREEN_OFF()    LED_OUT |=  BIT_GREEN
/**
 * Toggle the green LED.
 */
#define LED_GREEN_TOGGLE() LED_OUT ^=  BIT_GREEN
/**
 * Turn the red LED on.
 */
#define LED_RED_ON()       LED_OUT &= ~BIT_RED
/**
 * Turn the red LED off.
 */
#define LED_RED_OFF()      LED_OUT |=  BIT_RED
/**
 * Toggle the red LED.
 */
#define LED_RED_TOGGLE()   LED_OUT ^=  BIT_RED

/**
 * Turn the three LEDs on.
 */
#define LEDS_ON()      LED_OUT &= ~(BIT_BLUE | BIT_GREEN | BIT_RED)
/**
 * Turn the three LEDs off.
 */
#define LEDS_OFF()     LED_OUT |=  (BIT_BLUE | BIT_GREEN | BIT_RED)

/**
 * Set the three LEDs according to a given value.
 * The value should be between 0 and 7,
 * bit0 for red LED
 * bit1 for green LED
 * bit2 for blue LED
 * \param x the value
 */
#define LEDS_SET(x) \
do {                \
  LEDS_OFF();       \
  LED_OUT &= ~((x&0x7)<<4);  \
} while(0) 

/**
 * Configure the IO pins.
 */
#define LEDS_INIT()                             \
do {                                            \
   P5OUT  &= ~(BIT_BLUE | BIT_GREEN | BIT_RED); \
   P5DIR  |=  (BIT_BLUE | BIT_GREEN | BIT_RED); \
   P5SEL  &= ~(BIT_BLUE | BIT_GREEN | BIT_RED); \
} while(0)

#endif
