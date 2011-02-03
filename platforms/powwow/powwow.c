/**
 *  \file   powwow.c
 *  \brief  Platform devices handling functions, MSPv4, CAIRN platform
 *  \author Antoine Fraboulet, Romain Fontaine
 *  \date   2010
 **/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __DEBUG
#undef DEBUG
#endif

#include "arch/common/hardware.h"
#include "arch/msp430/msp430.h"

#include "devices/devices.h"
#include "devices/led/led_dev.h"
#include "devices/cc2420/cc2420_dev.h"
#include "devices/ptty/ptty_dev.h"
#include "devices/uigfx/uigfx_dev.h"
#include "src/options.h"

/* ****************************************
 * platform description for MSPv4 devices
 *
 * MCU MSP430f1611
 *
 * Port 1
 * =======
 *   1.0 : LED1
 *   1.1 : LED2
 *   1.2 : LED3
 *   1.3 : LED4
 *   1.4 : LBO
 *   1.5 : VAL232/
 *   1.6 : SW1
 *   1.7 : SW2
 *
 * Port 2
 * =======
 *   2.0 : CCVEN
 *   2.1 : CCRESET
 *   2.2 : CCFIFO
 *   2.3 : CCFIFOP
 *   2.4 : CCCCA
 *   2.6 : CCSFD
 *
 * Port 3
 * =======
 *   3.0 : PONOFF
 *   3.1 : SDA
 *   3.3 : SCL
 *   3.4 : TX
 *   3.5 : RX
 *
 * Port 4
 * =======
 *   8 GPIO
 *
 * Port 5
 * =======
 *   5.0 : CCCS
 *   5.1 : CCSI
 *   5.2 : CCSO
 *   5.3 : CCSCLK
 *
 * Port 6
 * =======
 *   8 GPIO
 *
 * ***************************************/

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

#define SYSTEM          0
#define LED1            1
#define LED2            2
#define LED3            3
#define LED4            4
#define RADIO           5
#define SERIAL0         6
#define LOGO1           7

#if defined(__msp430_have_usart1)
#  define SERIAL1       8
#  define FREE_DEVICE_MAX 9
#else
#  define FREE_DEVICE_MAX 8
#endif

#define NAME "PowWow"

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

