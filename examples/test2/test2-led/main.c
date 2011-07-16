/*
see README.txt for details.

chris <cliechti@gmx.net>
*/
#include "hardware.h"

/**
Delay function.
*/
void delay(unsigned int d) {
   int i;
   for (i = 0; i<d; i++) {
      nop();
      nop();
   }
}

/**
Main function with some blinking leds
*/
int main(void) {
    int i;
    int o = 0;

    WDTCTL = WDTPW + WDTHOLD;

    P1OUT  = P1OUT_INIT;                //Init output data of port1
    P2OUT  = P2OUT_INIT;                //Init output data of port2

    P1SEL  = P1SEL_INIT;                //Select port or module -function on port1
    P2SEL  = P2SEL_INIT;                //Select port or module -function on port2

    P1DIR  = P1DIR_INIT;                //Init port direction register of port1
    P2DIR  = P2DIR_INIT;                //Init port direction register of port2

    P1IES  = P1IES_INIT;                //init port interrupts
    P2IES  = P2IES_INIT;
    P1IE   = P1IE_INIT;
    P2IE   = P2IE_INIT;

    while (1) {                         //main loop, never ends...
        for (i=0; i<8; i++, o++) {
            P1OUT = (1<<i) | (0x80>>(o&7));
            delay(0xfff);
        }
    }
}

