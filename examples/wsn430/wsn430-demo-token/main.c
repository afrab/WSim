
#include <io.h>
#include <signal.h>
#include <iomacros.h>
#include <stdio.h>

#include "leds.h"
#include "uart1.h"
#include "clock.h"
#include "timer.h"
#include "cc1100.h"
#include "cc1100_wsn430.h"
#include "ds2411.h"

/**********************
* Interrupt handlers.
*
* LED_BLUE  : token
* LED_RED   : timer
* LED_GREEN : not used
**********************/

interrupt (PORT1_VECTOR) port1_irq_handler(void)
{
  if (P1IFG & (P1IE & GDO2BITMASK))
    {
      cc1100_interrupt_handler(CC1100_GDO2); 
    } 
  if (P1IFG & (P1IE & GDO0BITMASK))
    {
      cc1100_interrupt_handler(CC1100_GDO0); 
    }  
  P1IFG = 0;
}

/**********************/
/**********************/

volatile int red_led;
volatile int timer_wakeup;

void timer_cb(void)
{
  if (red_led)
    LED_RED_OFF;
  else
    LED_RED_ON;
  red_led = 1 - red_led;
  timer_wakeup = 1;
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
 * printf 
 **********************/

int putchar(int c)
{
  return uart1_putchar(c);
}

int get_id()
{
  ds2411_serial_number_t id; 
  switch (ds2411_init())
    {
    case DS2411_ERROR:
      printf("  ds2411_init() reports an error.\n");
      printf("  verify MCLK speed, should run at 8MHz\n");
      LPM4;
      break;
    case DS2411_SUCCESS:
      printf("  ds2411_init() ok.\n");
      break;
    }
  printf("  ds2411_get_id().\n");
  ds2411_get_id(&id);
  ds2411_print_id(&id);
  return id.fields.serial0;
}


/**********************
 * rx callback
 **********************/

volatile int cc1100_size;
volatile int cc1100_rx_received;

void rx_packet_cb(uint8_t *rx_buffer, int size)
{
  cc1100_size = size;
  cc1100_rx_received = 1;
}

/**********************
 * main
 **********************/

#define MSG_SIZE 255 

int main(void) 
{
  int myid;
  int token     = 0;
  char rx_buffer[MSG_SIZE +1];
  char tx_buffer[MSG_SIZE +1];

  timer_wakeup = 0;

  /* leds */
  LEDS_INIT();
  LEDS_OFF();

  // set MCLK on XT2 at 8MHz
  set_mcu_speed_xt2_mclk_8MHz_smclk_1MHz();

  //set_timer();
  uart1_init();
  printf("UART init.\n");

  myid = get_id();

  red_led = 0;
  timerA3_register_callback(timer_cb);
  timerA3_keep_active();
  timerA3_ACLK_start_Hz(2);        

  cc1100_init();
  printf("CC1100 init.\n");

  // enable interrupts
  eint();  

  printf("Worldsens test program, id=%d\n",myid);

  cc1100_set_rx_packet_status(CC1100_STATUS_ENABLE);
  cc1100_rx_register_buffer(rx_buffer,MSG_SIZE);
  cc1100_rx_register_rx_cb(rx_packet_cb);
  cc1100_rx_switch_mode(CC1100_RX_MODE_IRQ);

#define MAX_NODES 3

  tx_buffer[0] = (myid % MAX_NODES) + 1;

  delay(100);

  if (myid == 1)
    {
      token = 1;
      LED_BLUE_ON;
    }
  else
    {
      cc1100_rx_enter();
      printf("rx_enter node %d\n",myid);
    }

  rx_buffer[0]       = 0;
  rx_buffer[1]       = 0;
  cc1100_size        = 0;
  cc1100_rx_received = 0;

  while (1)
    {
      LPM0;

      if ((timer_wakeup == 1) && (cc1100_rx_received == 1))
	{
	  if (cc1100_size != -1)
	    {
	      /*
	      printf("recv: len=%d - rssi=%d - crc=%d - lqi=%d\n", 
		     cc1100_size,
		     rx_buffer[cc1100_size + 1],
		     rx_buffer[cc1100_size + 2] & 0x80,
		     rx_buffer[cc1100_size + 2] & 0x7F);
	      */
	    }
	  else
	    {
	      printf("recv: error\n");
	    }
	  
	  if (rx_buffer[0] == myid)
	    {
	      printf("received packet with correct id (%d)\n",rx_buffer[0]);
	      token = 1;
	      rx_buffer[0] = 0;
	      rx_buffer[1] = 0;
	      LED_BLUE_ON;
	    }
	  else
	    {
	      printf("received packet with id %d : cancel\n",rx_buffer[0]);
	    }
	  rx_buffer[0] = 0;
	  cc1100_rx_enter();
	  cc1100_rx_received = 0;
	  timer_wakeup       = 0;
	}
      else if ((timer_wakeup == 1) && (token == 1)) /* timer wakeup */
	{
	  cc1100_idle();
	  printf("sending packet for id > %d\n",tx_buffer[0]);
	  cc1100_utx(tx_buffer, 20);
	  token = 0;
	  LED_BLUE_OFF;
	  cc1100_rx_enter();
	  timer_wakeup = 0;
	}
      else if ((timer_wakeup == 1))
	{
	  /* printf("."); */
	  timer_wakeup = 0;
	}
      else
	{
	  printf("!%d%d\n",timer_wakeup, cc1100_rx_received);
	  LPM4;
	}
    }
}
