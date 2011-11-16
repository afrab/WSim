#include <io.h>
#include <signal.h>
#include <iomacros.h>
#include <stdio.h>
#include <string.h>
#include <msp430x14x.h>

/**********************
 * Delay function.
 **********************/

#define DELAY 0x100
#define NB_MAX_INTERRUPT 16

void delay(unsigned int d) 
{
  unsigned int i,j;
  for(j=0; j < 0xff; j++)
    {
      for (i = 0; i<d; i++) 
	{
	  nop();
	  nop();
	}
    }
}


/********************************************************/
/********************************************************/
/********************************************************/

#define SET_LED_DIR        P2DIR |= BIT1     //Direction port P2.1 (LED) = OUT
#define LED_OUT            P2OUT             //Registre OUT port P2.1 (LED)   
#define STATUS_LED_ON      P2OUT &= ~BIT1    //P2.1 (LED) = 0
#define STATUS_LED_OFF     P2OUT |= BIT1     //P2.1 (LED) = 1

uint8_t led_state;
int nbInterrupt;

/********************************************************/
/********************************************************/
/********************************************************/

#define NBCYCLE  65535

void timerA1_start(int DIV)
{
  /* Macro defined in $MSP430/include/msp430/ */
  /* Ensure the timer is stopped. */
  TACTL = 0;
  /* Run the timer of the ACLK. */
  TACTL = TASSEL_1;
  /* Clear everything to start with. */
  TACTL |= TACLR;
  /* Set the compare match value according to the tick rate we want. */
  TACCR0 = NBCYCLE/DIV;
  /* Enable the interrupts. */
  TACCTL0 = CCIE;
  /* Freq divider: no */
  TACTL |= ID_0;
  /* Up mode. */
  TACTL |= MC_1;
}

void timerA1_stop()
{
  TACTL = 0;
}

interrupt (TIMERA0_VECTOR) TickISR( void ); // __attribute__ ( ( naked ) );
interrupt (TIMERA0_VECTOR) TickISR( void )
{
  nbInterrupt++;
  if (nbInterrupt == NB_MAX_INTERRUPT)
    {/* switch every NB_MAX_INTERRUPT * MS */
      nbInterrupt = 0;
      if (led_state == 0)
	{
	  STATUS_LED_ON;
	  led_state = 1;
	}
      else
	{
	  STATUS_LED_OFF;
	  led_state = 0;
	}
    }
}
/********************************************************/
/********************************************************/
/********************************************************/


/********************************************************/
/********************************************************/
/********************************************************/

int main(void) 
{
  WDTCTL = WDTPW + WDTHOLD;


  BCSCTL1 |= XTS;        //ACLK = LFTX1 = High Freq 8Mhz
  BCSCTL1 |= DIVA_3;     //ACLK = 1 Mhz

  eint();

  led_state = 0;
  nbInterrupt = 0;

  SET_LED_DIR;
  STATUS_LED_OFF;
 
  //timerA counts 65635 cycles
  timerA1_start(1);
  delay(10000);
  timerA1_stop();
  //timerA counts 6563 cycles
  timerA1_start(10);


  return 0;
}
