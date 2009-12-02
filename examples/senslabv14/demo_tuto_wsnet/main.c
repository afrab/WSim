
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
 * printf 
 **********************/

int putchar(int c)
{
  return uart0_putchar(c);
}


/**********************
 * swap bits within a byte
 **********************/

uint8_t swapbits(uint8_t c, int count) {
    uint8_t result=0;
    int     i;
 
    for(i = 0; i < count; i++) {
	result  = result << 1;
	result |= (c & 1);
	c = c >> 1;
    }
    return result;
}


/**********************
 * rx callback
 **********************/

volatile int cc2420_size;
volatile int cc2420_rx_received;
volatile int cc2420_ack_flag = 0;
volatile int cc2420_waiting_ack = 0;
uint8_t cc2420_frame_seq = 1;

uint16_t rx_packet_cb(void)
{
  uint8_t rxframe[128];
  uint8_t length;
  uint8_t frame_type;

  if (cc2420_io_fifop_read())
    {
      cc2420_fifo_get(&length, 1);
      micro_delay(0xFFFF); // to fix: bug in wsim or on hardware?
      if ( length < 128 )
	{
	  cc2420_fifo_get(rxframe, length);
	  if ( (rxframe[length-1] & 0x80) != 0 )	// check CRC
	    {
	      cc2420_frame_seq = rxframe[2] + 1;
	      frame_type = swapbits((swapbits(rxframe[0], 8) & 0xE0) >> 5, 3);
	      
	      if (frame_type == 0x02)  /* ack frame */
		{
		  if (cc2420_waiting_ack)
		    {
		      printf("Acknowlegment received\n\n");
		      cc2420_ack_flag = 1;
		      cc2420_waiting_ack = 0;
		    }
		  return 0;
		}
	      else                     /* token frame */
		{
	      printf("Token received from pan id %02x:%02x and addr %02x:%02x\n",
		     rxframe[7],rxframe[8], rxframe[9], rxframe[10]);
	      printf("(to pan id %02x:%02x and addr %02x:%02x)\n\n",
		     rxframe[3],rxframe[4], rxframe[5], rxframe[6]);
	      LED_BLUE_TOGGLE();
	      cc2420_size = length;
		}
	    }
	  else {
	    printf("Frame received but CRC non OK, erreur de transmission?\n\n");
	    LED_RED_TOGGLE();
	    cc2420_size = -1;
	  }
	}
      else
	cc2420_size = -1;
      
      cc2420_rx_received = 1;
    }

  return 0;
}


/**********************
 * send token
 **********************/

void send_token(uint8_t *fcf, uint8_t *dest_pan_id, uint8_t *dest_addr,
		uint8_t *src_pan_id, uint8_t *src_addr)
{
  uint8_t txlength = 13;

  printf("Sending token to pan id %02x:%02x and addr %02x:%02x\n",
	 dest_pan_id[0], dest_pan_id[1], dest_addr[0], dest_addr[1]);
  printf("Waiting for acknowledge frame...\n");

  cc2420_fifo_put(&txlength, 1);
  cc2420_fifo_put(fcf, 2);
  cc2420_fifo_put(&cc2420_frame_seq, 1);
  cc2420_fifo_put(dest_pan_id, 2);
  cc2420_fifo_put(dest_addr, 2);
  cc2420_fifo_put(src_pan_id, 2);
  cc2420_fifo_put(src_addr, 2);

  cc2420_cmd_tx();

  cc2420_waiting_ack = 1;
}


/**********************
 * main
 **********************/

#define MSG_SIZE 255 

