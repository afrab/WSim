#include <io.h>
#include <signal.h>
#include <iomacros.h>
#include "timer.h"

#define LFXT1CLK_HZ  32768
#define XT2_HZ       8000000

/********************************************
 *
 * TIMER A
 *
 ********************************************/

static timerA3cb A3_0_cb;
static volatile uint8_t timerA3_keep_active_b;

void timerA3_ACLK_start_Hz(int HZ)
{
  /* Ensure the timer is stopped. */
  TACTL = 0;
  /* Clear everything to start with. */
  TACTL |= TACLR;
  /* Run the timer of the ACLK. */
  TACTL = TASSEL_1;
  /* Set the compare match value according to the tick rate we want. */
  TACCR0 = LFXT1CLK_HZ / HZ;
  /* Enable the interrupts. */
  TACCTL0 = CCIE;

  /* ID_0                (0<<6)  Timer A input divider: 0 - /1 */
  /* ID_1                (1<<6)  Timer A input divider: 1 - /2 */
  /* ID_2                (2<<6)  Timer A input divider: 2 - /4 */
  /* ID_3                (3<<6)  Timer A input divider: 3 - /8 */
  TACTL |= ID_0;

  /* Up mode. */
  TACTL |= MC_1;
}


void timerA3_ACLK_start_xS(int div)
{
  /* Ensure the timer is stopped. */
  TACTL = 0;
  /* Clear everything to start with. */
  TACTL |= TACLR;
  /* Run the timer of the ACLK. */
  TACTL = TASSEL_1;
  /* Set the compare match value according to the tick rate we want. */
  TACCR0 = LFXT1CLK_HZ;
  /* Enable the interrupts. */
  TACCTL0 = CCIE;

  /* ID_0                (0<<6)  Timer A input divider: 0 - /1 */
  /* ID_1                (1<<6)  Timer A input divider: 1 - /2 */
  /* ID_2                (2<<6)  Timer A input divider: 2 - /4 */
  /* ID_3                (3<<6)  Timer A input divider: 3 - /8 */
  TACTL |= div;

  /* Up mode. */
  TACTL |= MC_1;
}

void timerA3_ACLK_start_2S()
{
  timerA3_ACLK_start_xS(ID_1);
}

void timerA3_ACLK_start_4S()
{
  timerA3_ACLK_start_xS(ID_2);
}

void timerA3_ACLK_start_8S()
{
  timerA3_ACLK_start_xS(ID_3);
}

void timerA3_register_callback(timerA3cb f)
{
  A3_0_cb = f;
  timerA3_keep_active_b = 0;
}

void timerA3_keep_active(void)
{
  timerA3_keep_active_b = 1;
}

void timerA3_stop()
{
  TACTL = 0;
}

interrupt (TIMERA0_VECTOR) timerA0( void )
{
  if (A3_0_cb != 0)
    {
      (*A3_0_cb)();
    }
  if (timerA3_keep_active_b)
    {
      LPM0_EXIT;
    }
}

/********************************************
 *
 * TIMER B
 *
 ********************************************/

static timerB7cb B7_0_cb;

void timerB7_SMCLK_start_Hz(int Hz)
{
  /* Ensure the timer is stopped. */
  TBCTL = 0;

  /* Clear everything to start with. */
  TBCTL |= TBCLR;

  /* TBSSEL_0            (0<<8)   Clock source: TBCLK */
  /* TBSSEL_1            (1<<8)   Clock source: ACLK  */
  /* TBSSEL_2            (2<<8)   Clock source: SMCLK */
  /* TBSSEL_3            (3<<8)   Clock source: INCLK */
  TBCTL = TBSSEL_2;

  /* ID_0                (0<<6)  Timer A input divider: 0 - /1 */
  /* ID_1                (1<<6)  Timer A input divider: 1 - /2 */
  /* ID_2                (2<<6)  Timer A input divider: 2 - /4 */
  /* ID_3                (3<<6)  Timer A input divider: 3 - /8 */
  TBCTL |= ID_3;

  /* CNTL_0              (0<<11)  Counter length: 16 bit */
  /* CNTL_1              (1<<11)  Counter length: 12 bit */
  /* CNTL_2              (2<<11)  Counter length: 10 bit */
  /* CNTL_3              (3<<11)  Counter length:  8 bit */
  TBCTL |= CNTL_0;

  /* SMCLK is running @1MHz div 8 so the timer Clock is 125000 Hz */
  /* we divide by 2 to keep max value < 0xffff                    */

  TBCCR0 =  (125000 / 2) / (Hz / 2);

  /* Enable the interrupts. */
  TBCCTL0 = CCIE;

  /* Up mode. */
  TBCTL |= MC_1;
}

void timerB7_register_callback(timerB7cb f)
{
  B7_0_cb = f;
}

void timerB7_stop()
{
  TBCTL = 0;
}

interrupt (TIMERB0_VECTOR) timerB0( void )
{
  if (B7_0_cb != 0)
    {
      (*B7_0_cb)();
    }
}
