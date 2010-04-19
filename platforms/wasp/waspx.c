
/**
 *  \file   waspx.c
 *  \brief  Platform for SPI DSP
 *  \author Loic Lemaitre, Antoine Fraboulet
 *  \date   2010
 **/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "arch/common/hardware.h"
#include "arch/msp430/msp430.h"
#include "devices/devices.h"
#include "devices/led/led_dev.h"
#include "devices/ptty/ptty_dev.h"
#include "devices/spidev_dsp/spidev_dsp_dev.h"
#include "devices/uigfx/uigfx_dev.h"
#include "src/options.h"
#include "libgui/ui.h"


#define NO_UPDATE_LOOP_PRINT
#if defined(UPDATE_LOOP_PRINT)
#define HW_UP(x...) VERBOSE(2,x)
#else
#define HW_UP(x...) do {} while(0)
#endif

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
#define SPIDSP1  9
#define LOGO1    10

#define END_DEV           LOGO1
#define BOARD_DEVICE_MAX (END_DEV+1)

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

int devices_options_add()
{
  return 0;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

struct waspx_struct_t {
  uint8_t spidsp_csn;
  uint8_t spidsp_wn;
};

#define SYSTEM_DATA   ((struct waspx_struct_t*)(machine.device[SYSTEM].data))
#define SPIDSP_CSn    SYSTEM_DATA->spidsp_csn
#define SPIDSP_Wn     SYSTEM_DATA->spidsp_wn

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
  machine.device[dev_num].state_size    = sizeof(struct waspx_struct_t);
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
  machine.device_size[SYSTEM]  = sizeof(struct waspx_struct_t);
  machine.device_size[LED1]    = led_device_size();    // Led1
  machine.device_size[LED2]    = led_device_size();    // Led2
  machine.device_size[LED3]    = led_device_size();    // Led3
  machine.device_size[LED4]    = led_device_size();    // Led1
  machine.device_size[LED5]    = led_device_size();    // Led2
  machine.device_size[LED6]    = led_device_size();    // Led3
  machine.device_size[LED7]    = led_device_size();    // Led1
  machine.device_size[LED8]    = led_device_size();    // Led2
  machine.device_size[SPIDSP1] = spidev_dsp_device_size();
  machine.device_size[LOGO1]  = uigfx_device_size();

  /*********************************/
  /* allocate memory               */
  /*********************************/
  res += devices_memory_allocate();

  /*********************************/
  /* create peripherals            */
  /*********************************/

#  define BKG 0xffffff
#  define OFF 0x202020
#  include "waspx.xpm"

  res += system_create           (SYSTEM);
  res += led_device_create       (LED1,    0xee0000,OFF,BKG,"led1");
  res += led_device_create       (LED2,    0xee0000,OFF,BKG,"led2");
  res += led_device_create       (LED3,    0xee0000,OFF,BKG,"led3");
  res += led_device_create       (LED4,    0xee0000,OFF,BKG,"led4");
  res += led_device_create       (LED5,    0xee0000,OFF,BKG,"led5");
  res += led_device_create       (LED6,    0xee0000,OFF,BKG,"led6");
  res += led_device_create       (LED7,    0xee0000,OFF,BKG,"led7");
  res += led_device_create       (LED8,    0xee0000,OFF,BKG,"led8");
  res += spidev_dsp_device_create(SPIDSP1, 0, "waspxdsp");
  res += uigfx_device_create     (LOGO1,   waspx);

  /*********************************/
  /* place peripherals Gui         */
  /*********************************/

  {
    int lw,lh;
    int LW,LH;

    machine.device[LED1 ].ui_get_size(LED1,&lw,&lh);
    machine.device[LOGO1].ui_get_size(LOGO1, &LW, &LH);

    machine.device[LED1 ].ui_set_pos(LED1,    0,  0);
    machine.device[LED2 ].ui_set_pos(LED2,   lw,  0);
    machine.device[LED3 ].ui_set_pos(LED3, 2*lw,  0);
    machine.device[LED4 ].ui_set_pos(LED4, 3*lw,  0);
    machine.device[LED5 ].ui_set_pos(LED5, 4*lw,  0);
    machine.device[LED6 ].ui_set_pos(LED6, 5*lw,  0);
    machine.device[LED7 ].ui_set_pos(LED7, 6*lw,  0);
    machine.device[LED8 ].ui_set_pos(LED8, 7*lw,  0);
    machine.device[LOGO1].ui_set_pos(LOGO1,        0,   0);
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

  SPIDSP_CSn = 0;
  SPIDSP_Wn  = 0;

  REFRESH(LED1);
  REFRESH(LED2);
  REFRESH(LED3);
  REFRESH(LED4);
  REFRESH(LED5);
  REFRESH(LED6);
  REFRESH(LED7);
  REFRESH(LED8);
  REFRESH(LOGO1);
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

  
  HW_UP("......\n");
  /* *************************************************************************** */
  /* MCU -> devices                                                              */
  /* *************************************************************************** */

  /* port 3 :                          */
  /* ========                          */

  /* port 4 :                          */
  /* ========                          */
  /*   P4.7 spidsp ~CS                 */
  /*   P4.6 spidsp ~M                  */
  /*   P4.5 spidsp ~W                  */
  if (msp430_digiIO_dev_read(PORT4,&val8))
    {
      HW_DMSG("waspx:write port4, value = %02x\n", val8);
      machine.device[SPIDSP1].write(SPIDSP1, 
				    SPIDEV_DSP_S | SPIDEV_DSP_W | SPIDEV_DSP_M, 
				    (BIT(val8,7) << SPIDEV_DSP_S_SHIFT) | 
				    (BIT(val8,5) << SPIDEV_DSP_W_SHIFT) |
				    (BIT(val8,6) << SPIDEV_DSP_M_SHIFT) );
      SPIDSP_CSn = BIT(val8,7);
      SPIDSP_Wn  = BIT(val8,5);
    }

  /* port 6 :                          */
  /* ========                          */
  /*   P6.x leds                       */
  if (msp430_digiIO_dev_read(PORT6,&val8))
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

  /* SPI to DSP / configuration */
  switch (MCU.usart0.mode)
    {
    case USART_MODE_SPI:
      if (msp430_usart0_dev_read_spi(&val8))
	{
	  HW_UP("waspx:write spi0, value = %02x == slave\n", val8);
	  machine.device[SPIDSP1].write(SPIDSP1, SPIDEV_DSP_D, val8);
	  etracer_slot_access(0x0, 1, ETRACER_ACCESS_WRITE, ETRACER_ACCESS_BYTE, 
			      ETRACER_ACCESS_LVL_SPI0, 0);
	}      
      break; 
    default:
      break;
    }
    

  /* *************************************************************************** */
  /* devices -> MCU                                                              */
  /* *************************************************************************** */

  {
    uint32_t mask, value;
    machine.device[SPIDSP1].read(SPIDSP1, &mask, &value);

    /* input on usart0 line :: SPI device is slave */
    if ((mask & SPIDEV_DSP_D) != 0)
      {
	if (MCU.usart0.mode != USART_MODE_SPI)
	  {
	    ERROR("waspx:devices: read data for SPI0 while not in SPI mode ?\n");
	  }
	HW_UP("waspx:read spi0, value = %02x == slave\n", (value & SPIDEV_DSP_D) >> SPIDEV_DSP_D_SHIFT);
	msp430_usart0_dev_write_spi((value & SPIDEV_DSP_D) >> SPIDEV_DSP_D_SHIFT);

	/*
	etracer_slot_access(0x0, 1, ETRACER_ACCESS_READ, 
			    ETRACER_ACCESS_BYTE, ETRACER_ACCESS_LVL_SPI0, 0);
	*/
      }

    /* input on usart1 line :: SPI device is master */
    if ((mask & SPIDEV_DSP2_D) != 0)
      {
	if (MCU.usart1.mode != USART_MODE_SPI)
	  {
	    ERROR("waspx:devices: read data for SPI1 while not in SPI mode ?\n");
	  }
	HW_UP("waspx:write spi1, value = %02x == master\n", (value & SPIDEV_DSP2_D) >> SPIDEV_DSP2_D_SHIFT);
	msp430_usart1_dev_write_spi((value & SPIDEV_DSP2_D) >> SPIDEV_DSP2_D_SHIFT);

	etracer_slot_access(0x0, 1, ETRACER_ACCESS_WRITE, ETRACER_ACCESS_BYTE, 
			      ETRACER_ACCESS_LVL_SPI1, 0);
	/*
	etracer_slot_access(0x0, 1, ETRACER_ACCESS_READ, ETRACER_ACCESS_BYTE, 
			      ETRACER_ACCESS_LVL_SPI1, 0);
	*/
      }

  }

  HW_UP(".up\n");

  /* input on UI */
  ui_default_input("waspx:");

  /* *************************************************************************** */
  /* update                                                                      */
  /* *************************************************************************** */
  LIBSELECT_UPDATE();
  LIBWSNET_UPDATE();

  UPDATE(SPIDSP1);

  ui_refresh(refresh);

  return res;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */
