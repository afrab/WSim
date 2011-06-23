/**
 *  \file   main.c
 *  \brief  eZ430-RF2500 : serial registry reader (Hex values)
 *  \author Antoine Fraboulet
 *  \date   2011
 **/

#include <msp430x22x4.h>

#if defined(__GNUC__) && defined(__MSP430__)
/* This is the MSPGCC compiler */
#include <io.h>
#include <iomacros.h>
#include <signal.h>
#elif defined(__IAR_SYSTEMS_ICC__)
/* This is the IAR compiler */
//#include <io430.h>
#endif

#include <stdio.h>

#include "leds.h"
#include "clock.h"
#include "watchdog.h"
#include "uart.h"
#include "lpm_compat.h"

#define ID 1

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

#define SERIAL_RX_FIFO_SIZE 8

volatile uint8_t serial_rx_buffer[SERIAL_RX_FIFO_SIZE];
volatile uint8_t serial_rx_rptr;
volatile uint8_t serial_rx_wptr;
volatile uint8_t serial_rx_size;

/* ************************************************** */

void serial_ring_init()
{
  serial_rx_rptr = 0;
  serial_rx_wptr = 0;
  serial_rx_size = 0;
}

void serial_ring_put(uint8_t data)
{
  serial_rx_buffer[serial_rx_wptr] = data;
  serial_rx_wptr = (serial_rx_wptr + 1) % SERIAL_RX_FIFO_SIZE;
  if (serial_rx_size < SERIAL_RX_FIFO_SIZE)
    {
      serial_rx_size ++;
    }
  else
    {
      /* 
       * if (serial_rx_size == SERIAL_RX_FIFO_SIZE) 
       * we get a rx_overflow 
      */
    }
}

int serial_ring_get(uint8_t *data)
{
  if (serial_rx_size > 0)
    {
      *data = serial_rx_buffer[serial_rx_rptr];
      serial_rx_rptr = (serial_rx_rptr + 1) % SERIAL_RX_FIFO_SIZE;
      serial_rx_size --;
      return 1;
    }
  return 0;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

int serial_cb(unsigned char data)
{
  serial_ring_put(data);
  return 1; /* will wakeup from LPMx */
}

int main(void)
{
  uint16_t addr = 0;
  uint8_t  val, data;

  watchdog_stop();
  
 set_mcu_speed_dco_mclk_16MHz_smclk_8MHz();
  
  leds_init();
  led_red_on();
  
  uart_init(UART_9600_SMCLK_8MHZ);
  serial_ring_init();
  uart_register_cb( serial_cb);
  
  printf("serial read hex\n");
  led_green_on();
  eint();
  
  printf("address: \n");
  for(;;)
    {

      LPM(1);
      
      if (serial_ring_get(&data))
	{
	  // echo
	  printf("%c", data);

	  //
	  if (data == '\r')
	    {
	      led_green_switch();

	      val  = *((uint8_t*)addr);
	      printf("  [0x%04x] = 0x%02x\n",addr,val);
	      addr = 0;
	    }
	  else
	    {
	      //hex adress with 4 digit
	      addr <<= 4;

	      if ((data >= '0') && (data <= '9'))
		{
		  addr += data - '0';
		}
	      else if ((data >= 'a') && (data <= 'f'))
		{
		  addr += data - 'a' + 10;
		}
	      else if ((data >= 'A') && (data <= 'F'))
		{
		  addr += data - 'A' + 10;
		}
	      else
		{
		  printf("  bad char, address reset\n");
		  addr = 0;
		}
	    }
	}
      else
	{
	  led_red_switch();
	}
    }
}

/* ************************************************** */
/* ************************************************** */
 