int
devices_options_add()
{
  ptty_add_options(SERIAL0, 0, "serial0");
#if defined(SERIAL1)
  ptty_add_options(SERIAL1, 1, "serial1");
#endif
  return 0;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

struct powwow_struct_t
{
  int radio_cs;
  int buttons_lastvalue;
};

#define SYSTEM_DATA      ((struct powwow_struct_t*)(machine.device[SYSTEM].data))
#define SYSTEM_RADIO_CS  (SYSTEM_DATA->radio_cs)
#define BUTTONS_LAST     SYSTEM_DATA->buttons_lastvalue

int
system_reset(int UNUSED dev)
{
  BUTTONS_LAST = 0x03;
  return 0;
}

int
system_delete(int UNUSED dev)
{
  return 0;
}

int
system_create(int dev_num)
{
  machine.device[dev_num].reset = system_reset;
  machine.device[dev_num].delete = system_delete;
  machine.device[dev_num].state_size = sizeof(struct powwow_struct_t);
  machine.device[dev_num].name = "System Platform";

  STDOUT("=========================\n");
  STDOUT("%s: button 1 = '1'\n", NAME);
  STDOUT("%s: button 2 = '2'\n", NAME);
  STDOUT("%s: button 3 = '3'\n", NAME);
  STDOUT("%s: button 4 = '4'\n", NAME);
  STDOUT("%s:\n", NAME);
  STDOUT("%s: 'q' quit (not in gdb mode)\n", NAME);
  STDOUT("=========================\n\n");

  return 0;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

int
devices_create()
{
  int res = 0;
  char cc2420_antenna[] = "omnidirectionnal"; /* used by wsnet2, only this model available in wsnet2 */

  /*********************************/
  /* MSP430 MCU                    */
  /*********************************/
  res += msp430_mcu_create(32768 /* xin_freq*/, 8000000 /* xt2in_freq */);

  /*********************************/
  /* fix peripheral sizes          */
  /*********************************/
  machine.device_max = FREE_DEVICE_MAX;
  machine.device_size[SYSTEM] = sizeof(struct powwow_struct_t);
  machine.device_size[LED1] = led_device_size(); // Led1
  machine.device_size[LED2] = led_device_size(); // Led2
  machine.device_size[LED3] = led_device_size(); // Led3
  machine.device_size[LED4] = led_device_size(); // Led4
  machine.device_size[RADIO] = cc2420_device_size(); // cc2420 radio
  machine.device_size[SERIAL0] = ptty_device_size();
#if defined(SERIAL1)
  machine.device_size[SERIAL1] = ptty_device_size();
#endif

#if defined(LOGO1)
  machine.device_size[LOGO1] = uigfx_device_size();
#endif

  /*********************************/
  /* allocate memory               */
  /*********************************/
  res += devices_memory_allocate();
  /*********************************/
  /* create peripherals            */
  /*********************************/
#if defined(LOGO1)
#  include "powwow.xpm"
#  define POWWOW powwow
#endif
#define BKG 0xffffff
#define ON  0x00ee00
#define OFF 0x202020

  res += system_create(SYSTEM);
  res += led_device_create(LED1, ON, OFF, BKG, "led1");
  res += led_device_create(LED2, ON, OFF, BKG, "led2");
  res += led_device_create(LED3, ON, OFF, BKG, "led3");
  res += led_device_create(LED4, ON, OFF, BKG, "led4");
  res += cc2420_device_create(RADIO, 27, cc2420_antenna); // 27MHz
  res += ptty_device_create(SERIAL0, 0);
#if defined(SERIAL1)
  res += ptty_device_create (SERIAL1,0);
#endif
#if defined(LOGO1)
  res += uigfx_device_create(LOGO1, POWWOW);
#endif

  /*********************************/
  /* place peripherals Gui         */
  /*********************************/
    {
      int lw, lh;

      machine.device[LED1].ui_get_size(LED1, &lw, &lh);

      machine.device[LED1].ui_set_pos(LED1, 0, 0);
      machine.device[LED2].ui_set_pos(LED2, lw, 0);
      machine.device[LED3].ui_set_pos(LED3, 2 * lw, 0);
      machine.device[LED4].ui_set_pos(LED4, 3 * lw, 0);
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
int
devices_reset_post()
{
  int refresh = 0;

  /* p1.6 & p1.7 buttons set to 1 by default */
  msp430_digiIO_dev_write(PORT1, 0xc0, 0xc0);

  SYSTEM_RADIO_CS = 0;

  REFRESH(LOGO1);
  REFRESH(LED1);
  REFRESH(LED2);
  REFRESH(LED3);
  REFRESH(LED4);

  ui_refresh(refresh);

  return 0;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

int
devices_update()
{
  int res       = 0;
  int refresh   = 0;
  uint8_t val8  = 0;

  /* *************************************************************************** */
  /* MCU -> devices                                                              */
  /* *************************************************************************** */

  /* port 1 :                          */
  /* ========                          */
  /*   P1.0 - P1.3 : 4 LEDS            */
  if (msp430_digiIO_dev_read(PORT1, &val8))
    {
      int i;
      for (i = LED1; i <= LED4; i++)
        {
          machine.device[i].write(i, LED_DATA, BIT(val8,(i - LED1)));
          UPDATE(i);
          REFRESH(i);
        }
    }

  /* port 2 :                           */
  /* ========                           */
  if (msp430_usart0_dev_read_uart(&val8))
    {
      machine.device[SERIAL0].write(SERIAL0, PTTY_D, val8);
    }

#if defined(SERIAL1)
  if (msp430_usart1_dev_read_uart(&val8))
    {
      machine.device[SERIAL1].write(SERIAL1, PTTY_D, val8);
    }
#endif

  /* port 3:                            */
  /* =======                            */

  /* *************************************************************************** */
  /* devices -> MCU                                                              */
  /* *************************************************************************** */

  /* input on usart0 line */
  if (msp430_usart0_dev_write_uart_ok())
    {
      uint32_t mask, value;
      machine.device[SERIAL0].read(SERIAL0, &mask, &value);
      if ((mask & PTTY_D) != 0)
        msp430_usart0_dev_write_uart(value & PTTY_D);
    }

  /* input on buttons */
    {
      int ev;
      switch ((ev = ui_getevent()))
        {
      case UI_EVENT_USER:
        {
          //init for buttons 1 & 2
          uint8_t b = 0x03;

          // Detect state changes
          if ((machine.ui.b_down & UI_BUTTON_1) != 0)
            {
              b &= ~0x01;
              INFO("%s: button 1 pressed\n", NAME);
            }
          if ((machine.ui.b_down & UI_BUTTON_2) != 0)
            {
              b &= ~0x02;
              INFO("%s: button 2 pressed\n", NAME);
            }

          if (b != BUTTONS_LAST)
            {
              int i;
              HW_DMSG_PLATFORM(4,"%s: b 0x%02x last 0x%02x\n",NAME,b,BUTTONS_LAST);
              for(i=0; i<2; i++)
                {
                  if (((b            & (0x01<<i)) == 0) &&
                      ((BUTTONS_LAST & (0x01<<i)) != 0))
                    {
                      INFO("%s: button %d pressed\n",NAME,i+1);
                    }

                  if (((b            & (0x01<<i)) != 0) &&
                      ((BUTTONS_LAST & (0x01<<i)) == 0))
                    {
                      INFO("%s: button %d released\n",NAME,i+1);
                    }
                }
            }

          // Button 1->p1.6, button 2->p1.7
          // 2 buttons mask
          HW_DMSG_PLATFORM(4,"%s: port1 write 0x%02x\n",NAME,b<<6);
          msp430_digiIO_dev_write(PORT1, b<<6, 0xC0);

          BUTTONS_LAST = b;
        }
        break;
      case UI_EVENT_QUIT:
        mcu_signal_add(SIG_UI);
        break;
      case UI_EVENT_NONE:
        break;
      default:
        ERROR("%s: unknown ui event\n", NAME);
        break;
        }
    }

  /* *************************************************************************** */
  /* update                                                                      */
  /* *************************************************************************** */
  LIBSELECT_UPDATE();
  LIBWSNET_UPDATE();
  UPDATE(SERIAL0);

#if defined(SERIAL1)
  UPDATE(SERIAL1);
#endif

  ui_refresh(refresh);

  return res;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */
