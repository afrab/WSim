
/**
 *  \file   test3.c
 *  \brief  Platform for spi master devices test, test3 platform
 *  \author Loic Lemaitre
 *  \date   2009
 **/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "arch/common/hardware.h"
#include "arch/msp430/msp430.h"
#include "devices/devices.h"
#include "devices/led/led_dev.h"
#include "devices/ptty/ptty_dev.h"
#include "devices/spidev_master/spidev_master_dev.h"
#include "devices/uigfx/uigfx_dev.h"
#include "src/options.h"

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
#define SPIDEV1  11
#define LOGO1    12

#define BOARD_DEVICE_MAX 13

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
  uint8_t spidev_csn;
  uint8_t spidev_wn;
};

#define SYSTEM_DATA   ((struct test2_struct_t*)(machine.device[SYSTEM].data))
#define SPIDEV_CSn  SYSTEM_DATA->spidev_csn
#define SPIDEV_Wn   SYSTEM_DATA->spidev_wn

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
  machine.device_max           = BOARD_DEVICE_MAX;
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
  machine.device_size[SPIDEV1] = spidev_device_size();
  machine.device_size[LOGO1]   = uigfx_device_size();

  /*********************************/
  /* allocate memory               */
  /*********************************/
  res += devices_memory_allocate();

  /*********************************/
  /* create peripherals            */
  /*********************************/

#  define BKG 0xffffff
#  define OFF 0x202020
#  include "wsim.xpm"
#  define WSIM wsim

  res += system_create          (SYSTEM);
  res += led_device_create      (LED1,    0xee0000,OFF,BKG,"led1");
  res += led_device_create      (LED2,    0xee0000,OFF,BKG,"led2");
  res += led_device_create      (LED3,    0xee0000,OFF,BKG,"led3");
  res += led_device_create      (LED4,    0xee0000,OFF,BKG,"led4");
  res += led_device_create      (LED5,    0xee0000,OFF,BKG,"led5");
  res += led_device_create      (LED6,    0xee0000,OFF,BKG,"led6");
  res += led_device_create      (LED7,    0xee0000,OFF,BKG,"led7");
  res += led_device_create      (LED8,    0xee0000,OFF,BKG,"led8");
  res += ptty_device_create     (SERIAL0, 0);
  res += ptty_device_create     (SERIAL1, 1);
  res += spidev_device_create   (SPIDEV1, 0);
  res += uigfx_device_create    (LOGO1,   wsim);

  /*********************************/
  /* place peripherals Gui         */
  /*********************************/

  {
    int lw,lh;
    int LW,LH;

    machine.device[LED1].ui_get_size(LED1,&lw,&lh);
    machine.device[LOGO1].ui_get_size(LOGO1, &LW, &LH);

    machine.device[LED1].ui_set_pos(LED1,    0,  0);
    machine.device[LED2].ui_set_pos(LED2,   lw,  0);
    machine.device[LED3].ui_set_pos(LED3, 2*lw,  0);
    machine.device[LED4].ui_set_pos(LED4, 3*lw,  0);
    machine.device[LED5].ui_set_pos(LED5, 4*lw,  0);
    machine.device[LED6].ui_set_pos(LED6, 5*lw,  0);
    machine.device[LED7].ui_set_pos(LED7, 6*lw,  0);
    machine.device[LED8].ui_set_pos(LED8, 7*lw,  0);
    machine.device[LOGO1].ui_set_pos (LOGO1,        0,   0);
  }

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
  int refresh = 0;

  SPIDEV_CSn = 0;
  SPIDEV_Wn  = 0;

  REFRESH(LOGO1);
  REFRESH(LED1);
  REFRESH(LED2);
  REFRESH(LED3);
  REFRESH(LED4);
  REFRESH(LED5);
  REFRESH(LED6);
  REFRESH(LED7);
  REFRESH(LED8);
  ui_refresh(refresh);
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

  /* port 4 :                          */
  /* ========                          */
  /*   P4.7 spidev ~CS                 */
  /*   P4.6 spidev ~M                  */
  /*   P4.5 spidev ~W                  */
  if (msp430_digiIO_dev_read(PORT4,&val8))
    {
      HW_DMSG("platform test3: Port4, value = %02x\n", val8);
      machine.device[SPIDEV1].write(SPIDEV1, 
				    SPIDEV_S | SPIDEV_W | SPIDEV_M, 
				    (BIT(val8,7) << SPIDEV_S_SHIFT) | 
				    (BIT(val8,5) << SPIDEV_W_SHIFT) |
				    (BIT(val8,6) << SPIDEV_M_SHIFT) );
      SPIDEV_CSn = BIT(val8,7);
      SPIDEV_Wn  = BIT(val8,5);
    }

  /* port 5 :                          */
  /* ========                          */
  /*   P5.x leds                       */
  if (msp430_digiIO_dev_read(PORT5,&val8))
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
  if (msp430_usart0_dev_read_spi(&val8))
    {
      machine.device[SPIDEV1].write(SPIDEV1, SPIDEV_D, val8);
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
    if (msp430_##USART##_dev_write_uart_ok())		    \
      {                                                     \
         machine.device[ DEV ].read( DEV ,&mask,&value);    \
         if ((mask & MASK) != 0)                            \
           {                                                \
	     msp430_##USART##_dev_write_uart(value & MASK); \
           }                                                \
      }                                                     \
  }                                                         \
while(0)

#define READ_DEV_TO_SPI(DEV,USART,MASK)			    \
do                                                          \
  {                                                         \
         machine.device[ DEV ].read( DEV ,&mask,&value);    \
         if ((mask & MASK) != 0)                            \
           {                                                \
	     msp430_##USART##_dev_write_spi(value & MASK);  \
           }                                                \
  }                                                         \
while(0)


  /* input on usart0 line */
  uint32_t mask, value;
  switch (MCU.usart0.mode)
    {
    case USART_MODE_UART:
      {
	READ_DEV_TO_UART(SERIAL0,usart0,PTTY_D);
      }
      break;
    case USART_MODE_SPI:
      {
	READ_DEV_TO_SPI(SPIDEV1,usart0,SPIDEV_D);
	break;
      }
    case USART_MODE_I2C:
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
      break;
    case USART_MODE_I2C:
      break;
    }


  /* input on UI */
  ui_default_input("test3:");

  /* *************************************************************************** */
  /* update                                                                      */
  /* *************************************************************************** */
  LIBSELECT_UPDATE();
  LIBWSNET_UPDATE();

  UPDATE(SERIAL0);
  UPDATE(SERIAL1);
  UPDATE(SPIDEV1);

  ui_refresh(refresh);

  return res;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */
