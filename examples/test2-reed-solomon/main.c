
#include <io.h>
#include <signal.h>
#include <iomacros.h>
#include <stdio.h>
#include <string.h>

#include "uart0.h"
#include "nn4.h"

/**********************
 * Delay function.
 **********************/

#define DELAY 0x100

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
 * Timer setting : 1Hz
 **********************/

#define TA_FREQUENCY_HZ  32768
#define TA_RATE_HZ           1
#define TA_DIV            ID_0

void
set_timer()
{
  /* Ensure the timer is stopped. */
  TACTL = 0;
  /* Run the timer of the ACLK. */
  TACTL = TASSEL_1;
  /* Clear everything to start with. */
  TACTL |= TACLR;
  /* Set the compare match value according to the tick rate we want. */
  TACCR0 = TA_FREQUENCY_HZ / TA_RATE_HZ;
  /* Enable the interrupts. */
  TACCTL0 = CCIE;
  /* Freq divider. */
  TACTL |= TA_DIV;
  /* Start up clean. */
  TACTL |= TACLR;
  /* Up mode. */
  TACTL |= MC_1;
}

/**********************
 * Leds 
 **********************/

#define LED_OUT   P1OUT

int led_state;

interrupt (TIMERA0_VECTOR) prvTickISR( void ); // __attribute__ ( ( naked ) );
interrupt (TIMERA0_VECTOR) prvTickISR( void )
{
  led_state <<= 1;
  if (led_state > 0x80)
    {
      led_state = 1;
    }
  LED_OUT = led_state;
}

/**********************
 * printf 
 **********************/

int putchar(int c)
{
  return uart0_putchar(c);
}

void print_array(char* name, dtype *d, int size)
{
  int i;
  printf(name);
  for(i=0; i < size; i++)
    {
      printf("%d ",d[i]);
    }
  printf("\n");
}

/**********************
 * main
 **********************/

int main(void) 
{
  /* leds */
  P1DIR = 0xff;
  P3DIR = 0x01;
  led_state = 1;

  set_timer();
  uart0_init();
  eint();        

  printf("Reed Solomon test program ready.\n");

  while (1)
    {
      
	int res;
	dtype data[] = {1,2,3,15,5,15,11,15,15,15,0,0,0,0,0,0};
	dtype erasepos[5];
	
	print_array("0: data  ",data,16);
	
	/* Encode gata in GF(15,10) */
	encode_rs(&data[0],&data[10]);
	print_array("1: data  ",data,16);
	
	erasepos[0] = 5;
	data[0] = 3;
	data[11]= 0;
	data[5] = 0;
	/* Rstore data */
	res = eras_dec_rs(data, erasepos, 1);

	print_array("2: data  ",data,16);
	print_array("2: erase ",erasepos,5);
	printf("2: res = %d\n",res);
	printf("\n");

	/* P3.1 can halt simulation on 'test' platform */
	// P3OUT = 1; 
    }
}
