#include <io.h>
#include <signal.h>
#include <iomacros.h>
#include <stdio.h>

#include "leds.h"

/**********************
 * Delay function.
 **********************/

#define DELAY 0x800

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
 * Leds 
 **********************/

int led_state;

void led_change( void )
{
  LEDS_OFF();
  switch (led_state)
    {
    case 0: LED_RED_ON;   break;
    case 1: LED_GREEN_ON; break;
    case 2: LED_BLUE_ON;  break;
    case 3: LEDS_ON();    break;
    }
  led_state = (led_state + 1) & 0x3;
}

/**********************
 *
 **********************/
int i = 0;

int main(void) 
{
  int i;

  WDTCTL = WDTPW + WDTHOLD;

  P1IE   = 0x00;        // Interrupt enable
  P2IE   = 0x00;        // 0:disable 1:enable

  LEDS_INIT();
  LEDS_ON();
  delay(DELAY);
  LEDS_OFF();

  led_state = 0;

  while (1) 
    {                         
      for (i=0; i<99; i++)
	{
	  led_change();
	  delay(DELAY >> 2);
	}
    }
}

