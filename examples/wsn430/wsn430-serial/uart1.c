
#include <io.h>
#include <signal.h>
#include <iomacros.h>

#include "uart1.h"

#ifndef U1ME
#define U1ME ME2
#endif
#ifndef U1IE
#define U1IE IE2
#endif

#define UART1_PIN_RX 7
#define UART1_PIN_TX 6

#define UART1_BIT_RX (1 << UART1_PIN_RX)
#define UART1_BIT_TX (1 << UART1_PIN_TX)

/**************************************************/
/** INPUT FIFO ************************************/
/**************************************************/

/* keep ^2 size to strength-reduce % operation */
#define RX_FIFO_SIZE 16 

volatile unsigned char fifo[RX_FIFO_SIZE];
volatile unsigned char fifo_ptr_read;
volatile unsigned char fifo_ptr_write;
volatile unsigned char fifo_state;

static inline unsigned int fifo_read()
{
  if (fifo_state == 0)
    {
      return 0xffff;
    }
  else
    {
      unsigned char v    = fifo[fifo_ptr_read];
      fifo_state        -= 1;
      fifo_ptr_read      = (fifo_ptr_read + 1) % RX_FIFO_SIZE;
      return v;
    }
}

static inline void fifo_write(unsigned char val)
{
  if (fifo_state == RX_FIFO_SIZE)
    {
      /* buffer overflow */
    }
  else
    {
      fifo[fifo_ptr_write]  = val;
      fifo_state           += 1;
      fifo_ptr_write        = (fifo_ptr_write + 1) % RX_FIFO_SIZE;
    }
}

/**************************************************/
/** USART *****************************************/
/**************************************************/

int uart1_init()
{
  // Select port or module -function on port
  // 0 : I/O function 
  // 1 : peripheral module

  // Init port direction register of port
  // 0 : input direction
  // 1 : output direction

  //Init of MSP430 Usart1 pins
  P3SEL |= (UART1_BIT_RX | UART1_BIT_TX);

  //Init of USART1 Module
  U1ME  |= UTXE1|URXE1;           //Enable USART1 transmiter and receiver (UART mode)
  
  U1CTL  = SWRST;                 //reset
  U1CTL  = CHAR;                  //init & release reset
  
  U1TCTL = SSEL1 | TXEPT;        //use SMCLK 
  U1RCTL = 0;

  // 38400 @ SMCLK 1MHz
#define U1BR1_INIT        0
#define U1BR0_INIT        0x1B
#define U1MCTL_INIT       0x03

//115200 @4MHz
//#define U1BR1_INIT      0               //Baud rate 1 register init 'U0BR1' 
//#define U1BR0_INIT      0x22            //Baud rate 0 register init 'U0BR0'
//#define U1MCTL_INIT     0xdd            //Modulation Control Register init 'U0MCTL':

//9600 @4MHz
//#define U1BR1_INIT      0x01
//#define U1BR0_INIT      0xA0
//#define U1MCTL_INIT     0x5B
  
  U1BR1  = U1BR1_INIT;
  U1BR0  = U1BR0_INIT;
  U1MCTL = U1MCTL_INIT;

  fifo_ptr_read  = 0;
  fifo_ptr_write = 0;
  fifo_state     = 0;

  return 0;
}


void uart1_eint()
{
  U1IE  |= UTXIE1 | URXIE1;  // Enable USART1 transmit and receive interrupts                  
}

void uart1_eint_tx()
{
  U1IE  |= UTXIE1;
}

void uart1_eint_rx()
{
  U1IE  |= URXIE1;         
}

void uart1_dint()
{
  U1IE &= ~( UTXIE1 | URXIE1);
}

/**************************************************/
/** USART *****************************************/
/**************************************************/

#define	UART1_WAIT_FOR_EOTx() while ((U1TCTL & TXEPT) != TXEPT)
#define	UART1_WAIT_FOR_EORx() while ((IFG2 & URXIFG1) == 0)

#define USART1_TX(x)           \
do {                           \
	U1TXBUF = x;           \
	UART1_WAIT_FOR_EOTx(); \
} while(0)

#define USART1_RX(x)            \
do {                            \
	UART1_WAIT_FOR_EORx();  \
	x = U1RXBUF;            \
} while(0)


/**************************************************/
/** USART *****************************************/
/**************************************************/

int uart1_putchar(int c)
{
  USART1_TX(c);
  return (unsigned char)c;
}

int uart1_getchar(int *c)
{
  if (fifo_state == 0)
    return 0;

  *c =  fifo_read();
  return 1;
}

int uart1_getchar_polling()
{
  int c;
  USART1_RX(c);
  return c;
}

/**************************************************/
/** USART *****************************************/
/**************************************************/

interrupt (USART1RX_VECTOR) wakeup usart1irq( void )
{
  volatile unsigned char dummy;
  /* Check status register for receive errors. */
  if(URCTL1 & RXERR) 
    {
      /* Clear error flags by forcing a dummy read. */
      dummy = RXBUF1;
    } 
  else 
    {
      fifo_write(U1RXBUF);
    }
}

/**************************************************/
/** USART *****************************************/
/**************************************************/
