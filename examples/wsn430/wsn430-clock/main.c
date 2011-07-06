#include <io.h>
#include <signal.h>
#include <iomacros.h>
#include <stdio.h>

#include "leds.h"
#include "clock.h"

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

/* adjust timing loop according to wait loop */

// 8MHz -> *2
// 4MHz -> *1

#define MICRO  *2
#define MILLI  *1


static void __inline__ 
micro_wait(register unsigned int n)
{
  /* MCLK is running 8MHz, 1 cycle = 125ns    */
  /* n=1 -> waiting = 4*125ns = 500ns         */

  /* MCLK is running 4MHz, 1 cycle = 250ns    */
  /* n=1 -> waiting = 4*250ns = 1000ns        */

    __asm__ __volatile__ (
		"1: \n"
		" dec	%[n] \n" /* 1 cycle  */
		" nop        \n" /* 1 cycle  */
		" jne	1b   \n" /* 2 cycles */
        : [n] "+r"(n));
} /* micro_wait */


/* static void __inline__*/
void
milli_wait(register unsigned int n)
{
  int i;
  for(i=0; i<n; i++)
    micro_wait(1000 MICRO);
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

void leds_cc(void)
{
  int i;
  for (i=0; i < 5; i++)
    {
      LED_RED_OFF;
      delay(DELAY >> 2);
      LED_RED_ON;
      delay(DELAY >> 2);
    }

  LED_BLUE_ON;
  delay(DELAY);
  LED_BLUE_OFF;
}

/**********************
 *
 **********************/

int main(void) 
{
  int i;
  P1IE   = 0x00;        // Interrupt enable
  P2IE   = 0x00;        // 0:disable 1:enable

  LEDS_INIT();
  LEDS_ON();
  delay(DELAY);
  LEDS_OFF();

  led_state = 0;
  //  leds_cc();

  LED_BLUE_ON;
  set_mcu_speed_xt2_mclk_8MHz_smclk_1MHz();
  for(i=0; i<50; i++)
    {
      milli_wait(500);
      LED_BLUE_OFF;
      milli_wait(500);
      LED_BLUE_ON;
    }

  while (1) 
    {       
      set_mcu_speed_dco_mclk_4MHz_smclk_1MHz();
      leds_cc();

      set_mcu_speed_xt2_mclk_4MHz_smclk_1MHz();
      leds_cc();

      set_mcu_speed_xt2_mclk_8MHz_smclk_1MHz();
      leds_cc();

      set_mcu_speed_xt2_mclk_8MHz_smclk_8MHz();
      leds_cc();
    }
}

