/**
 *  \file   spi.c
 *  \brief  test2-spi
 *  \author Antoine Fraboulet
 *  \date   2009
 **/

#include <io.h>
#include <signal.h>
#include <iomacros.h>
#include <stdio.h>

#include "spi.h"

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

#define SPI_TX           U0TXBUF
#define SPI_RX           U0RXBUF

#define	SPI_WAIT_EOR()	 do { } while (! (IFG1 & URXIFG0) )
#define SPI_CLR_RX_IFG() do { IFG1 &= ~URXIFG0; } while (0)

#define SPIDEV_PORT      P4
#define SPIDEV_CS_PIN    (1 << 0) /* P4.7 */
#define SPIDEV_DISABLE() do { P4OUT |=  SPIDEV_CS_PIN; } while (0)
#define SPIDEV_ENABLE()  do { P4OUT &= ~SPIDEV_CS_PIN; } while (0)

#define SPI_PORT          P3
#define SIMO_PIN          (1 << 1) /* P3.1 - SPI Master out - slave in   (MOSI) */
#define SOMI_PIN          (1 << 2) /* P3.2 - SPI Master in  - slave out  (MISO) */
#define SCK_PIN           (1 << 3) /* P3.3 - SPI Serial Clock - slave in (SCLK) */
#define CHECK_MISO_HIGH() (P3IN & SOMI_PIN)

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void spi_init(void)
{
  /* ===================================== */
  /* configure all MSP430/SPI related pins */
  /* ===================================== */

  P3SEL |=  (SIMO_PIN | SCK_PIN | SOMI_PIN);
  P3DIR |=  (SIMO_PIN | SCK_PIN           );  // Configure as outputs(SIMO,SCK)
  P3DIR &= ~(                     SOMI_PIN);  // Configure as inputs(SOMI)
  
  P4SEL &= ~(SPIDEV_CS_PIN);
  P4DIR |=  (SPIDEV_CS_PIN);

  /* ===================================== */
  /* initialize the SPI registers          */
  /* ===================================== */

  // Data on Rising Edge, SMCLK, 3-wire SPI mode (c[2]p14-15)
  U0CTL  = CHAR + SYNC + MM + SWRST;
  U0TCTL = CKPH + SSEL1 + STC;      
  
  // SPI Speed
  U0BR0  = 2;         // SPICLK set baud : SMCLK divider (c[2]p14-17)
  U0BR1  = 0;         // Dont need baud rate control register 2 - clear it (c[2]p14-15)
  U0MCTL = 0;         // Dont need modulation control (c[2]p14-15)
  
  ME1 |= USPIE0;      // Module 0 enabled (c[2]p14-19)
  
  U0CTL &= ~SWRST;    // Remove RESET (c[2]p14-14)
  IE1   &= ~(UTXIE0); // no interrupt on SPI tx, active polling on U0TCTL.TXEPT
  IE1   &= ~(URXIE0); // to verify, active polling on IFG2.URXIFG0
}  

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

int spi_tx_rx(int data)
{
  char res;
  SPI_CLR_RX_IFG();
  SPI_TX = data;
  SPI_WAIT_EOR();
  res    = SPI_RX;
  return res;
} 

int spi_check_miso_high(void) 
{ 
  return CHECK_MISO_HIGH();
}

void spi_tx_burst(char* data, int len)
{
  int i;
  for(i=0; i<len; i++)
    {
      SPI_CLR_RX_IFG();
      SPI_TX  = data[i];
      SPI_WAIT_EOR();
    }
}

void spi_rx_burst(char* data, int len)
{
  int i;
  for(i=0; i<len; i++)
    {
      SPI_TX  = SPI_DUMMY_BYTE;
      SPI_WAIT_EOR();
      data[i] = SPI_RX;
    }
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void spidev_select (void)
{
  SPIDEV_ENABLE();
}

void spidev_deselect (void)
{
  SPIDEV_DISABLE();
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */
