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

int putchar(int c)
{
  return uart1_putchar(c);
}


volatile int red_led;
void timer_cb(void)
{
  if (red_led)
    LED_RED_OFF;
  else
    LED_RED_ON;
  red_led = 1 - red_led;
}

/**********************
 *
 **********************/
char gcc_wants_me = 0;

int main(void) 
{
  int c;

  WDTCTL = WDTPW + WDTHOLD;

  P1IE   = 0x00;        // Interrupt enable
  P2IE   = 0x00;        // 0:disable 1:enable

  LEDS_INIT();
  LEDS_ON();
  delay(DELAY >> 4);
  LEDS_OFF();

  set_mcu_speed_xt2_mclk_8MHz_smclk_1MHz();
  LED_BLUE_ON;
  delay(DELAY);
  LED_BLUE_OFF;

  uart1_init();
  LED_RED_ON;
  delay(DELAY);
  LED_RED_OFF;

  timerA3_register_callback(timer_cb);
  timerA3_keep_active();
  timerA3_ACLK_start_Hz(1);
  eint();

  printf("Worldsens serial console test\n");
  printf("wsn430-serial test ready\n");

  while (1) 
    {       
      printf("\n1: test polling on serial1 with echo\n");
      printf("   press 'Z' to swith to IRQ mode\n");
      while ((c = uart1_getchar_polling()) != 'Z')
	{
	  putchar(c);
	}

      printf("\n2: test IRQ + LPM0 on serial1 with echo\n");
      printf("   press 'Z' to swith to polling mode\n");

      uart1_eint_rx();
      c = 0;
      
      while (1)
	{
	  if (uart1_getchar(&c))
	    {
	      putchar(c);
	    }

	  if (c == 'Z')
	    break;

	  LPM0;
	}
      uart1_dint();

    }
}

