
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

/*************************************************************


                                          sensor 1 > file.copy1
                                        /
   cat file.org | input | sensor0 ____ /
                                       \
                                        \ sensor 2 > file.copy2


  Every message sent to a node is sent to another node, 
   
 *************************************************************/

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

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

volatile int green_led = 0;
volatile int timer_wakeup = 0;

uint16_t timer_cb(void)
{
  if (green_led)
    LED_GREEN_OFF();
  else
    LED_GREEN_ON();
  green_led = 1 - green_led;
  timer_wakeup = 1;

  return 0; /* 1 == wakeup */
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

#define SERIAL_RX_BUFFS     3
#define SERIAL_RX_FIFO_SIZE 0x100

struct rx_buffer_t {
  uint8_t  buffer[SERIAL_RX_FIFO_SIZE];
  uint16_t rptr;
  uint16_t wptr;
  uint16_t size;
};

volatile uint8_t   rx_rbuff;
volatile uint8_t   rx_wbuff;
volatile uint8_t   rx_bsize;
struct rx_buffer_t rx_buffs[SERIAL_RX_BUFFS];

/* ************************************************** */

void serial_rx_buffer_init()
{
  int i;
  for(i=0; i<SERIAL_RX_BUFFS; i++)
    {
      rx_buffs[i].rptr = 0;
      rx_buffs[i].wptr = 0;
      rx_buffs[i].size = 0;
    }
  rx_rbuff = 0;
  rx_wbuff = 0;
  rx_bsize = 0;
}

/* ************************************************** */

void serial_rx_buffer_put(struct rx_buffer_t *rx_buffer, uint8_t data) 
{

  rx_buffer->buffer[ rx_buffer->wptr ] = data;
  rx_buffer->wptr = (rx_buffer->wptr + 1) % SERIAL_RX_FIFO_SIZE;
  if (rx_buffer->size < SERIAL_RX_FIFO_SIZE)
    {
      rx_buffer->size ++;
    }
  else
    {
      LED_RED_ON();
    }
}

/* ************************************************** */

int serial_rx_buffer_get(struct rx_buffer_t *rx_buffer, uint8_t *data)
{
  dint();
  if (rx_buffer->size > 0)
    {
      *data = rx_buffer->buffer[ rx_buffer->rptr ];
      rx_buffer->rptr = (rx_buffer->rptr + 1) % SERIAL_RX_FIFO_SIZE;
      rx_buffer->size --;
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

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

#define TX_PACKET_SIZE 20

volatile int uart_wakeup = 0;

uint16_t uart_cb(uint8_t c)
{
  serial_rx_buffer_put(& rx_buffs[ rx_wbuff ], c);
  
  if (rx_buffs[ rx_wbuff ].size > TX_PACKET_SIZE)
    {
      uart_wakeup = 1;
    }

  return uart_wakeup; 
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

#define PKT_TYPE_OPEN  01
#define PKT_TYPE_DATA  02
#define PKT_TYPE_CLOSE 03

/*
   frame:
      [ x]  length
      [ 0]  fcf frame_type
      [ 1]  fcf
      [ 2]  frame_seq
      [ 3]  dst panid
      [ 4]  dst panid
      [ 5]  dst addr
      [ 6]  dst addr
      [ 7]  src panid
      [ 8]  src panid
      [ 9]  src addr
      [10]  src addr
      [11]  pkt type
      [12]  data length
      [13]  data
      [xx]  ...
*/

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void tx_packet(uint8_t *fcf, uint8_t frame_seq, uint8_t *dest_pan_id, uint8_t *dest_addr,
               uint8_t *src_pan_id, uint8_t *src_addr, 
               uint8_t pkt_type, uint8_t* data, uint8_t datalen)
{
  //int i;
  uint8_t txlength  = 13 + 2 + datalen;

  cc2420_cmd_idle();
  cc2420_cmd_flushtx();

  cc2420_fifo_put(& txlength,  1);
  cc2420_fifo_put(  fcf,       2);
  cc2420_fifo_put(& frame_seq, 1);
  cc2420_fifo_put(dest_pan_id, 2);
  cc2420_fifo_put(dest_addr,   2);
  cc2420_fifo_put(src_pan_id,  2);
  cc2420_fifo_put(src_addr,    2);
  // ==
  cc2420_fifo_put(&pkt_type,   1);
  cc2420_fifo_put(&datalen,    1);
  cc2420_fifo_put(data,  datalen);

  cc2420_cmd_tx();
	     
  /* send */
  while (cc2420_io_sfd_read());
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void sender( uint8_t *fcf )
{
  uint8_t *pktdata;
  uint8_t pktdatalen;
  uint8_t cc2420_frame_seq     =  1;

  uint8_t src_pan_id[2]        = {0x22, NODEID};
  uint8_t src_addr[2]          = {0x11, NODEID};
  uint8_t dest_pan_id_low_byte = (NODEID + 1) % MAX_NODES;
  uint8_t dest_addr_low_byte   = (NODEID + 1) % MAX_NODES;
  uint8_t dest_pan_id[2]       = {0x22, dest_pan_id_low_byte};
  uint8_t dest_addr[2]         = {0x11, dest_addr_low_byte};

  rx_rbuff = 0;
  rx_wbuff = 0;
  rx_bsize = 0;

  printf("sender start\n");
  while (1)
    {
      LPM0;

      if ((timer_wakeup == 1) /* && */ )
	{
	  timer_wakeup    = 0;
	}
      
      if ((uart_wakeup == 1) /* && */)
	{
	  LED_BLUE_ON();
	  /* prepare next uart buffer */
	  rx_wbuff = (rx_wbuff + 1) % SERIAL_RX_BUFFS;
	  rx_bsize += 1;
	  if ( rx_bsize == SERIAL_RX_BUFFS)
	    {
	      printf("overflow, out of buffers\n");
	      LED_RED_ON();
	      dint();
	      LPM4;
	    }

	  /* send packet */
	  pktdatalen = rx_buffs[ rx_rbuff ].size;
	  pktdata    = rx_buffs[ rx_rbuff ].buffer;

	  tx_packet(fcf, cc2420_frame_seq, 
	            dest_pan_id, dest_addr, 
	            src_pan_id, src_addr, 
	            PKT_TYPE_DATA, pktdata, pktdatalen);
	  cc2420_frame_seq ++;

	  /* drop uart buffer */
	  rx_rbuff  = (rx_rbuff + 1) % SERIAL_RX_BUFFS;
	  rx_bsize -= 1;

	  LED_BLUE_OFF();
	  uart_wakeup        = 0;
	}
    }
}


/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

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

uint16_t rx_packet_cb(void)
{
  uint8_t length;
  uint8_t frame_type;

  if (cc2420_io_fifop_read())
    {
      cc2420_fifo_get(& length, 1);
      // micro_delay(0xFFFF); /* to fix: bug in wsim or on hardware */
      if ( length < 128 )
        {
          cc2420_fifo_get( rx_buffs[ rx_wbuff ].buffer, length);
	  rx_buffs[ rx_wbuff ].size = length;
	  rx_buffs[ rx_wbuff ].rptr = 0;
	  rx_buffs[ rx_wbuff ].wptr = length;

          if ( (rx_buffs[ rx_wbuff ].buffer[length-1] & 0x80) != 0 ) // check CRC
            {
              frame_type = swapbits((swapbits( rx_buffs[ rx_wbuff ].buffer[0], 8) & 0xE0) >> 5, 3);
              
              if (frame_type == 0x02)  /* ack frame */
                {
		  LED_RED_ON();
		  rx_buffs[ rx_wbuff ].size = 0;
                  return 0; 
                }
              else                     /* data frame */
                {
		  rx_wbuff = (rx_wbuff + 1) % SERIAL_RX_BUFFS;
		  rx_bsize += 1;
		  if ( rx_bsize == SERIAL_RX_BUFFS)
		    {
		      LED_RED_ON();
		      dint();
		      LPM4;
		    }
                }
            }
          else
	    {
	      LED_RED_ON();
	      rx_buffs[ rx_wbuff ].size = 0;
	      return 0;
	    }
        }
      else
	{
	  rx_buffs[ rx_wbuff ].size = 0;
	  return 0;
	}
    }
  return 1; /* wakeup after IRQ */
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void receiver( uint8_t *fcf )
{

  printf("receiver start\n");
  while (1)
    {
      LPM0;

      if ((timer_wakeup == 1) /* && */ )
	{
	  timer_wakeup       = 0;
	}

      if ( (rx_bsize > 0) && (rx_buffs[ rx_rbuff ].size > 0) )
        {
	  switch (rx_buffs[ rx_rbuff ].buffer[11]) 
	    {
	    case PKT_TYPE_OPEN:
	      break;
	    case PKT_TYPE_DATA:
	      {
		int i;
		for(i=13; i < rx_buffs[ rx_rbuff ].size; i++)
		  {
		    putchar( rx_buffs[ rx_rbuff ].buffer[i] );
		  }
		rx_rbuff  = (rx_rbuff + 1) % SERIAL_RX_BUFFS;
		rx_bsize -= 1;
	      }
	      break;
	    case PKT_TYPE_CLOSE:
	      break;
	    }
          cc2420_cmd_flushrx();
          cc2420_cmd_rx();
        }
    }
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

int main(void) 
{

  // leds
  LEDS_INIT();
  LEDS_OFF();

  // set MCLK on XT2 at 8MHz
  set_mcu_speed_xt2_mclk_8MHz_smclk_1MHz();

  //init uart;
  uart0_init(UART0_CONFIG_1MHZ_9600);
  uart0_register_callback(uart_cb);
  
  // cc2420
  cc2420_init();
  cc2420_io_sfd_register_cb(rx_packet_cb);
  cc2420_io_sfd_int_set_falling();
  cc2420_io_sfd_int_clear();
  cc2420_io_sfd_int_enable();

  // timerA
  timerA_init();
  timerA_register_cb(TIMERA_ALARM_CCR0, timer_cb);
  timerA_set_alarm_from_now(TIMERA_ALARM_CCR0, 1, 4496);
  timerA_start_ACLK_div(TIMERA_DIV_1);

  // enable interrupts
  eint();

  /* 
       0x21, 0x88
     -> 00x0 0001, 1000 1000 
     -> reverse of bits for each byte 
     -> 1000 0100  0001 0001 
        z     x      yy   yy
     
     x  = ack bit (6th bit), 
     yy = src and dst addressing mode, 01 == 16 bits
     z  = packet type
          Frame type = 001 (don't forget to read from right to left) 
  */
  uint8_t fcf[2] = {0x01, 0x00};  
  uint8_t src_pan_id[2]        = {0x22, NODEID};
  uint8_t src_addr[2]          = {0x11, NODEID};

  // waiting for xosc being stable
  while ( (cc2420_get_status() & 0x40) == 0 ); 

  cc2420_set_panid(src_pan_id);  // save pan id in ram 
  cc2420_set_shortadr(src_addr); // save short address in ram
  delay(100);
  cc2420_cmd_idle();
  cc2420_cmd_flushrx();
  cc2420_cmd_rx();

  if (NODEID == 0)
    {
      sender( fcf );
    }
  else
    {
      receiver( fcf );
    }

  return 0;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */
