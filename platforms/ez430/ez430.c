
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

#define LED1            0
#define FREE_DEVICE_MAX 1

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
  machine.device_max           = FREE_DEVICE_MAX;
  machine.device_size[LED1]    = led_device_size();  /* Led1 */

  /*********************************/
  /* allocate memory               */
  /*********************************/
  res += devices_memory_allocate();
  /*********************************/
  /* create peripherals            */
  /*********************************/
  res += led_device_create      (LED1,0,0xee,0, "led");      /* RVB */

  /*********************************/
  /* place peripherals Gui         */
  /*********************************/
  {
    int lw,lh;
    #define SHIFT 50
    machine.device[LED1].ui_get_size(LED1,&lw,&lh);
    machine.device[LED1].ui_set_pos(LED1, SHIFT + 0,  0);
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
  return 0;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

int devices_update()
{
  int res = 0;
  int ev;
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
      {
	machine.device[LED1].write(LED1,LED_DATA,BIT(val8,LED1));
	UPDATE(LED1);
	REFRESH(LED1);
      }
    }

  /* *************************************************************************** */
  /* devices -> MCU                                                              */
  /* *************************************************************************** */

  /* input on UI */
  switch ((ev = ui_getevent()))
    {
    case UI_EVENT_USER:
      {
	uint8_t b = 0;
	// the reset button is negated
	//  if (machine.ui.val & UI_BUTTON_1)
	//  msp430_reset_pin((machine.ui.b_down & UI_BUTTON_1) ? 0 : 1);
	
	// P0.012 buttons 1 2 and 3 -> p6 3 4 5
	b |= (machine.ui.b_down & UI_BUTTON_1) ? 0x00 : 0x10;
	b |= (machine.ui.b_down & UI_BUTTON_2) ? 0x00 : 0x20;
	b |= (machine.ui.b_down & UI_BUTTON_3) ? 0x00 : 0x40;
	b |= (machine.ui.b_down & UI_BUTTON_4) ? 0x00 : 0x80;

	msp430_digiIO_dev_write(0, b, 0xf0);
      }
      break;
    case UI_EVENT_QUIT:
      HW_DMSG_UI("  devices UI event QUIT\n");
      MCU_SIGNAL = SIG_UI;
      break;
    case UI_EVENT_NONE:
      break;
    default:
      ERROR("devices: unknown ui event\n");
      break;
    }

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
