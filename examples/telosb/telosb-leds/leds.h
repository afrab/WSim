#ifndef _LED_H
#define _LED_H

/**********************
* Leds 
**********************/

#define LED_OUT   P5OUT

#define BIT_BLUE     (1 << 6)
#define BIT_GREEN    (1 << 5)
#define BIT_RED      (1 << 4)

#define LED_BLUE_ON    LED_OUT &= ~BIT_BLUE
#define LED_BLUE_OFF   LED_OUT |=  BIT_BLUE
#define LED_GREEN_ON   LED_OUT &= ~BIT_GREEN
#define LED_GREEN_OFF  LED_OUT |=  BIT_GREEN
#define LED_RED_ON     LED_OUT &= ~BIT_RED
#define LED_RED_OFF    LED_OUT |=  BIT_RED

#define LEDS_OFF()     LED_OUT |=  (BIT_BLUE | BIT_GREEN | BIT_RED)
#define LEDS_ON()      LED_OUT &= ~(BIT_BLUE | BIT_GREEN | BIT_RED)

#define LEDS_INIT()                             \
do {                                            \
   P5OUT  &= ~(BIT_BLUE | BIT_GREEN | BIT_RED); \
   P5DIR  |=  (BIT_BLUE | BIT_GREEN | BIT_RED); \
   P5SEL  &= ~(BIT_BLUE | BIT_GREEN | BIT_RED); \
} while(0)

#endif
