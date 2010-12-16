
/**
 *  \file   ez430.c
 *  \brief  Platform devices handling functions, Ti eZ430 platform
 *  \author Antoine Fraboulet
 *  \date   2005
 **/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __DEBUG
#  undef DEBUG
#endif

#include "arch/common/hardware.h"
#include "arch/msp430/msp430.h"

#include "devices/devices.h"
#include "devices/led/led_dev.h"
#include "devices/ptty/ptty_dev.h"
#include "devices/uigfx/uigfx_dev.h"
#include "src/options.h"

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

/* ****************************************
 * platform description : ez430
 *
 * msp430f2013
 * Port 1.0 blue led 
 *
 * ***************************************/

#define SYSTEM          0
#define LED1            1
#define LOGO1           2

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

struct ez430_struct_t {
};

#define SYSTEM_DATA      ((struct ez430_struct_t*)(machine.device[SYSTEM].data))

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
  machine.device[dev_num].state_size    = sizeof(struct ez430_struct_t);
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
  res += msp430_mcu_create(0 /* xin_freq*/, 12*1000*1000 /* xt2in_freq */);


  /*********************************/
  /* fix peripheral sizes          */
  /*********************************/
  machine.device_max           = BOARD_DEVICE_MAX;
  machine.device_size[SYSTEM]  = sizeof(struct ez430_struct_t);
  machine.device_size[LED1]    = led_device_size();  /* Led1 */
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
#  include "ez430.xpm"
#  define IMG ez___

  res += system_create          (SYSTEM);
  res += led_device_create      (LED1, 0x0000ee, OFF, BKG, "led");      /* RVB */
  res += uigfx_device_create    (LOGO1, IMG);

  /*********************************/
  /* place peripherals Gui         */
  /*********************************/

  {
    int led_w,led_h;
    int log_w,log_h;

    machine.device[LED1].ui_get_size  (LED1,  &led_w, &led_h);
    machine.device[LOGO1].ui_get_size (LOGO1, &log_w, &log_h);

    machine.device[LED1].ui_set_pos   (LED1,   0,  0);
    machine.device[LOGO1].ui_set_pos  (LOGO1,  0,  0);
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
int devices_reset_post(void)
{
  int refresh = 0;

  REFRESH(LOGO1);
  REFRESH(LED1);
  ui_refresh(refresh);
  return 0;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

int devices_update()
{
  int res     = 0;
  int refresh = 0;
  uint8_t  val8;

  /* *************************************************************************** */
  /* MCU -> devices                                                              */
  /* *************************************************************************** */

  if (msp430_digiIO_dev_read(PORT1,&val8))
    {
      { /* led on port 1.0 */
	machine.device[LED1].write(LED1,LED_DATA,BIT(val8,0));
	UPDATE(LED1);
	REFRESH(LED1);
      }
    }

  /* *************************************************************************** */
  /* devices -> MCU                                                              */
  /* *************************************************************************** */

  /* input on UI */
  ui_default_input("ez430:devices");
  
  /* *************************************************************************** */
  /* update                                                                      */
  /* *************************************************************************** */
  LIBSELECT_UPDATE();
  LIBWSNET_UPDATE();

  ui_refresh(refresh);

  return res;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */
