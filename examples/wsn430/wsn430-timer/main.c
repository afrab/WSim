#include <io.h>
#include <signal.h>
#include <iomacros.h>
#include <stdio.h>

#include "leds.h"
#include "clock.h"
#include "uart1.h"
#include "timer.h"

/**********************
 * Delay function.
 **********************/

#define DELAY 0x4000

void delay(unsigned int d) 
{
  unsigned int i,j;
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

int putchar(int c)
{
  return uart1_putchar(c);
}

/**********************
 *
 **********************/

void red_led()
{
  static char led_state = 0;
  if (led_state)
    LED_RED_OFF;
  else
    LED_RED_ON;
  led_state = 1 - led_state;
}

/**********************
 *
 **********************/

void blue_led()
{
  static char led_state = 0;
  if (led_state)
    LED_BLUE_OFF;
  else
    LED_BLUE_ON;
  led_state = 1 - led_state;
}

/**********************
 *
 **********************/

int main(void) 
{

  WDTCTL = WDTPW + WDTHOLD;
  
  P1IE   = 0x00;        // Interrupt enable
  P2IE   = 0x00;        // 0:disable 1:enable

  LEDS_INIT();
  LEDS_ON();
  delay(DELAY >> 4);
  LEDS_OFF();

  set_mcu_speed_xt2_mclk_8MHz_smclk_1MHz();
  LED_BLUE_ON;
  delay(DELAY >> 4);
  LED_BLUE_OFF;

  uart1_init();
  LED_RED_ON;
  delay(DELAY >> 4);
  LED_RED_OFF;

  printf("wsn430-timer test ready\n");
  printf("timerA3 register 0 = red / ACLK @ 32KHz\n");
  printf("timerB5 register 0 = blue / SMCLK @ 1MHz\n");

  timerA3_register_callback(red_led);
  timerB7_register_callback(blue_led);

  eint();
  timerB7_SMCLK_start_Hz(5);

  while (1) 
    {       
      timerA3_ACLK_start_Hz(5);
      delay(DELAY);

      timerA3_ACLK_start_Hz(10);
      delay(DELAY);

      timerA3_ACLK_start_Hz(2);
      delay(DELAY);

      timerB7_SMCLK_start_Hz(5);
      delay(DELAY);

      timerB7_SMCLK_start_Hz(20);
      delay(DELAY);

      timerB7_SMCLK_start_Hz(2);
      delay(DELAY);
    }
}

