#ifndef HARDWARE_H
#define HARDWARE_H

//Hardware description
//http://mspgcc.sf.net
//chris <cliechti@gmx.net>

#include <io.h>
#include <signal.h>

//PINS
//--- PORT1 ---
#define P1OUT_INIT      0
#define P1SEL_INIT      0
#define P1DIR_INIT      0xff

#define P1IE_INIT       0
#define P1IES_INIT      0

//--- PORT2 ---
#define LEDRT           BIT1

#define P2OUT_INIT      0
#define P2SEL_INIT      0
#define P2DIR_INIT      0xff

#define P2IE_INIT       0
#define P2IES_INIT      0

//--- PORT3 ---
#define CTS             BIT2
#define RTS             BIT3
#define TX              BIT4
#define RX              BIT5

#define P3OUT_INIT      RTS
#define P3SEL_INIT      TX|RX
#define P3DIR_INIT      TX|RTS

//--- PORT4 ---
#define P4OUT_INIT      0
#define P4SEL_INIT      0
#define P4DIR_INIT      0

//--- PORT5 ---
#define P5OUT_INIT      0
#define P5SEL_INIT      0
#define P5DIR_INIT      0

//--- PORT6 ---
#define P6OUT_INIT      0
#define P6SEL_INIT      0
#define P6DIR_INIT      0

////
#define IE1_INIT        0
#define IE2_INIT        0
#define ME1_INIT        0
#define ME2_INIT        0


#define WDTCTL_INIT     WDTPW|WDTHOLD

#define BCSCTL1_INIT    DIVA0|RSEL2|RSEL0
#define BCSCTL2_INIT    SELM_2|SELS|DIVS_1

#define U0CTL_INIT      CHAR
#define U0TCTL_INIT     SSEL_SMCLK|TXEPT     //use SMCLK
#define U0RCTL_INIT     0

//~ //115200 @4MHz
//~ #define U0BR1_INIT      0               //Baud rate 1 register init 'U0BR1' 
//~ #define U0BR0_INIT      0x22            //Baud rate 0 register init 'U0BR0'
//~ #define U0MCTL_INIT     0xdd            //Modulation Control Register init 'U0MCTL':

//9600 @4MHz
#define U0BR1_INIT      0x01
#define U0BR0_INIT      0xA0
#define U0MCTL_INIT     0x5B

#endif //HARDWARE_H
