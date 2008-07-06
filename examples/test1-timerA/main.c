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
  // DIVS_1 : SMCLK divider /2 
  BCSCTL2 = SELM_2 | SELS  | DIVS_1;

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
  int  c;

  uart0_init();
  set_mcu_speed();
  eint();

  led_state = 1;
  LED_DIR = 0xff;
  LED_OUT = 0xff;

  printf("Timer test program ready.\n");
  printf("10Hz\n");
  timerA3_start(20);
  delay(4000);
  timerA3_stop();
  timerA3_start(10);

  while (1)
    {
      LPM3;
      while ((c = uart0_getchar()))
	{
	  printf("%c",c);
	}
    }

  return 0;
}
