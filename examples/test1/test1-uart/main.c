
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
 * Timer setting
 **********************/

#define portACLK_FREQUENCY_HZ  32768
#define configTICK_RATE_HZ     1

void
set_timer()
{
  /* Ensure the timer is stopped. */
  TACTL = 0;
  /* Run the timer of the ACLK. */
  TACTL = TASSEL_1;
  /* Clear everything to start with. */
  TACTL |= TACLR;
  /* Set the compare match value according to the tick rate we want. */
  TACCR0 = portACLK_FREQUENCY_HZ / configTICK_RATE_HZ;
  /* Enable the interrupts. */
  TACCTL0 = CCIE;
  /* Freq divider. */
  TACTL |= ID_0;
  /* Start up clean. */
  TACTL |= TACLR;
  /* Up mode. */
  TACTL |= MC_1;
}

/**********************
 * Leds 
 **********************/

#define LED_OUT   P1OUT

int led_state;

interrupt (TIMERA0_VECTOR) prvTickISR( void ); // __attribute__ ( ( naked ) );
interrupt (TIMERA0_VECTOR) prvTickISR( void )
{
  led_state <<= 1;
  if (led_state > 0x04)
    {
      led_state = 1;
    }
  // free leds are on bits 6, 5 and 4, port 5
  LED_OUT = (led_state << 4) | (LED_OUT & 0x8fu);
}

/**********************
 * printf 
 **********************/

int putchar(int c)
{
  return uart1_putchar(c);
}

/**********************
 * main
 **********************/

int main(void) 
{
  int  c;

  /* leds */
  P5DIR  = 0xff;
  led_state = 1;

  BCSCTL1 = DIVA0|RSEL2|RSEL0;
  BCSCTL2 = SELM_2|SELS|DIVS_1;

  set_timer();
  uart1_init();
  eint();                             //enable interrupts

  printf("UART Test program ready.\n");

  while (1)
    {

      while ((c = uart1_getchar()))
	{
	  printf("%c",c);
	}
    }
}
