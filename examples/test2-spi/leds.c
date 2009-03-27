/**
 *  \file   leds.c
 *  \brief  test2-spi
 *  \author Antoine Fraboulet
 *  \date   2009
 **/

#include <io.h>
#include <signal.h>
#include <iomacros.h>

#include "leds.h"


/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

volatile int leds;

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */


#define LED_PORT       P5OUT

void LED_ON(int i)
{
  LED_PORT |= (1 << i);
  leds     |= (1 << i);
}

void LED_OFF(int i)
{
  LED_PORT &= ~(1 << i);
  leds     &= ~(1 << i);
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void LEDS_INIT(void)
{
  P5OUT  &= ~(0xff);
  P5DIR  |=  (0xff);
  P5SEL  &= ~(0xff);
  LEDS_OFF();
}

void LEDS_ON(void)
{
  LED_PORT = 0xff;
  leds     = 0xff;
}

void LEDS_OFF(void)
{
  LED_PORT = 0;
  leds     = 0;
}

void LED_SWITCH(int i)
{
  if (leds & (1 << i))
    {
      LED_ON(i);
    }
  else
    {
      LED_OFF(i);
    }
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

#define LED_DELAY 60000

#define LED_WAIT_LOOP(time)			\
  do {						\
    unsigned int n = time;			\
    /* 4 cycles loop */				\
    __asm__ __volatile__ (			\
			  "1: \n"		\
			  " dec   %[n] \n"	\
			  " jne   1b \n"	\
			  : [n] "+r"(n));	\
  } while (0)


#define LED_WAIT()				\
  do {						\
    LED_WAIT_LOOP(LED_DELAY);			\
    LED_WAIT_LOOP(LED_DELAY);			\
  } while (0)


void LEDS_BLINK(void)
{
  int i = 0;
  while (1)
    {
      LEDS_OFF();
      for(i=0; i < 8; i++)
	{
	  LED_ON(i);
	  LED_WAIT();
	  LED_OFF(i);
	}
    }
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

