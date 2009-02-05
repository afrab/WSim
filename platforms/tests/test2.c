
/**
 *  \file   test2.c
 *  \brief  Platform devices handling functions, test2 platform
 *  \author Antoine Fraboulet
 *  \date   2005
 **/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "arch/common/hardware.h"
#include "arch/msp430/msp430.h"
#include "devices/devices.h"
#include "devices/led/led_dev.h"
#include "devices/ptty/ptty_dev.h"
#include "src/options.h"

#if defined(GUI)
#include "libgui/ui.h"
#define UI_EVENT_SKIP 1000
#endif

/* ****************************************
 * platform description 
 *
 * Port 1 led array
 * ======
 *
 * ***************************************/


/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

#define SYSTEM   0
#define LED1     1
#define LED2     2
#define LED3     3
#define LED4     4
#define LED5     5
#define LED6     6
#define LED7     7
#define LED8     8
#define SERIAL0  9
#define SERIAL1  10

#define FREE_DEVICE_MAX 11

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

int devices_options_add()
{
  ptty_add_options(SERIAL0,0,"serial0");
  ptty_add_options(SERIAL1,1,"serial1");
  return 0;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

struct test2_struct_t {
};

#define SYSTEM_DATA   ((struct test2_struct_t*)(machine.device[SYSTEM].data))

int system_reset (int UNUSED dev) 
{ 
  return 0; 
}

int system_delete(int UNUSED dev) 
{ 
  return 0; 
}

int system_create(int dev_num)
{
  machine.device[dev_num].reset         = system_reset;
  machine.device[dev_num].delete        = system_delete;
  machine.device[dev_num].state_size    = sizeof(struct test2_struct_t);
  machine.device[dev_num].name          = "System Platform";

  return 0;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

int devices_create()
{
  int res = 0;

  /*********************************/
  /* MSP430 MCU                    */
  /*********************************/
  res += msp430_mcu_create(32768 /* xin_freq*/, 8000000 /* xt2in_freq */);

  /*********************************/
  /* fix peripheral sizes          */
  /*********************************/
  machine.device_max           = FREE_DEVICE_MAX;
  machine.device_size[SYSTEM]  = sizeof(struct test2_struct_t);
  machine.device_size[LED1]    = led_device_size();    // Led1
  machine.device_size[LED2]    = led_device_size();    // Led2
  machine.device_size[LED3]    = led_device_size();    // Led3
  machine.device_size[LED4]    = led_device_size();    // Led1
  machine.device_size[LED5]    = led_device_size();    // Led2
  machine.device_size[LED6]    = led_device_size();    // Led3
  machine.device_size[LED7]    = led_device_size();    // Led1
  machine.device_size[LED8]    = led_device_size();    // Led2
  machine.device_size[SERIAL0] = ptty_device_size();
  machine.device_size[SERIAL1] = ptty_device_size();

  /*********************************/
  /* allocate memory               */
  /*********************************/
  res += devices_memory_allocate();

  /*********************************/
  /* create peripherals            */
  /*********************************/
  res += led_device_create      (LED1,0xee,0,0,"led1");
  res += led_device_create      (LED2,0xee,0,0,"led2");
  res += led_device_create      (LED3,0xee,0,0,"led3");
  res += led_device_create      (LED4,0xee,0,0,"led4");
  res += led_device_create      (LED5,0xee,0,0,"led5");
  res += led_device_create      (LED6,0xee,0,0,"led6");
  res += led_device_create      (LED7,0xee,0,0,"led7");
  res += led_device_create      (LED8,0xee,0,0,"led8");
  res += ptty_device_create     (SERIAL0,0);
  res += ptty_device_create     (SERIAL1,1);

  /*********************************/
  /* place peripherals Gui         */
  /*********************************/
#if defined(GUI)
  {
    int lw,lh;

    machine.device[LED1].ui_get_size(LED1,&lw,&lh);

    machine.device[LED1].ui_set_pos(LED1,    0,  0);
    machine.device[LED2].ui_set_pos(LED2,   lw,  0);
    machine.device[LED3].ui_set_pos(LED3, 2*lw,  0);
    machine.device[LED4].ui_set_pos(LED4, 3*lw,  0);
    machine.device[LED5].ui_set_pos(LED5, 4*lw,  0);
    machine.device[LED6].ui_set_pos(LED6, 5*lw,  0);
    machine.device[LED7].ui_set_pos(LED7, 6*lw,  0);
    machine.device[LED8].ui_set_pos(LED8, 7*lw,  0);
  }
#endif
  /*********************************/
  /* end of platform specific part */
  /*********************************/
  return res;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

/* this function is called after devices reset    */
/* devices init conditions should be written here */
int devices_reset_post()
{
  return 0;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

int devices_update()
{
  int res = 0;
  int refresh = 0;
  uint8_t  val8;

  /* *************************************************************************** */
  /* MCU -> devices                                                              */
  /* *************************************************************************** */

  /* port 3 :                          */
  /* ========                          */
  /*   P3.7 urxd1 : serial             */
  /*   P3.6 utxd1 : serial             */
  /*   P3.5 urxd0                      */
  /*   P3.4 utxd0                      */ 

  /* port 5 :                          */
  /* ========                          */
  /*   P5.7 NC                         */
  /*   P5.6 led 3                      */
  /*   P5.5 led 2                      */
  /*   P5.4 led 1                      */
  /*   P5.0 NC                         */
  if (msp430_digiIO_dev_read(PORT1,&val8))
    {
      int i;
      for(i = LED1; i <= LED8 ; i++)
	{
	  machine.device[i].write(i,LED_DATA,BIT(val8,(i - LED1)));
	  UPDATE(i);
	  REFRESH(i);
	}
    }

  /* Usart0  port 3                    */
  /* ==============                    */
  if (msp430_usart0_dev_read_uart(&val8))
    {
      machine.device[SERIAL0].write(SERIAL0, PTTY_D, val8);
    }


  /* Usart1  port 3                    */
  /* ==============                    */
  if (msp430_usart1_dev_read_uart(&val8))
    {
      machine.device[SERIAL1].write(SERIAL1, PTTY_D, val8);
    }
    

  /* *************************************************************************** */
  /* devices -> MCU                                                              */
  /* *************************************************************************** */

#define READ_DEV_TO_UART(DEV,USART,MASK)                    \
do                                                          \
  {                                                         \
    if (MCU.USART.uxrx_shift_empty == 1)                    \
      {                                                     \
         machine.device[ DEV ].read( DEV ,&mask,&value);    \
         if ((mask & MASK) != 0)                            \
           {                                                \
	     msp430_##USART##_dev_write_uart(value & MASK); \
           }                                                \
      }                                                     \
  }                                                         \
while(0)


  /* input on usart0 line */
  switch (MCU.usart0.mode)
    {
    case USART_MODE_UART:
      {
	uint32_t mask, value;
	READ_DEV_TO_UART(SERIAL0,usart0,PTTY_D);
      }
      break;
    case USART_MODE_SPI:
    case USART_MODE_I2C:
      ERROR("Bad Uart mode\n");
      break;
    }

  /* input on usart1 line */
  switch (MCU.usart1.mode)
    {
    case USART_MODE_UART:
      {
	uint32_t mask, value;
	READ_DEV_TO_UART(SERIAL1,usart1,PTTY_D);
      }
      break;
    case USART_MODE_SPI:
    case USART_MODE_I2C:
      ERROR("Bad Uart mode\n");
      break;
    }


  /* input on UI */
#if defined(GUI)
  {
    /* poll event every */
    static int loop_count = UI_EVENT_SKIP;
    if ((loop_count--) == 0)
      {
	int ev;
	loop_count = UI_EVENT_SKIP;
	switch ((ev = ui_getevent()))
	  {
	  case UI_EVENT_QUIT:
	    HW_DMSG_UI("  devices UI event QUIT\n");
	    mcu_signal_set(SIG_UI);
	    break;
	  case UI_EVENT_USER:
	  case UI_EVENT_NONE:
	    break;
	  default:
	    ERROR("devices: unknown ui event\n");
	    break;
	  }
      }
  }
#endif

  /* *************************************************************************** */
  /* update                                                                      */
  /* *************************************************************************** */
  LIBSELECT_UPDATE();
  LIBWSNET_UPDATE();

  UPDATE(SERIAL0);
  UPDATE(SERIAL1);

#if defined(GUI)
  if (refresh) {
    ui_refresh();
  }
#endif

  return res;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */
