/**
 *  \file   uart.c
 *  \brief  test2-spi
 *  \author Antoine Fraboulet
 *  \date   2009
 **/

#include <io.h>
#include <signal.h>
#include <iomacros.h>
#include <stdio.h>

#include "uart.h"

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
/** USART *****************************************/
/**************************************************/

volatile uart_cb_t callback;

void uart_eint(void);
void uart_dint(void);

void uart_init(void)
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
  
  U1TCTL = SSEL_SMCLK|TXEPT;      //use SMCLK 
  U1RCTL = 0;

  /* SMCLK @ 1MHz */
  UBR01=0x08; UBR11=0x00; UMCTL1=0x5B; /* uart1 1000000Hz 114942bps */

  callback = NULL;
  uart_eint();
}

void uart_stop()
{
  P3SEL &= ~(UART1_BIT_RX | UART1_BIT_TX);
  U1ME  &= ~(UTXE1 | URXE1);
}

void uart_register_cb(uart_cb_t f)
{
  callback = f;
}

/**************************************************/
/** USART *****************************************/
/**************************************************/

void uart_eint()
{
  U1IE  |= URXIE1;         
}

void uart_dint()
{
  U1IE &= ~( URXIE1);
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

int uart_putchar(int c)
{
  USART1_TX(c);
  return (unsigned char)c;
}

int uart_getchar(void)
{
  int c;
  USART1_RX(c);
  return c;
}

int putchar(int c)
{
  USART1_TX(c);
  return (unsigned char)c;
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
      if (callback(U1RXBUF) != 0)
	{
	  LPM1_EXIT;
	}
    }
}

/**************************************************/
/** USART *****************************************/
/**************************************************/
