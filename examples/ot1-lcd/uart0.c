
#include <io.h>
#include <signal.h>
#include <iomacros.h>

#include "uart0.h"

/**************************************************/
/** INPUT FIFO ************************************/
/**************************************************/

#define FIFO_SIZE 10
static unsigned char fifo[FIFO_SIZE];
static int           fifo_ptr_read;
static int           fifo_ptr_write;
static int           fifo_state;

static inline unsigned char fifo_read()
{
  if (fifo_state == 0)
    {
      return 0;
    }
  else
    {
      unsigned char v    = fifo[fifo_ptr_read];
      fifo_state        -= 1;
      fifo_ptr_read      = (fifo_ptr_read + 1) % FIFO_SIZE;
      return v;
    }
}

static inline void fifo_write(unsigned char val)
{
  if (fifo_state == FIFO_SIZE)
    {
      /*printf("fifo overflow\n");*/
    }
  else
    {
      fifo[fifo_ptr_write]  = val;
      fifo_state           += 1;
      fifo_ptr_write        = (fifo_ptr_write + 1) % FIFO_SIZE;
    }
}

/**************************************************/
/** USART *****************************************/
/**************************************************/

int uart0_init()
{
  //Init of USART0 Module

  U0ME  |= UTXE0|URXE0;           //Enable USART0 transmiter and receiver (UART mode)
  
  U0CTL  = SWRST;                 //reset
  U0CTL  = CHAR;                  //init & release reset
  
  U0TCTL = SSEL_SMCLK|TXEPT;      //use SMCLK
  U0RCTL = 0;

//~ //115200 @4MHz
//~ #define U1BR1_INIT      0               //Baud rate 1 register init 'U0BR1' 
//~ #define U1BR0_INIT      0x22            //Baud rate 0 register init 'U0BR0'
//~ #define U1MCTL_INIT     0xdd            //Modulation Control Register init 'U0MCTL':

#define U0BR1_INIT      0x01
#define U0BR0_INIT      0xA0
#define U0MCTL_INIT     0x5B
  
  U0BR0  = U0BR0_INIT;
  U0BR1  = U0BR1_INIT;
  U0MCTL = U0MCTL_INIT;

  U0IE  |= URXIE0;                    //Enable USART0 receive interrupts (UART mode)
  return 0;
}

/**************************************************/
/** USART *****************************************/
/**************************************************/

interrupt (USART0RX_VECTOR) irq_usart0rx( void )
{
  fifo_write(U0RXBUF);
  _BIC_SR_IRQ(LPM0_bits);
}

/**************************************************/
/** USART *****************************************/
/**************************************************/

#define	UART0_WAIT_FOR_EOTx() while ((U0TCTL & TXEPT) != TXEPT)
#define	UART0_WAIT_FOR_EORx() while ((IFG1 & URXIFG0) == 0)

#define USART0_TX(x)           \
do {                           \
	U0TXBUF = x;           \
	UART0_WAIT_FOR_EOTx(); \
} while(0)

#define USART0_RX(x)            \
do {                            \
	UART0_WAIT_FOR_EORx();  \
	x = U0RXBUF;            \
} while(0)

/**************************************************/
/** USART *****************************************/
/**************************************************/
int uart0_putchar(int c)
{
  USART0_TX(c);
  return (unsigned char)c;
}


int uart0_getchar()
{
  return fifo_read();
}

/*
int uart0_getchar()
{
  int c;
  USART0_RX(c);
  return c;
}
*/
