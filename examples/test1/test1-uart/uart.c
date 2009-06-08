
#include <io.h>
#include <signal.h>
#include <iomacros.h>

#include "uart.h"



int uart1_init()
{
  //Init of USART0 Module

  U1ME  |= UTXE1|URXE1;           //Enable USART1 transmiter and receiver (UART mode)
  
  U1CTL  = SWRST;                 //reset
  U1CTL  = CHAR;                  //init & release reset
  
  U1TCTL = SSEL_SMCLK|TXEPT;      //use SMCLK
  U1RCTL = 0;

//~ //115200 @4MHz
//~ #define U1BR1_INIT      0               //Baud rate 1 register init 'U0BR1' 
//~ #define U1BR0_INIT      0x22            //Baud rate 0 register init 'U0BR0'
//~ #define U1MCTL_INIT     0xdd            //Modulation Control Register init 'U0MCTL':

//9600 @4MHz
#define U1BR1_INIT      0x01
#define U1BR0_INIT      0xA0
#define U1MCTL_INIT     0x5B
  
  U1BR1  = U1BR1_INIT;
  U1BR0  = U1BR0_INIT;
  U1MCTL = U1MCTL_INIT;

  //  U1IE  |= URXIE1;                    //Enable USART0 receive interrupts (UART mode)
  return 0;
}

/**************************************************/
/** USART *****************************************/
/**************************************************/

#define	UART1_WAIT_FOR_EOTx() while ((U1TCTL & TXEPT) != TXEPT)
#define	UART1_WAIT_FOR_EORx() while ((IFG2 & URXIFG1) == 0)

#define USART1_TX(x)          \
do {                          \
	U1TXBUF = x;          \
	UART1_WAIT_FOR_EOTx(); \
} while(0)

#define USART1_RX(x)            \
do {                          \
	UART1_WAIT_FOR_EORx(); \
	x = U1RXBUF;          \
} while(0)

/**************************************************/
/** USART *****************************************/
/**************************************************/
int uart1_putchar(int c)
{
  USART1_TX(c);
  return (unsigned char)c;
}

int uart1_getchar()
{
  int c;
  USART1_RX(c);
  return c;
}
