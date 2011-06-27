#include <io.h>
#include <signal.h>
#include <iomacros.h>
#include <stdio.h>

#include "uart1.h"
#include "clock.h"

/* Very simple program to test SPI slave mode (MSP430, so master mode for SPI the device) in WSim.
 * Configure MSP430 USART0 in master SPI mode first. Send 10 bytes to the SPI device. Switch MSP430
 * SPI in slave mode, and put SPI mode pin of the device to 1 to signal it to go in SPI master mode.
 * Wait for data returned by the device.
 *
 * No driver for SPI device (as it is not based on a real peripheral): raw functions are coded in 
 * main.c. This example is written to use SPIDEV_MASTER device in WSim.
 */

/**********************
 * Serial printf
 **********************/

int putchar(int c)
{
  return uart1_putchar(c);
}


/**********************
 * Delay function.
 **********************/

#define DELAY 0x800

void delay(unsigned int d) 
{
  int i,j;
  for(j=0; j < 0xff; j++)
    {
      for (i = 0; i<d; i++) 
	{
	  nop();
	  nop();
	}
    }
}

/**********************
 *
 **********************/

uint8_t returned_value;

/* IO Port 3 */
#define SIMO_PIN     1 /* P3.1 - SPI Master out - slave in   (MOSI) */
#define SOMI_PIN     2 /* P3.2 - SPI Master in  - slave out  (MISO) */
#define SCK_PIN      3 /* P3.3 - SPI Serial Clock - slave in (SCLK) */

/* IO Port 4 */
#define SPI_MODE_PIN 6 /* P4.6 - SPI Mode                           */

/* SPI com */
#define SPI_MODE_SLAVE  0
#define SPI_MODE_MASTER 1
#define SPI_RX_READY() ((IFG1 & URXIFG0) != 0)
#define	SPI_WAITFOREORx() while (!SPI_RX_READY())  /* USART0 Rx buffer ready? */

#define SPI_MASTER_TX(x)                    \
do {                                        \
	U0TXBUF = x;                        \
	SPI_WAITFOREORx();                  \
	returned_value = U0RXBUF;           \
} while(0)

#define SPI_MASTER_RX(x)                    \
do {                                        \
	U0TXBUF = 0;                        \
	SPI_WAITFOREORx();                  \
	x = U0RXBUF;                        \
} while(0)


#define SPI_SLAVE_TX(x)                     \
do {                                        \
	U0TXBUF = x;                        \
} while(0)

#define SPI_SLAVE_RX(x)                     \
do {                                        \
	x = U0RXBUF;                        \
} while(0)

#define BM(x) (0x01 << x)

#define SIGNAL_SPI_MODE(x)                  \
do {                                        \
	if (x)                              \
	    P4OUT |= BM(SPI_MODE_PIN);      \
	else                                \
            P4OUT &= ~BM(SPI_MODE_PIN);     \
} while(0)

#define MAX_TAB 10

int i = 0;

int main(void) 
{
  uint8_t tab[MAX_TAB] = {0xa1,0xb2,0xc3,0xd4,0xe5,0xa6,0xb7,0xc8,0xd9,0xe0};

  P1IE  = 0x00;        // Interrupt enable
  P2IE  = 0x00;        // 0:disable 1:enable

  /* Stop the watchdog timer. */
  WDTCTL = WDTPW + WDTHOLD;
    
  /* Setup MCLK 8MHz and SMCLK 1MHz */
  set_mcu_speed_xt2_mclk_8MHz_smclk_1MHz();

  uart1_init();  

  P3SEL |= (BM(SIMO_PIN) | BM(SCK_PIN) | BM(SOMI_PIN)); // Select Peripheral functionality (SPI: SIMO, SOI, SCK)
  P3DIR |= (BM(SIMO_PIN) | BM(SCK_PIN));            // Configure as outputs(SIMO,SCK)
  P3DIR &= ~BM(SOMI_PIN);                       // Configure as inputs(SOMI)

  P4SEL &= ~BM(SPI_MODE_PIN);
  P4DIR |=  BM(SPI_MODE_PIN);

  /* Perform SPI module initialization */
  U0CTL  = CHAR + SYNC + MM + SWRST; // SPI mode, 8-bit transfer, Listen disabled, SPI master, SW reset (c[2]p14-14)
  U0TCTL = CKPH + SSEL1 + STC; // Data on Rising Edge, SMCLK, 3-wire SPI mode (c[2]p14-15)
  
  U0BR0  = 0x04; // SPICLK set baud (c[2]p14-17)
  U0BR1  = 0; // Dont need baud rate control register 2 - clear it (c[2]p14-15)
  U0MCTL = 0; // Dont need modulation control (c[2]p14-15)

  ME1 |= USPIE0; // Module 0 enabled (c[2]p14-19)

  SIGNAL_SPI_MODE(SPI_MODE_SLAVE);
  printf("\n");
  printf("**************************\n");
  printf("SPI device in slave mode  \n");
  printf("**************************\n");
                
  for (i=0; i<MAX_TAB; i++)
      {
          printf("Value %d [0x%02x] sent to SPI device\n", i, tab[i]);
	  SPI_MASTER_TX(tab[i]);
      }

  /* Reset of the table */
  for (i=0; i<MAX_TAB; i++)
      {
          tab[i] = 0;
      }


  U0CTL &= ~MM; /* MSP430 SPI in slave mode */
  printf("\n");
  printf("**************************\n");
  printf("SPI device in master mode \n");
  printf("**************************\n");

  SPI_SLAVE_TX(0xff);
  SIGNAL_SPI_MODE(SPI_MODE_MASTER);

  for (i=0; i<MAX_TAB; i++)
      {
	  SPI_WAITFOREORx();
	  SPI_SLAVE_RX(tab[i]);
          SPI_SLAVE_TX(0xff);
      }

  for (i=0; i<MAX_TAB; i++)
      {
          printf("Value %d [0x%02x] received from SPI device\n", i, tab[i]);
      }

  return 0;
}

