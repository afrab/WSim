
/**
 *  \file   ot2006.c
 *  \brief  Platform devices handling functions, OT Setre  2006 edition
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
#include "devices/7seg/7seg_dev.h"
#include "devices/led/led_dev.h"
#include "devices/gdm1602a/gdm_dev.h"
#include "devices/ptty/ptty_dev.h"
#include "src/options.h"

/**
 * OT Setre platform 1 / 2006
 *
 *  port 1 : 4 lower bits = buttons
 *  port 2 : NC
 *  port 3 : 7 seg data out 
 *  port 4 : LED out
 *  port 5 : LCD data
 *  port 6 : Control
 *    p6.7 : LCD.RW
 *    p6.6 : LCD.RS
 *    p6.5 : LCD.E
 *    p6.4 : LE2 : commande 7seg U1
 *    p6.3 : LE1 : commande 7seg U2
 **/

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

#define SEG0     0
#define SEG1     1
#define LED1     2
#define LED2     3
#define LED3     4
#define LED4     5
#define LED5     6
#define LED6     7
#define LED7     8
#define LED8     9
#define LCD     10
#define SERIAL0 11
#define OT_DEVICE_MAX 12

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

static struct moption_t ptty0_opt = {
  .longname    = "serial0",
  .type        = required_argument,
  .helpstring  = "serial fifo 0",
  .value       = NULL
};

int devices_options_add()
{
  options_add(&ptty0_opt);
  return 0;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

int devices_create()
{
  int i;
  int res = 0;

  /*********************************/
  /* MSP430 MCU                    */
  /*********************************/
  res += msp430_mcu_create(32768 /* xin_freq*/, 8000000 /* xt2in_freq */);

  /*********************************/
  /* begin platform specific part  */
  /*********************************/
  machine.device_max           = OT_DEVICE_MAX;
  machine.device_size[SEG0]    = sevenseg_device_size();
  machine.device_size[SEG1]    = sevenseg_device_size();
  machine.device_size[LED1]    = led_device_size();
  machine.device_size[LED2]    = led_device_size();
  machine.device_size[LED3]    = led_device_size();
  machine.device_size[LED4]    = led_device_size();
  machine.device_size[LED5]    = led_device_size();
  machine.device_size[LED6]    = led_device_size();
  machine.device_size[LED7]    = led_device_size();
  machine.device_size[LED8]    = led_device_size();
  machine.device_size[LCD]     = gdm_device_size();
  machine.device_size[SERIAL0] = ptty_device_size();

  /*********************************/
  /* end of platform specific part */
  /*********************************/
  res += devices_memory_allocate();

  /*********************************/
  /* begin platform specific part  */
  /*********************************/
  res += sevenseg_device_create(SEG0);
  res += sevenseg_device_create(SEG1);
  for(i=LED1; i <= LED8; i++) // 8 leds
    {
      res += led_device_create(i,0xee,0,0);
    }
  res += gdm_device_create(LCD);
  res += ptty_device_create(SERIAL0,0,ptty0_opt.value);

  /*********************************/
  /* end of platform specific part */
  /*********************************/
  {
    int sw,sh,lw,lh;

    machine.device[LED1].ui_get_size(LED1,&lw,&lh);
    machine.device[SEG0].ui_get_size(SEG0,&sw,&sh);

    machine.device[SEG0].ui_set_pos(SEG0,  0,  0);
    machine.device[SEG1].ui_set_pos(SEG1, sw,  0);
    for(i=LED1; i <= LED8 ; i++)
      {
	machine.device[i].ui_set_pos(i, 2*sw + (i-LED1)*lw, 0);
      }
    machine.device[LCD].ui_set_pos(LCD,0,sh);
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
  return 0;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

int devices_update()
{
  int ev;
  int res = 0;
  int refresh = 0;

  uint8_t   val8;
  uint32_t  mask32,val32;
  static uint8_t seg_latch;

  /* ************************* */
  /* MCU -> devices            */
  /* ************************* */

  // port 1 : no output, buttons input
  // port 2 : serial 0
  //   2.4: utxd0
  //   2.5: urxd0 

  // port 3 : 
  //          7seg data out, depends on p6.4 and port6.3
  if (msp430_digiIO_dev_read(PORT3,&val8))
    {
      seg_latch = val8;
    }
  
  // port 4 : 
  //          8 leds, full 8 bits
  if (msp430_digiIO_dev_read(PORT4,&val8))
    {
      int i;
      for(i=LED1; i <= LED8; i++)
	{
	  machine.device[i].write(i,LED_DATA,BIT(val8,(i - LED1)));
	  UPDATE(i);
	  REFRESH(i);
	}
    }

  // port 5 : 
  //          LCD data
  if (msp430_digiIO_dev_read(PORT5,&val8))
    {
      machine.device[LCD].write(LCD,0x00ff,val8);
      UPDATE(LCD);
      REFRESH(LCD);
    }

  // port 6 : 
  //          LCD Contrast, RS, R/W, Enable : 4 bits
  //          7seg chip select on bit p6.3 and p6.4
  if (msp430_digiIO_dev_read(PORT6,&val8))
    {
      machine.device[LCD].write(LCD,0x0f00,(val8 >> 5) & 0x07); 
      UPDATE(LCD);
      REFRESH(LCD);

      if (val8 & 0x10) // p6.4
	{
	  machine.device[SEG0].write(SEG0,0,seg_latch);
	  UPDATE(SEG0);
	  REFRESH(SEG0);
	}

      if (val8 & 0x08) // p6.3
	{
	  machine.device[SEG1].write(SEG1,0,seg_latch);
	  UPDATE(SEG1);
	  REFRESH(SEG1);
	}
    }

  if (msp430_usart0_dev_read_uart(&val8))
    {
      machine.device[SERIAL0].write(SERIAL0, PTTY_D, val8);
    }

  /* ************************* */
  /* devices -> MCU            */
  /* ************************* */

  /* input on usart0 line */
  if (msp430_usart0_dev_write_uart_ok())
    {
      uint32_t mask,value;
      machine.device[SERIAL0].read(SERIAL0,&mask,&value);
      if ((mask & PTTY_D) != 0)
	msp430_usart0_dev_write_uart(value & PTTY_D);
    }

  /* input on LCD */
  machine.device[LCD].read(LCD,&mask32,&val32);
  if (mask32)
    {
      msp430_digiIO_dev_write(4,val32 & 0xff,0xff);
    }

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
	b |= (machine.ui.b_down & UI_BUTTON_1) ? 0x08 : 0x0;
	b |= (machine.ui.b_down & UI_BUTTON_2) ? 0x04 : 0x0;
	b |= (machine.ui.b_down & UI_BUTTON_3) ? 0x02 : 0x0;
	b |= (machine.ui.b_down & UI_BUTTON_4) ? 0x01 : 0x0;

	msp430_digiIO_dev_write(0, b, 0xf);
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

  /* ************************* */
  /* update                    */
  /* ************************* */
  LIBSELECT_UPDATE();
  LIBWSNET_UPDATE();

  if (refresh)
    {
      ui_refresh();
    }
  
  return res;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */
