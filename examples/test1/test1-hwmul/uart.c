
#include <io.h>
#include <signal.h>
#include <iomacros.h>

#include "uart.h"



int uart0_init()
{
  //Init of USART0 Module

  U0ME  |= UTXE0|URXE0;           //Enable USART0 transmiter and receiver (UART mode)
  
  U0CTL  = SWRST;                 //reset
  U0CTL  = CHAR;                  //init & release reset
  
  U0TCTL = SSEL1|TXEPT;      //use SMCLK
  U0RCTL = 0;

//~ //115200 @4MHz
//~ #define U0BR1_INIT      0               //Baud rate 1 register init 'U0BR1' 
//~ #define U0BR0_INIT      0x22            //Baud rate 0 register init 'U0BR0'
//~ #define U0MCTL_INIT     0xdd            //Modulation Control Register init 'U0MCTL':

//9600 @4MHz
#define U0BR1_INIT      0x01
#define U0BR0_INIT      0xA0
#define U0MCTL_INIT     0x5B
  
  U0BR1  = U0BR1_INIT;
  U0BR0  = U0BR0_INIT;
  U0MCTL = U0MCTL_INIT;

  U0IE  |= URXIE1;                    //Enable USART0 receive interrupts (UART mode)
  return 0;
}

/**************************************************/
/** USART *****************************************/
/**************************************************/

#define	UART0_WAIT_FOR_EOTx() while ((U0TCTL & TXEPT) != TXEPT)
#define	UART0_WAIT_FOR_EORx() while ((IFG2 & URXIFG0) == 0)

#define USART0_TX(x)          \
do {                          \
	U0TXBUF = x;          \
	UART0_WAIT_FOR_EOTx(); \
} while(0)

#define USART0_RX(x)            \
do {                          \
	UART0_WAIT_FOR_EORx(); \
	x = U0RXBUF;          \
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
  int c;
  USART0_RX(c);
  return c;
}
