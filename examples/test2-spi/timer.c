/**
 *  \file   timer.c
 *  \brief  test2-spi
 *  \author Antoine Fraboulet
 *  \date   2009
 **/

#include <io.h>
#include <signal.h>
#include <iomacros.h>
#include <stdio.h>

#include "timer.h"

/* ************************************************** */
/* TimerA on SMCLK / 8                                */
/* ************************************************** */

volatile static timer_cb timerA_cb;
volatile static int      timerA_wakeup;

interrupt (TIMERA0_VECTOR)  Timer_A (void)
{
  if (timerA_cb != NULL)
    timerA_cb();

  if (timerA_wakeup == 1)
    LPM1_EXIT;
}

void timerA_init(void)
{
  timerA_cb     = NULL;
  timerA_wakeup = 0;
  timerA_stop();
}

void timerA_register_cb(timer_cb cb)
{
  timerA_cb = cb;
}

void timerA_set_wakeup(int w)
{
  timerA_wakeup = w;
}

void timerA_start(uint16_t ticks)
{
  /* Ensure the timer is stopped. */
  TACTL = 0;
  /* Clear everything to start with. */
  TACTL |= TACLR;
  /* Run the timer of the ACLK. */
  TACTL = TASSEL_1;
  /* Enable the interrupts. */
  TACTL |= TAIE;

  /* ID_3                (3<<6)  Timer A input divider: 3 - /8 */
  /*
  switch (div) {
  case 1:  TACTL |= ID_0; break;
  case 2:  TACTL |= ID_1; break;
  case 4:  TACTL |= ID_2; break;
  case 8:  TACTL |= ID_3; break;
  default: TACTL |= ID_0; break;
  }
  */
  TACTL |= ID_3;
  /* Continuous mode. */
  TACTL |= MC_2;
}

void timerA_stop(void)
{
  TACTL = 0;
}

/* ************************************************** */
/*                                                    */
/* ************************************************** */
