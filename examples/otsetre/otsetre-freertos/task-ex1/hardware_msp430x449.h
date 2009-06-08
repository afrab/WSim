#ifndef HARDWARE_H
#define HARDWARE_H

#include <io.h>
#include <signal.h>
#include <iomacros.h>

//Port Output Register 'P1OUT, P2OUT':
#define P1OUT_INIT      0                       // Init Output data of port1
#define P2OUT_INIT      0                       // Init Output data of port2

//Port Direction Register 'P1DIR, P2DIR':
#define P1DIR_INIT      0xff                    // Init of Port1 Data-Direction Reg (Out=1 / Inp=0)
#define P2DIR_INIT      0xff                    // Init of Port2 Data-Direction Reg (Out=1 / Inp=0)

//Selection of Port or Module -Function on the Pins 'P1SEL, P2SEL'
#define P1SEL_INIT      0                       // P1-Modules:
#define P2SEL_INIT      0                       // P2-Modules:

//Interrupt capabilities of P1 and P2
#define P1IE_INIT       0                       // Interrupt Enable (0=dis 1=enabled)
#define P2IE_INIT       0                       // Interrupt Enable (0=dis 1=enabled)
#define P1IES_INIT      0                       // Interrupt Edge Select (0=pos 1=neg)
#define P2IES_INIT      0                       // Interrupt Edge Select (0=pos 1=neg)

#define WDTCTL_INIT     WDTPW|WDTHOLD

#endif //HARDWARE_H
