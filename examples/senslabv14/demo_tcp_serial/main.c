
#include <io.h>
#include <signal.h>
#include <iomacros.h>
#include <stdio.h>

#include "leds.h"
#include "uart0.h"
#include "clock.h"
#include "timerA.h"
#include "cc2420.h"
#include "ds2411.h"

/**********************
 * Interrupt handlers.
 *
 * LED_BLUE  : token
 * LED_RED   : timer
 * LED_GREEN : not used
 **********************/

volatile int red_led;
volatile int timer_wakeup;

uint16_t timer_cb(void)
{
  if (red_led)
    LED_RED_OFF();
  else
    LED_RED_ON();
  red_led = 1 - red_led;
  timer_wakeup = 1;

  return 0;
}

/**********************
 * Delay function.
 **********************/

#define DELAY 0x100
void delay(unsigned int d) 
{
  int i,j;
  for(j=0; j < d; j++)
    {
      for (i = 0; i < 0xff; i++) 
	{
	  nop();
	  nop();
	}
    }
}

/**********************
 * printf putchar()
 **********************/

int putchar(int c)
{
  return uart0_putchar(c);
}

uint16_t uart_cb(uint8_t c)
{
  printf("%02x\n",c);
  return 0;
}

/**********************
 * main
 **********************/

#define MSG_SIZE 255 

int main(void) 
{

  timer_wakeup = 0;

  /* leds */
  LEDS_INIT();
  LEDS_OFF();
  LEDS_ON();

  // set MCLK on XT2 at 8MHz
  set_mcu_speed_xt2_mclk_8MHz_smclk_1MHz();

  //init uart;
  uart0_init(UART0_CONFIG_1MHZ_115200);
  uart0_register_callback(uart_cb);
  printf("UART init.\n");
  
  cc2420_init();
  printf("CC2420 init.\n");  

  red_led = 0;
  timerA_init();
  timerA_register_cb(TIMERA_ALARM_CCR0, timer_cb);
  timerA_set_alarm_from_now(TIMERA_ALARM_CCR0, 1, 4496);
  timerA_start_ACLK_div(TIMERA_DIV_1);
  printf("Timer A init.\n");

  // enable interrupts
  eint();

  while (1)
    {
      LPM0;
    }
}
