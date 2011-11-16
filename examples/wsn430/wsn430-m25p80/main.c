#include <io.h>
#include <signal.h>
#include <iomacros.h>
#include <stdio.h>

#include "leds.h"
#include "clock.h"
#include "uart1.h"
#include "m25p80.h"

/**********************
 * Delay function.
 **********************/

#define DELAY 0x4000

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

/**********************
 *
 **********************/

int main(void) 
{
  unsigned char signature = 0xfa;
  unsigned char state     = 0xfb;
  
  int i;
  uint8_t buff[256];

  WDTCTL = WDTPW + WDTHOLD;

  P1IE   = 0x00;        // Interrupt enable
  P2IE   = 0x00;        // 0:disable 1:enable

  LEDS_INIT();
  LED_RED_ON;

  set_mcu_speed_xt2_mclk_8MHz_smclk_1MHz();
  signature = m25p80_init();

  LED_RED_OFF;

  state = m25p80_get_state();
  m25p80_power_down();

  uart1_init();
  printf("\nwsn430-m25p80 test ready\n");
  printf("m25p80_init      returned %x\n",signature);
  printf("m25p80_get_state returned %x\n",state);

  for(i=0; i<256; i++)
    buff[i] = i;

  signature = m25p80_init();
  m25p80_save_page(2,buff);
  
  for(i=0; i<256; i++)
    buff[i] = 0;

  m25p80_load_page(2,buff);

  uart1_init();
  printf("results\n");
  for(i=0; i<256; i++)
    {
      if (buff[i] != i)
	{
	  printf("error for index i=%d, data=%d\n",i,buff[i]);
	}
    }
  printf("test done\n");
  while (1) 
    {       
      LED_BLUE_ON;
      delay(0x1000);
      LED_BLUE_OFF;
      delay(0x1000);
    }
}

