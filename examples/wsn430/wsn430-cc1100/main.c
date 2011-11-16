
#include <io.h>
#include <signal.h>
#include <iomacros.h>
#include <stdio.h>
#include <string.h>

#include "leds.h"
#include "uart1.h"
#include "clock.h"
#include "timer.h"
#include "ds2411.h"
#include "cc1100.h"
#include "cc1100_wsn430.h"

/**********************
* Interrupt handlers.
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

/**********************
 * Delay function.
 **********************/

#define DELAY 0x100

void delay(unsigned int d) 
{
  unsigned int i,j;
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
 * main
 **********************/


#define DATA 0
#define SIZE 255
#define SEND_SIZE 60

void idle()
{
  LPM0;
}


void tx_main()
{
  int i;
  uint8_t tx_buffer[255];

  printf("Worldsens test program, CC1100ng sender\n");
  LEDS_OFF();
  timerA3_register_callback(timer_cb);
  timerA3_keep_active();
  timerA3_ACLK_start_Hz(1);

  /* Tx/Rx Fifo threshold, page 46
   * value   0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15
   * TX     61 57 53 49 45 41 37 33 29 25 21 17 13  9  5  1
   * RX      4  8 12 16 20 24 28 32 36 40 44 48 52 56 60 64
   */
  cc1100_set_fifo_threshold(10); 

  i = 0;
  while (1)
    {
      sprintf(tx_buffer,"hello world %d!",i++); /* 14 bytes = 7 int */
      printf("snd: %s\n",tx_buffer);

      LED_BLUE_ON; 
      cc1100_utx(tx_buffer,SEND_SIZE); /* check size */
      LED_BLUE_OFF; 
      LEDS_OFF();

      idle();
    }    
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void rx_packet_cb(uint8_t *rx_buffer, int size)
{
  if (size != -1)
    {
      printf("recv: %s / len=%d - rssi=%d - crc=%d - lqi=%d\n", 
	     & rx_buffer[0],
	     size,
	     rx_buffer[size + 1],
	     rx_buffer[size + 2] & 0x80,
	     rx_buffer[size + 2] & 0x7F);
    }
  else
    {
      printf("recv: error\n");
    }
  LED_BLUE_OFF;
  LED_GREEN_OFF;
  rx_buffer[0] = 0;
  cc1100_rx_enter();
}

void rx_main()
{
  char   rx_buffer[SIZE];

/* Tx/Rx Fifo threshold, page 46
 * value   0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15
 * TX     61 57 53 49 45 41 37 33 29 25 21 17 13  9  5  1
 * RX      4  8 12 16 20 24 28 32 36 40 44 48 52 56 60 64
 */
  cc1100_set_fifo_threshold(9); 

  printf("Worldsens test program, CC1100ng receiver\n");
  timerA3_register_callback(timer_cb);
  timerA3_keep_active();
  timerA3_ACLK_start_4S();

  cc1100_calibrate();
  printf("ready...\n");

  cc1100_set_rx_packet_status(CC1100_STATUS_ENABLE);
  cc1100_rx_register_buffer(rx_buffer,SIZE);
  cc1100_rx_register_rx_cb(rx_packet_cb);
  // cc1100_rx_switch_mode(CC1100_RX_MODE_POLLING);
  cc1100_rx_switch_mode(CC1100_RX_MODE_IRQ);
  cc1100_rx_enter();

  while (1)
    {
      LPM0;
    }      
}


/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

int got_root()
{
  int i;
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

  /* DS1  = 0a:00:00:00:00:00:01:01 */
  /* unsigned char DSROOT[] = { 0x0a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01 }; */
  unsigned char DSROOT[] = { 0x0a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01 }; 
  //  unsigned char DSROOT[] = { 0x9d, 0x00, 0x00, 0x0d, 0x99, 0xb0, 0x4b, 0x01};
 
  for(i=0; i<6; i++)
    {
      if (DSROOT[i] != id.raw[i])
	return 0;
    }

  return 1;
}

/* ************************************************** */
/* main                                               */
/* ************************************************** */

int main(void) 
{
  int root;
  
  WDTCTL = WDTPW + WDTHOLD;
    
  P1IE   = 0x00;        // Interrupt enable
  P2IE   = 0x00;        // 0:disable 1:enable

  // set MCLK on XT2 at 8MHz
  set_mcu_speed_xt2_mclk_8MHz_smclk_1MHz();

  // leds
  LEDS_INIT();
  LEDS_ON();

  // uart console
  uart1_init();
  printf("wsn430 cc1100 tx/rx packet driver test.\n");

  // ds2411
  root = got_root();

  // cc1100
  cc1100_init();
  printf("CC1100 init.\n");

  LEDS_OFF();
  
  eint();

  if (root)
    {
      tx_main();
    }
  else
    {
      rx_main();
    }

  return 0;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */
