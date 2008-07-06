#ifndef _LED_H
#define _LED_H

/**********************
* Leds 
**********************/

#define LED_OUT   P1OUT


#define LED_OFF()     LED_OUT &= ~0x01 
#define LED_ON()      LED_OUT |= 0x01

#define LED_INIT()  \
do {                \
   P1OUT  |=  0x01; \
   P1DIR  |=  0x01; \
} while(0)


#endif
