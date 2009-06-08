#include <io.h>
#include <signal.h>
#include <iomacros.h>
#include <stdio.h>

#include "leds.h"

/**********************
 * Leds 
 **********************/

volatile int led_state;

void led_change( void )
{
  switch (led_state)
    {
    case 0: LED_ON(); led_state = 1;  break;
    case 1: LED_OFF(); led_state = 0; break;
    }
}

/* ************************************************** */
/* * Timer ****************************************** */
/* ************************************************** */

#define TA_ACLK_FREQ_HZ  32768
#define TA_TICK_RATE_HZ    10

void set_timer(int i)
{
  /* Ensure the timer is stopped. */
  TACTL = 0;
  /* Clear everything to start with. */
  TACTL |= TACLR;
  /* Run the timer of the ACLK. */
  TACTL = TASSEL_1;
  /* Set the compare match value according to the tick rate we want. */
  TACCR0 = TA_ACLK_FREQ_HZ / i;
  /* Enable the interrupts. */
  TACCTL0 = CCIE;

  /* ID_0                (0<<6)  Timer A input divider: 0 - /1 */
  /* ID_1                (1<<6)  Timer A input divider: 1 - /2 */
  /* ID_2                (2<<6)  Timer A input divider: 2 - /4 */
  /* ID_3                (3<<6)  Timer A input divider: 3 - /8 */
  TACTL |= ID_0;

  /* Up mode. */
  TACTL |= MC_1;
}


interrupt (TIMERA0_VECTOR) prvTickISR( void )
{
  led_change();
}


/**********************
 * Delay function.
 **********************/

#define DELAY 0x400

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
 *
 **********************/

int a = 0;

int main(void) 
{
  int i;

  P1IE   = 0x00;        // Interrupt enable
  P2IE   = 0x00;        // 0:disable 1:enable

  LED_INIT();

  LED_ON();
  delay(200);
  LED_OFF();
  delay(200);
  LED_ON();
  delay(200);
  LED_OFF();
  delay(200);
  LED_ON();
  delay(200);
  LED_OFF();

  led_state = 0;
  set_timer(1);
  eint();

  while (1) 
    {      
      for(i=2; i<20; i++)
	{
	  /* flashing LEDS */
	  set_timer(i);
	  LED_ON();
	  delay(200);
	  LED_OFF();
	}
    }
}