int main(void) 
{
  int token = 0;
  int i;
  int attempt_send;
  int nb_dead_nodes;

  timer_wakeup = 0;

  /* leds */
  LEDS_INIT();
  LEDS_OFF();

  // set MCLK on XT2 at 8MHz
  set_mcu_speed_xt2_mclk_8MHz_smclk_1MHz();

  //init uart;
  uart0_init(UART0_CONFIG_1MHZ_115200);
  printf("UART init.\n");
  
  cc2420_init();
  cc2420_io_sfd_register_cb(rx_packet_cb);
  cc2420_io_sfd_int_set_falling();
  cc2420_io_sfd_int_clear();
  cc2420_io_sfd_int_enable();
  printf("CC2420 init.\n");  

  red_led = 0;
  timerA_init();
  timerA_register_cb(TIMERA_ALARM_CCR0, timer_cb);
  timerA_set_alarm_from_now(TIMERA_ALARM_CCR0, 1, 4496);
  timerA_start_ACLK_div(TIMERA_DIV_1);
  printf("Timer A init.\n");

  // enable interrupts
  eint();

  printf("Worldsens test program, id=%d\n",NODEID);

  uint8_t fcf[2] = {0x21, 0x88};  /* -> 00100001 10001000 -> reverse of bits for each byte -> 10000100 00010001 -> ack bit = 1 (6th bit), Frame type = 001 (don't forget to read from right to left) */

  uint8_t dest_pan_id_low_byte = (NODEID + 1) % MAX_NODES;
  uint8_t dest_addr_low_byte   = (NODEID + 1) % MAX_NODES;
  uint8_t dest_pan_id[2]       = {0x22, dest_pan_id_low_byte};
  uint8_t dest_addr[2]         = {0x11, dest_addr_low_byte};

  uint8_t src_pan_id[2]        = {0x22, NODEID};
  uint8_t src_addr[2]          = {0x11, NODEID};

  while ( (cc2420_get_status() & 0x40) == 0 ); // waiting for xosc being stable

  cc2420_set_panid(src_pan_id); // save pan id in ram 
  cc2420_set_shortadr(src_addr); // save short address in ram


  delay(100);

  if (NODEID == 0)
    {
      token = 1;
      LED_BLUE_ON();
      printf("Node %d has token\n\n", NODEID);
    }
  else
    {
      cc2420_cmd_idle();
      cc2420_cmd_flushrx();
      cc2420_cmd_rx();
      printf("Node %d enters in RX mode\n\n", NODEID);
    }

  cc2420_size        = 0;
  cc2420_rx_received = 0;


  while (1)
    {
      if ((timer_wakeup == 1) && (cc2420_rx_received == 1))
	{
	  if (cc2420_size != -1)
	    {
	      token = 1;
	      LED_BLUE_ON();
	    }
	  cc2420_cmd_flushrx();
	  cc2420_cmd_rx();
	  cc2420_rx_received = 0;
	  timer_wakeup       = 0;
	}
      else if ((timer_wakeup == 1) && (token == 1)) /* timer wakeup */
	{
	  attempt_send = 0;

	  while(!cc2420_ack_flag)
	    {
	      cc2420_cmd_idle();
	      cc2420_cmd_flushtx();

	      if (attempt_send >= 5)
		{
		  /* dest node must be dead, try to send to the next one */
		  nb_dead_nodes++;
		  dest_pan_id[1] = (NODEID + nb_dead_nodes + 1) % MAX_NODES;
		  dest_addr[1]   = (NODEID + nb_dead_nodes + 1) % MAX_NODES;
		  attempt_send = 0;
		}
	     
	      /* send */
	      send_token(fcf, dest_pan_id, dest_addr, src_pan_id, src_addr);
	      while (cc2420_io_sfd_read());
	      attempt_send++;

	      /* wait for ack */
	      for (i = 0; i < 10; i++) micro_delay(0xFFFF);
	    }

	  cc2420_ack_flag = 0;
	  token = 0;
	  LED_BLUE_OFF();
	  timer_wakeup = 0;
	}
      else if ((timer_wakeup == 1))
	{
	  /* printf("."); */
	  timer_wakeup = 0;
	}
      else
	{
	  //printf("!%d%d\n",timer_wakeup, cc2420_rx_received);
	  //LPM4;
	}
    }
}
