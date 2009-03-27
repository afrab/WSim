/**
 *  \file   leds.h
 *  \brief  test2-spi
 *  \author Antoine Fraboulet
 *  \date   2009
 **/

#ifndef LEDS_H
#define LEDS_H

/* ************************************************** */
/* Leds                                               */
/* ************************************************** */

void LEDS_INIT     (void);
void LEDS_ON       (void);
void LEDS_OFF      (void);
void LEDS_BLINK    (void); /* endless loop */

void LED_ON        (int);
void LED_OFF       (int);
void LED_SWITCH    (int);

/* ************************************************** */
/*                                                    */
/* ************************************************** */

#endif
