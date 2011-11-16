/**
see README.txt for details.

Main application.

http://mspgcc.sf.net
chris <cliechti@gmx.net>
*/
#include <stdio.h>
#include "hardware.h"
#include "taskhandler.h"


/**
dummy task for unused tasktable entries.
*/
void foo(void) {
}

/**
Main function with init and an endless loop that is synced with the
interrupts trough the lowpower mode (taskhandler).
*/
int main(void) {
    unsigned short i;
    WDTCTL = WDTCTL_INIT;               //Init watchdog timer

    P1OUT  = P1OUT_INIT;                //Init output data of port1
    P1SEL  = P1SEL_INIT;                //Select port or module -function on port1
    P1DIR  = P1DIR_INIT;                //Init port direction register of port1
    P1IES  = P1IES_INIT;                //init port interrupts
    P1IE   = P1IE_INIT;

    P2OUT  = P2OUT_INIT;                //Init output data of port2
    P2SEL  = P2SEL_INIT;                //Select port or module -function on port2
    P2DIR  = P2DIR_INIT;                //Init port direction register of port2
    P2IES  = P2IES_INIT;                //init port interrupts
    P2IE   = P2IE_INIT;

#ifdef __MSP430_HAS_PORT3__
    P3OUT  = P3OUT_INIT;                //Init output data of port3
    P3SEL  = P3SEL_INIT;                //Select port or module -function on port3
    P3DIR  = P3DIR_INIT;                //Init port direction register of port3
#endif
#ifdef __MSP430_HAS_PORT4__
    P4OUT  = P4OUT_INIT;                //Init output data of port4
    P4SEL  = P4SEL_INIT;                //Init port or module -function on port4
    P4DIR  = P4DIR_INIT;                //Init port direction register of port4
#endif
#ifdef __MSP430_HAS_PORT5__
    P5OUT  = P5OUT_INIT;                //Init output data of port5
    P5SEL  = P5SEL_INIT;                //Init port or module -function on port5
    P5DIR  = P5DIR_INIT;                //Init port direction register of port5
#endif
#ifdef __MSP430_HAS_PORT6__
    P6OUT  = P6OUT_INIT;                //Init output data of port6
    P6DIR  = P6DIR_INIT;                //Init port direction register of port6
    P6SEL  = P6SEL_INIT;                //Init port or module -function on port6
#endif

    //    IE1    = IE1_INIT;
    //    IE2    = IE2_INIT;
    //    ME1    = ME1_INIT;
    //    ME2    = ME2_INIT;

    P2OUT |= BIT0;                       //light LED during init
    
    //start up crystall oscillator XT2
    BCSCTL1 = BCSCTL1_INIT;
    BCSCTL2 = BCSCTL2_INIT;
    
    do { 
        IFG1 &= ~OFIFG;                         //Clear OSCFault flag 
        for (i = 0xff; i > 0; i--) nop();       //Time for flag to set 
    }  while ((IFG1 & OFIFG) != 0);             //OSCFault flag still set? 
    IFG1 &= ~OFIFG;                             //Clear OSCFault flag again 
    
    P2OUT &= ~BIT0;                     //light LED during init
    
    //Init of USART0 Module
    U0ME  |= UTXE0|URXE0;               //Enable USART0 transmiter and receiver (UART mode)

    U0CTL  = SWRST;                     //reset
    U0CTL  = U0CTL_INIT;                //init & release reset
    
    U0TCTL = U0TCTL_INIT;
    U0RCTL = U0RCTL_INIT;
    
    U0BR1  = U0BR1_INIT;
    U0BR0  = U0BR0_INIT;
    U0MCTL = U0MCTL_INIT;

    U0IE  |= URXIE0;                    //Enable USART0 receive interrupts (UART mode)
    
    eint();                             //enable interrupts
    
    printf("UART Test program ready.\n");
    
    taskhandler(0);                     //enter the taskhandler
}

