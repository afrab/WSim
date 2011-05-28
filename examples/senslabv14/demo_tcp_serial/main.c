
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


/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

volatile int green_led;

uint16_t timer_cb(void)
{
  if (green_led)
    LED_GREEN_OFF();
  else
    LED_GREEN_ON();
  green_led = 1 - green_led;

  return 0; /* 1 == wakeup */
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

#define SERIAL_RX_FIFO_SIZE 0xFF

volatile uint8_t serial_rx_buffer[SERIAL_RX_FIFO_SIZE];
volatile uint8_t serial_rx_rptr;
volatile uint8_t serial_rx_wptr;
volatile uint8_t serial_rx_size;

/* ************************************************** */

void serial_rx_buffer_init()
{
  serial_rx_rptr = 0;
  serial_rx_wptr = 0;
  serial_rx_size = 0;
}

/* ************************************************** */

void serial_rx_buffer_put(uint8_t data) 
{
  serial_rx_buffer[serial_rx_wptr] = data;
  serial_rx_wptr = (serial_rx_wptr + 1) % SERIAL_RX_FIFO_SIZE;
  if (serial_rx_size < SERIAL_RX_FIFO_SIZE)
    {
      serial_rx_size ++;
    }
  else
    {
      LED_RED_ON();
    }
}

/* ************************************************** */

int serial_rx_buffer_get(uint8_t *data)
{
  dint();
  if (serial_rx_size > 0)
    {
      *data = serial_rx_buffer[serial_rx_rptr];
      serial_rx_rptr = (serial_rx_rptr + 1) % SERIAL_RX_FIFO_SIZE;
      serial_rx_size --;
      eint();        
      return 1;
    }
  eint();        
  return 0;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

int putchar(int c)
{
  return uart0_putchar(c);
}

uint16_t uart_cb(uint8_t c)
{
  serial_rx_buffer_put(c);
  return 1; /* wake up */
}


/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

/*
 * Serial port using 8 bit configuration
 * 115200 bauds = 14400 bytes/s = 1 byte every 69.4µs
 *  38400 bauds =  4800 bytes/s = 1 byte every 208.3µs
 *   9600 bauds =  1200 bytes/s = 1 byte every 833.3µs
 *
 * VCD traces 
 *  115200 =  63.5 µs
 *   38400 = 208.0 µs
 *    9600 = 831.8 µs
 */

int main(void) 
{
  int res;
  uint8_t data;

  /* leds */
  LEDS_INIT();
  LEDS_OFF();
  green_led = 0;

  // set MCLK on XT2 at 8MHz
  set_mcu_speed_xt2_mclk_8MHz_smclk_1MHz();

  //init uart;
  uart0_init(UART0_CONFIG_1MHZ_115200);
  uart0_register_callback(uart_cb);
  printf("UART init.\n");
  
  cc2420_init();
  printf("CC2420 init.\n");  

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

      while ((res = serial_rx_buffer_get( &data )) != 0)
	{
	  putchar(data);
	}
    }
}
