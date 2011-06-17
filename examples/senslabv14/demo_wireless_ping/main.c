
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

    serial <> sensor1 <> ________
                                 \
                                  | sensor 2
    serial <> sensor1 <> ________/

  Every message sent to a node is sent to another node, modifyed and
  sent back to the initiator.

  Every message is acknowledged at the MAC layer.
   
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

#define SERIAL_RX_FIFO_SIZE 0x40 /*  must be a power of 2 */

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

volatile int uart_wakeup = 0;

uint16_t uart_cb(uint8_t c)
{
  serial_rx_buffer_put(c);
  
  if (c == 10)
    {
      uart_wakeup = 1;
    }

  return uart_wakeup; 
}

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

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

uint8_t rxframe[128];
volatile int16_t cc2420_size         = -1;
volatile uint8_t cc2420_rx_received  =  0;
volatile uint8_t cc2420_waiting_ack  =  0;
volatile uint8_t cc2420_frame_seq    =  1;

#define PKT_TYPE_REQUEST 01
#define PKT_TYPE_REPLY   02


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

uint16_t rx_packet_cb(void)
{
  uint8_t length;
  uint8_t frame_type;

  if (cc2420_io_fifop_read())
    {
      cc2420_fifo_get(&length, 1);
      micro_delay(0xFFFF); /* to fix: bug in wsim or on hardware */
      if ( length < 128 )
        {
          cc2420_fifo_get(rxframe, length);
          if ( (rxframe[length-1] & 0x80) != 0 )        // check CRC
            {
              cc2420_frame_seq = rxframe[2] + 1;
              frame_type = swapbits((swapbits(rxframe[0], 8) & 0xE0) >> 5, 3);
              
#define RSSI_OFFSET (-45)
	      uint8_t lqi  = rxframe[length-1] & 0x7f ;
	      int8_t  rssi = rxframe[length-2] + RSSI_OFFSET;        

              if (frame_type == 0x02)  /* ack frame */
                {
                  if (cc2420_waiting_ack)
                    {
                      printf("Acknowlegment received\n\n");
                      cc2420_waiting_ack = 0;
                    }
                  return 0; 
                }
              else                     /* token frame */
                {
		  printf("Frame received from pan id %02x:%02x and addr %02x:%02x\n",
		         rxframe[7],rxframe[8], rxframe[9], rxframe[10]);
		  printf("to pan id %02x:%02x and addr %02x:%02x, type = %d, size %d, frame_seq %d, rssi %ddBm, lqi %d\n",
		         rxframe[3], rxframe[4], rxframe[5], rxframe[6],
		         rxframe[11], rxframe[12], rxframe[2], rssi, lqi);
		  cc2420_size = length;
                }
            }
          else {
            printf("Frame received but CRC non OK, transmission error?\n\n");
            LED_RED_TOGGLE();
            cc2420_size = -1;
          }
        }
      else
	{
	  cc2420_size = -1;
	}

      cc2420_rx_received = 1;
    }

  return 1; /* wakeup after IRQ */
}

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

  printf("Sending msg to pan id %02x:%02x and addr %02x:%02x, type %d, size %d, frame_seq %d\n",
         dest_pan_id[0], dest_pan_id[1], dest_addr[0], dest_addr[1], pkt_type, datalen, frame_seq);

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

  cc2420_waiting_ack = 1;
	     
  /* send */
  while (cc2420_io_sfd_read());
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

int main(void) 
{
  int i;
  uint8_t res;
  uint8_t uartdata;
  uint8_t pktdata[128];
  uint8_t pktdatalen;


  // leds
  LEDS_INIT();
  LEDS_OFF();

  // set MCLK on XT2 at 8MHz
  set_mcu_speed_xt2_mclk_8MHz_smclk_1MHz();

  //init uart;
  uart0_init(UART0_CONFIG_1MHZ_115200);
  uart0_register_callback(uart_cb);
  printf("UART init.\n");
  
  // cc2420
  cc2420_init();
  cc2420_io_sfd_register_cb(rx_packet_cb);
  cc2420_io_sfd_int_set_falling();
  cc2420_io_sfd_int_clear();
  cc2420_io_sfd_int_enable();
  printf("CC2420 init.\n");  

  // timerA
  timerA_init();
  timerA_register_cb(TIMERA_ALARM_CCR0, timer_cb);
  timerA_set_alarm_from_now(TIMERA_ALARM_CCR0, 1, 4496);
  timerA_start_ACLK_div(TIMERA_DIV_1);
  printf("Timer A init.\n");

  // enable interrupts
  eint();

  printf("Worldsens test program, id=%d\n",NODEID);

  uint8_t fcf[2] = {0x21, 0x88};  
  /* 
     -> 00100001 10001000 -> reverse of bits for each byte -> 10000100 00010001 -> ack bit = 1 (6th bit), 
     Frame type = 001 (don't forget to read from right to left) 
  */

  uint8_t dest_pan_id_low_byte = (NODEID + 1) % MAX_NODES;
  uint8_t dest_addr_low_byte   = (NODEID + 1) % MAX_NODES;
  uint8_t dest_pan_id[2]       = {0x22, dest_pan_id_low_byte};
  uint8_t dest_addr[2]         = {0x11, dest_addr_low_byte};

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
  printf("Node %d ready and in RX mode\n\n", NODEID);




  while (1)
    {
      LPM0;

      if ((timer_wakeup == 1) /* && */ )
	{
	  timer_wakeup       = 0;
	}

      if (cc2420_rx_received == 1)
	{
	  if (cc2420_size != -1)
	    {
	      switch (rxframe[11]) {
	      case PKT_TYPE_REQUEST:
		printf("Request received with frame_seq %d, data - ", rxframe[2]);
		for(i=0;i<rxframe[12];i++)
		  {
		    putchar(rxframe[13+i]);
		  }
		putchar('\n');
		tx_packet(fcf, rxframe[2], 
		          &rxframe[7], &rxframe[9], /* dst */
		          &rxframe[3], &rxframe[5], /* src */
		          PKT_TYPE_REPLY, &rxframe[13], rxframe[12]);
		break;
	      case PKT_TYPE_REPLY:
		printf("Reply received for frame_seq %d, data  - ", rxframe[2]);
		for(i=0;i<rxframe[12];i++)
		  {
		    putchar(rxframe[13+i]);
		  }
		putchar('\n');
		break;
	      };
		
	    }
	  cc2420_cmd_flushrx();
	  cc2420_cmd_rx();
	  cc2420_rx_received = 0;
	}

      if (uart_wakeup == 1)
	{
	  /* pkt received from uart, send and wait for ack */

	  /* copy from uart buffer to pkt */
	  LED_BLUE_ON();

	  pktdatalen = 0;
	  while ((res = serial_rx_buffer_get( &uartdata )) != 0)
	    {
	      pktdata[ pktdatalen++ ] = uartdata;
	    }

	  tx_packet(fcf, cc2420_frame_seq, 
	            dest_pan_id, dest_addr, 
	            src_pan_id, src_addr, 
	            PKT_TYPE_REQUEST, pktdata, pktdatalen);

	  LED_BLUE_OFF();
	  uart_wakeup        = 0;
	}
    } 
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */
