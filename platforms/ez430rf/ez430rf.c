
/**
 *  \file   ez430rf.c
 *  \brief  Platform devices handling functions, ez430-RF2500 platform
 *  \author Antoine Fraboulet
 *  \date   2008
 **/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "arch/common/hardware.h"
#include "arch/msp430/msp430.h"

#include "devices/devices.h"
#include "devices/led/led_dev.h"
#include "devices/cc1100_2500/cc1100_2500_dev.h"
#include "devices/ptty/ptty_dev.h"
#include "devices/uigfx/uigfx_dev.h"
#include "src/options.h"

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

/* ****************************************
 * platform description for ez430rf devices
 *
 * Port 1
 * ======
 *   1.4 : 
 *   1.3 : 
 *   1.2 : button
 *   1.1 : led GREEN
 *   1.0 : led RED
 *
 * Port 2
 * ======
 *   2.7 : GDO2
 *   2.6 : GDO0
 *   2.5 :
 *   2.4 : 
 *   2.3 : 
 *   2.2 : 
 *   2.1 : 
 *   2.0 : 
 *
 * Port 3
 * ======
 *   3.7 : 
 *   3.6 : 
 *   3.5 : USCIA uart data in   
 *   3.4 : USCIA uart data out   
 *   3.3 : USCIB spi clock (radio)
 *   3.2 : USCIB spi somi  (radio)
 *   3.1 : USCIB spi simo  (radio)
 *   3.0 : USCIB STE radio CSN (~CS)
 *
 * Port 4
 * ======
 *   4.7 : 
 *   4.6 : 
 *   4.5 : 
 *   4.4 : 
 *   4.3 : 
 *   4.2 : 
 *   4.1 : 
 *   4.0 : 
 *
 * ***************************************/

/* ****************************************
 *
 * XIN is set to GDO2/radio
 * MSP430x22xx: XT2 is not present.
 * VLOCLK: Internal very low power, low frequency oscillator with 12-kHz,typical frequency.
 * 
 * ***************************************/

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

#define SYSTEM     0
#define LED_RED    1
#define LED_GREEN  2
#define RADIO      3
#define SERIAL     4
#define LOGO1      5

#define END_DEV           LOGO1
#define BOARD_DEVICE_MAX (END_DEV+1)

#define NAME "ez430rf"

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

int devices_options_add(void)
{
  ptty_add_options(SERIAL, 0, "serial0");

  return 0;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

struct ez430rf_struct_t {
  int radio_cs;
  int button_lastvalue;
};

#define SYSTEM_DATA         ((struct ez430rf_struct_t*)(machine.device[SYSTEM].data))
#define SYSTEM_RADIO_CS     (SYSTEM_DATA->radio_cs)
#define SYSTEM_BUTTON_LAST  (SYSTEM_DATA->button_lastvalue)

int system_reset (int UNUSED dev) 
{ 
  SYSTEM_BUTTON_LAST  = 0xff;
  return 0; 
}

int system_delete(int UNUSED dev) 
{ 
  return 0; 
}

int system_create(int dev_num)
{
  //  struct led_t *dev = (struct led_t*) machine.device[dev_num].data;
  machine.device[dev_num].reset         = system_reset;
  machine.device[dev_num].delete        = system_delete;
  machine.device[dev_num].state_size    = sizeof(struct ez430rf_struct_t);
  machine.device[dev_num].name          = "System Platform";

  STDOUT("%s:\n", NAME);
  STDOUT("%s: =========================\n", NAME);
  STDOUT("%s: button 1 = '1'\n", NAME);
  STDOUT("%s:\n", NAME);
  STDOUT("%s: 'q' quit (except gdb mode)\n", NAME);
  STDOUT("%s: =========================\n", NAME);
  STDOUT("%s:\n", NAME);
  return 0;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

int devices_create(void)
{
  int res = 0;
  //int xin_freq, xt2_freq, xosc_freq;
  int xin_freq, vlo_freq, xosc_freq;
  char cc2500_antenna[] = "omnidirectionnal"; /* used by wsnet2, only this model available in wsnet2 */

  xin_freq  =        0; /*  0 kHz */
  vlo_freq  =    12000; /* 12 kHz */
  xosc_freq = 26000000; /* 26 MHz */

  /*********************************/
  /* MSP430 MCU                    */
  /*********************************/

  res += msp430_mcu_create(xin_freq, vlo_freq);

  /*********************************/
  /* fix peripheral sizes          */
  /*********************************/

  machine.device_max             = BOARD_DEVICE_MAX;
  machine.device_size[SYSTEM]    = sizeof(struct ez430rf_struct_t);
  machine.device_size[LED_RED]   = led_device_size();    
  machine.device_size[LED_GREEN] = led_device_size();    
  machine.device_size[RADIO]     = cc2500_device_size(); 
  machine.device_size[SERIAL]    = ptty_device_size();
  machine.device_size[LOGO1]     = uigfx_device_size();

  /*********************************/
  /* allocate memory               */
  /*********************************/

  res += devices_memory_allocate();

  /*********************************/
  /* create peripherals            */
  /*********************************/

#  define BKG 0xffffff
#  define OFF 0x202020
#  include "ez430rf.xpm"
#  define IMG ez___rf

  res += system_create          (SYSTEM);
  res += led_device_create      (LED_RED,   0xee0000,OFF,BKG,"red");
  res += led_device_create      (LED_GREEN, 0x00ee00,OFF,BKG,"green");
  res += cc2500_device_create   (RADIO,     xosc_freq / 1000000, cc2500_antenna);
  res += ptty_device_create     (SERIAL,    0);
  res += uigfx_device_create    (LOGO1,     IMG);

  /*********************************/
  /* place peripherals Gui         */
  /*********************************/

  {
    int led_w,led_h;
    int log_w,log_h;

    machine.device[LED_RED].ui_get_size(LED_RED, &led_w, &led_h);
    machine.device[LOGO1  ].ui_get_size(LOGO1,   &log_w, &log_h);

    machine.device[LED_RED  ].ui_set_pos(LED_RED,         0,   0);
    machine.device[LED_GREEN].ui_set_pos(LED_GREEN, 1*led_w,   0);
    machine.device[LOGO1    ].ui_set_pos(LOGO1,           0,   0);
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

  SYSTEM_RADIO_CS       = 0;
  SYSTEM_BUTTON_LAST    = 0xff;

  REFRESH(LOGO1);
  REFRESH(LED_RED);
  REFRESH(LED_GREEN);
  ui_refresh(refresh);
  return 0;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

int devices_update(void)
{
  int res = 0;
  int refresh = 0;
  int CC2500_CSn = 0;
  uint8_t  val8;

  /* *************************************************************************** */
  /* MCU -> devices                                                              */
  /* *************************************************************************** */

  /* port 1 :                          */
  /* ========                          */
  if (msp430_digiIO_dev_read(PORT1,&val8))
    {
      machine.device[LED_RED].write(LED_RED,LED_DATA, ! BIT(val8,0));
      etracer_slot_access(0x0, 1, ETRACER_ACCESS_WRITE, ETRACER_ACCESS_BIT, ETRACER_ACCESS_LVL_GPIO, 0);
      UPDATE(LED_RED);
      REFRESH(LED_RED);

      machine.device[LED_GREEN].write(LED_GREEN,LED_DATA, !  BIT(val8,1));
      etracer_slot_access(0x0, 1, ETRACER_ACCESS_WRITE, ETRACER_ACCESS_BIT, ETRACER_ACCESS_LVL_GPIO, 0);
      UPDATE(LED_GREEN);
      REFRESH(LED_GREEN);
    }
  
  /* port 2 :                          */
  /* ========                          */

  /* port 3 :                          */
  /* ========                          */

  /*   P3.5 : USCIA uart data in        */
  /*   P3.4 : USCIA uart data out       */
  /*   P3.4 : USCIA uart data out       */
  /*   P3.3 : USCIB spi clock (radio)   */
  /*   P3.2 : USCIB spi somi  (radio)   */
  /*   P3.1 : USCIB spi simo  (radio)   */
  /*   P3.0 : USCIB STE radio (~CS)     */
  
  if (msp430_digiIO_dev_read(PORT3,&val8))
    {
      CC2500_CSn = BIT(val8,0);
      machine.device[RADIO].write(RADIO, CC2500_CSn_MASK, CC2500_CSn << CC2500_CSn_SHIFT);
      etracer_slot_access(0x0, 1, ETRACER_ACCESS_WRITE, ETRACER_ACCESS_BIT, ETRACER_ACCESS_LVL_GPIO, 0);
    }

  /* port 4 :                          */
  /* ========                          */

  /* USCIA (UART Mode)                 */
  /* ==============                    */
  /* UCA0 : serial                     */
  if (msp430_uscia0_dev_read_uart(&val8))
   {
      machine.device[SERIAL].write(SERIAL, PTTY_D, val8);
      etracer_slot_access(0x0, 1, ETRACER_ACCESS_WRITE, ETRACER_ACCESS_BYTE, ETRACER_ACCESS_LVL_OUT, 0);
   }
  
  /* USCIB (SPI Mode)                 */
  /* ==============                   */
  /* UCB0 : radio                     */
  if (msp430_uscib0_dev_read_spi(&val8))
   {
      machine.device[RADIO].write(RADIO, CC2500_DATA_MASK, val8);
      etracer_slot_access(0x0, 1, ETRACER_ACCESS_WRITE, ETRACER_ACCESS_BYTE, ETRACER_ACCESS_LVL_SPI0, 0);
      HW_DMSG_PLATFORM("msp430:ez430rf:spi write mcu > radio with val 0x%02x\n",val8);
  }      

    
  /* *************************************************************************** */
  /* devices -> MCU                                                              */
  /* *************************************************************************** */


  /* input on radio */
  {
    uint32_t mask  = 0;
    uint32_t value = 0;
    machine.device[RADIO].read( RADIO ,&mask,&value);
    if (mask & CC2500_DATA_MASK)
      {
	msp430_uscib0_dev_write_spi(value & CC2500_DATA_MASK);
	etracer_slot_access(0x0, 1, ETRACER_ACCESS_READ, ETRACER_ACCESS_BYTE, ETRACER_ACCESS_LVL_SPI0, 0);
	HW_DMSG_PLATFORM("msp430:ez430rf:spi write radio > mcu with val 0x%02x\n",value);
      }
    if (mask & CC2500_GDO2_MASK) // GDO2 -> P2.7
      { 
	msp430_digiIO_dev_write(PORT2, (CC2500_GDO2_MASK & value) ? 0x80 : 0x00, 0x80);
	etracer_slot_access(0x0, 1, ETRACER_ACCESS_READ, ETRACER_ACCESS_BIT, ETRACER_ACCESS_LVL_GPIO, 0);
      }
    if (mask & CC2500_GDO0_MASK) // GDO0 -> P2.6
      { 
	msp430_digiIO_dev_write(PORT2, (CC2500_GDO0_MASK & value) ? 0x40 : 0x00, 0x40);
	etracer_slot_access(0x0, 1, ETRACER_ACCESS_READ, ETRACER_ACCESS_BIT, ETRACER_ACCESS_LVL_GPIO, 0);
      }
  }
  
  /* input on USCIA serial */
  if (msp430_uscia0_dev_write_uart_ok())
    {
      uint32_t mask,value;
      machine.device[SERIAL].read(SERIAL,&mask,&value);
      if ((mask & PTTY_D) != 0)
	{
	  msp430_uscia0_dev_write_uart(value & PTTY_D);
	  etracer_slot_access(0x0, 1, ETRACER_ACCESS_READ, ETRACER_ACCESS_BYTE, ETRACER_ACCESS_LVL_OUT, 0);
	}
    }


  /* input on UI */
  /* input on buttons */
  {
    int ev;
    switch ((ev = ui_getevent()))
      {
      case UI_EVENT_USER:
	{
	  uint8_t b = 0xff;
	  // the reset button is negated
	  //  if (machine.ui.val & UI_BUTTON_1)
	  //  msp430_reset_pin((machine.ui.b_down & UI_BUTTON_1) ? 0 : 1);
	  
	  if ((machine.ui.b_down & UI_BUTTON_1) != 0)
	    {
	      b &= ~0x10;
	    }

	  if (b != SYSTEM_BUTTON_LAST)
	    {
	     
	      if (((b                  & (0x10)) == 0) && 
		  ((SYSTEM_BUTTON_LAST & (0x10)) != 0))
		{
		  INFO("%s: button 1 pressed\n",NAME);
		  msp430_digiIO_dev_write(PORT1, 0x00, 0x04);
		}
	      
	      if (((b                  & (0x10)) != 0) && 
		  ((SYSTEM_BUTTON_LAST & (0x10)) == 0))
		{
		  INFO("%s: button 1 released\n",NAME);
		  msp430_digiIO_dev_write(PORT1, 0x00, 0x0400);
		}

	      SYSTEM_BUTTON_LAST = b;
	    }
	}
	break;
      case UI_EVENT_QUIT: /* q */
	mcu_signal_add(SIG_UI);
	break;
      case UI_EVENT_NONE:
	break;
      default:
	ERROR("%s: unknown ui event\n",NAME);
	break;
      }
  }


  /* *************************************************************************** */
  /* update                                                                      */
  /* *************************************************************************** */
  LIBSELECT_UPDATE();
  LIBWSNET_UPDATE();

  UPDATE(RADIO);
  UPDATE(SERIAL);

  ui_refresh(refresh);

  return res;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */
