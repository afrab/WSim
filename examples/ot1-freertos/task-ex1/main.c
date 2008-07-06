/*

	***************************************************************************
	See http://www.FreeRTOS.org for documentation, latest information, license 
	and contact details.  Please ensure to read the configuration and relevant 
	port sections of the online documentation.
	***************************************************************************
*/



/* Misc includes. */
#include "hardware_msp430x449.h"
#include "hardware_setre_1_0.h"

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"

/* Demo task priorities. */

#define mainLED_TASK_PRIORITY			( tskIDLE_PRIORITY + 1 )

#define DELAY                                   ( ( portTickType ) 10 )
#define LED_DELAY    (2  * DELAY)
#define FLASH_DELAY  (13 * DELAY)

/* Functions definition */

static void prvSetupHardware( void );
static void vLedsChecks( void *pvParameters );
void TacheA(void *pvParameters);
void output_7Seg(int i,char a); 


/* Global declaration */
xTaskHandle xstoptaskhandler;
//int etat;

/*-----------------------------------------------------------*/

/*
 * Start the demo application tasks - then start the real time scheduler.
*/

int main( void )
{

	/* Setup the hardware ready for the demo. */
	prvSetupHardware();
	xTaskCreate( TacheA, NULL, configMINIMAL_STACK_SIZE, NULL, mainLED_TASK_PRIORITY+1, NULL);	

	/* Start the 'Flash Led' task which is defined in this file. */
	xTaskCreate( vLedsChecks, NULL, configMINIMAL_STACK_SIZE, NULL, mainLED_TASK_PRIORITY, &xstoptaskhandler);	

	/* Start the scheduler. */
	vTaskStartScheduler();

	/* As the scheduler has been started the demo applications tasks will be
	executing and we should never get here! */
	return 0;
}

/*-----------------------------------------------------------*/

static void vLedsChecks( void *pvParameters )
{ 
  for( ;; )
    {
      output_7Seg(1,'b'); 
      
      LED_OUT=255 ;
      vTaskDelay( LED_DELAY );
      
      /* Flash! */
      
      LED_OUT=0   ;   
      vTaskDelay( LED_DELAY );
    }
}


void TacheA( void *pvParameters )
{
  for( ;; )
    {
      output_7Seg(8,'b'); 
      
      vTaskDelay( FLASH_DELAY );
      vTaskSuspend(xstoptaskhandler);   
      
      /* Flash! */
      output_7Seg(8,'b'); 
      
      vTaskDelay( FLASH_DELAY );
      vTaskResume(xstoptaskhandler);
    }
}

/*-----------------------------------------------------------*/

static void prvSetupHardware( void )
{
    P1OUT  = 0;           // Init output data of port1 0:
    P2OUT  = 0;
    P3OUT  = 0;
    P4OUT  = 0;
    P5OUT  = 0;
    P6OUT  = 0;

    P1SEL  = 0;           // Select port or module -function on port1
    P2SEL  = 0;           // 0 : I/O function 
    P3SEL  = 0;           // 1 : peripheral module
    P4SEL  = 0;
    P5SEL  = 0;
    P6SEL  = 0;

    P1DIR  = 0x00;        // Init port direction register of port1
    P2DIR  = 0xff;        // 0 : input direction
    P3DIR  = 0xff;        // 1 : output direction
    P4DIR  = 0xff;
    P5DIR  = 0xff;
    P6DIR  = 0xff;
    
    P1IES  = 0xff;        // Interrupt edge select
    P2IES  = 0x00;        // 0: low to high, 1 : high to low

    P1IE   = 0xff;        // Interrupt enable
    P2IE   = 0x00;        // 0:disable 1:enable
}

/*-----------------------------------------------------------*/

interrupt (PORT1_VECTOR) prvbutton( void )
{
  static int etat=0;

  if (etat==0) {
    output_7Seg(etat,'b');
    etat=1;
  } 
  else {
    output_7Seg(etat,'b');
    etat=0;
  }  

  P1IFG = 0;
}


void output_7Seg(int i,char a)  // display value "i" on either 'l'eft or 'r'ight or 'b'oth
{
  //  LED_OUT  = bitnumbers[i % 11] ; // leds
  SEG_OUT  = bitnumbers[i % 11] ; // 7seg
  switch (a)
  {
	case 'l': 
		{	P6OUT |=  	0x10;	// bit 6.4 buffer latch 
			P6OUT &=  	~0x10; 	// bit 6.4 buffer latch 
		}
	case 'r': 
		{	P6OUT |= 	0x08; 	// bit 6.3 buffer latch 
			P6OUT &=  	~0x08; 	// bit 6.4 buffer latch 
		}
	case 'b': 
		{	P6OUT |= 	0x18; 	// bit 6.3 & 6.4  buffer latch 
			P6OUT &=  	~0x18; 	// bit 6.3 & 6.4 buffer latch 
		}
	default:;
	break;
  }
}


