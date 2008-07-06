This program can be compiled using FreeRTOS 3.2.4 and must be run
on the ot1 platform.


/FreeRTOSConfig.h
=================
#define configCPU_CLOCK_HZ		( ( unsigned portLONG ) 799539 )
#define configTICK_RATE_HZ		( ( portTickType ) 50 )

configTICK_RATE_HZ represents the scheduler timing unit

/main.c
=======
#define DELAY                           ( ( portTickType ) 10 )

#define LED_DELAY    (2  * DELAY)
#define FLASH_DELAY  (13 * DELAY)

To show timing precision you can changes the values and look at 
the effects on the 7 segments and leds

Leds are turned on and off every LED_DELAY
7seg are flashed every FLASH_DELAY
