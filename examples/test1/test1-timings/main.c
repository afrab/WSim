#include <io.h>
#include <signal.h>
#include <iomacros.h>
#include <stdio.h>
#include <string.h>
#include "uart.h"


/**********************
 * Delay function.
 **********************/

#define DELAY 0x100


void delay(unsigned int d) 
{
  int i,j;
  for(j=0; j < 0xff; j++)
    {
      for (i = 0; i<d; i++) 
	{
	  nop();
	  nop();
	}
    }
}

/**********************
 * printf 
 **********************/

int putchar(int c)
{
  return uart0_putchar(c);
}

/********************************************************/
/********************************************************/
/********************************************************/

#define LED_OUT   P1OUT
#define LED_DIR   P1DIR

uint8_t led_state;

/********************************************************/
/********************************************************/
/********************************************************/

#define ACLK_FREQUENCY_HZ  32768

void timerA3_start(int HZ)
{
  /* Ensure the timer is stopped. */
  TACTL = 0;
  /* Run the timer of the ACLK. */
  TACTL = TASSEL_1;
  /* Clear everything to start with. */
  TACTL |= TACLR;
  /* Set the compare match value according to the tick rate we want. */
  TACCR0 = ACLK_FREQUENCY_HZ / HZ;
  /* Enable the interrupts. */
  TACCTL0 = CCIE;
  /* Freq divider. */
  TACTL |= ID_0;
  /* Up mode. */
  TACTL |= MC_1;
}

void timerA3_stop()
{
  TACTL = 0;
}

interrupt (TIMERA0_VECTOR) TickISR( void ); // __attribute__ ( ( naked ) );
interrupt (TIMERA0_VECTOR) TickISR( void )
{
  led_state <<= 1;
  if (led_state == 0)
    {
      led_state = 1;
    }
  LED_OUT = led_state;
}

/********************************************************/
/********************************************************/
/********************************************************/

void set_mcu_speed()
{
  int i;
  //start up crystall oscillator XT2
  // DIVA_0 -> ACLK divider = 1
  // RSEL2 | RSEL0 -> resistor select
  BCSCTL1 = DIVA_0 | RSEL2 | RSEL0;
  // SELM_2 = select XT2
  // SELS   : SMCLK source XT2
  // DIVS_3 : SMCLK divider /8 == 1MHz
  BCSCTL2 = SELM_2 | SELS  | DIVS_3;

  do { 
    IFG1 &= ~OFIFG;                         //Clear OSCFault flag 
    for (i = 0xff; i > 0; i--) nop();       //Time for flag to set 
  }  while ((IFG1 & OFIFG) != 0);           //OSCFault flag still set? 
  IFG1 &= ~OFIFG;                           //Clear OSCFault flag again 
}

/********************************************************/
/********************************************************/
/********************************************************/

int main(void) 
{
  asm("nop");              // 1 cycle
  asm("mov #0,r3");        // == nop, 1 cycle
  asm("mov 0x130,r4");     // 3 cycles
  asm("dec r4");
  asm("mov 0(r4),0(r4)");  // 6 cycles 
  
  LPM4;
  return 0;
}

/********************************************************/
/********************************************************/
/********************************************************/
