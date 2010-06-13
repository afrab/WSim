/**
 *  \file   main.c
 *  \brief  test2-spi
 *  \author Antoine Fraboulet
 *  \date   2009
 **/

#include <io.h>
#include <signal.h>
#include <iomacros.h>
#include <stdio.h>

#include "leds.h"
#include "clock.h"
#include "timer.h"
#include "uart.h"
#include "spi.h"

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void green_led_cb(void)
{
  LED_SWITCH(2);
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

int uart_cb_echo(uint8_t data)
{
  putchar(data);
  return 0; /* return 1 if LPM1_EXIT */
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

int dummy=0; /* gcc needs this */

int main(void)
{
  WDTCTL = WDTPW + WDTHOLD;                 // Stop WDT
  P1IE   = 0x00;  /* Interrupt enable   */
  P2IE   = 0x00;  /* 0:disable 1:enable */

  /* do not enable interrupts on P1 until radio is configured */

  P1SEL   = 0x00;        // Selector         = 0:GPIO     1:peripheral
  P1DIR   = 0x00;        // Direction        = 0:input    1:output
  P1IE    = 0x00;        // Interrupt enable = 0:disable  1:enable
  P1IES   = 0x00;        // Edge select      = 0:L to H   1:H to L
  P1IFG   = 0x00;
  
  P2SEL   = 0x00; /* note: basculer toutes les portes inutilisée en IN */
  P2DIR   = 0x00; /* désactiver les périphériques                      */
  P2IE    = 0x00;
  P2IES   = 0x00;
  P2IFG   = 0x00;

  P3SEL   = 0x00;
  P3DIR   = 0x00;  
  P3OUT   = 0x00;
  
  P4SEL   = 0x00;
  P4DIR   = 0x00;  
  P4OUT   = 0x00;

  /* leds */
  LEDS_INIT();
  LEDS_OFF();
  
  /* switch to running freq mode            */
  set_mcu_speed_xt2_mclk_8MHz_smclk_1MHz ();

  /* timer settings */
  timerA_init();
  timerA_register_cb(green_led_cb);
  timerA_start(12000);

  /* uart init */
  uart_init();
  printf("test2-spi test program\n");
  uart_register_cb(uart_cb_echo);

  spi_init();

  /* enable interrupt */
  eint();

  while (1)
    {
      LEDS_BLINK();
    }
  return 0;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */
