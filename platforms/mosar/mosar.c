
/**
 *  \file   mosar.c
 *  \brief  Platform devices handling functions, MOSAR platform
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
#include "devices/m25p80/m25p80_dev.h"
#include "devices/cc1100/cc1100_dev.h"
#include "devices/ptty/ptty_dev.h"
#include "devices/uigfx/uigfx_dev.h"
#include "src/options.h"

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

/* ****************************************
 * platform description for WSN430 devices
 *
 * Port 1
 * ======
 *   1.4 : 
 *   1.3 : 
 *   1.2 : 
 *   1.1 : led GREEN
 *   1.0 : led RED
 *
 * Port 2
 * ======
 *   2.7 : GDO2
 *   2.6 : GDO0
 *   2.5 :
 *   2.4 : timer ~E
 *   2.3 : timer ~RST
 *   2.2 : timer ~IRQ
 *   2.1 : timer SQW   // TAINCLK
 *   2.0 : 
 *
 * Port 3
 * ======
 *   3.7 : 
 *   3.6 : 
 *   3.5 : uart data in    -> battery expansion board
 *   3.4 : uart data out   -> battery expansion board
 *   3.3 : spi clock (timer + flash + radio)
 *   3.2 : spi somi  (timer + flash + radio)
 *   3.1 : spi simo  (timer + flash + radio)
 *   3.0 : radio CSN (~CS)
 *
 * Port 4
 * ======
 *   4.7 : 
 *   4.6 : flash ~HOLD
 *   4.5 : 
 *   4.4 : flash ~W
 *   4.3 : flash ~S
 *   4.2 : 
 *   4.1 : 
 *   4.0 : 
 *
 * ***************************************/

/* ****************************************
 * Header <> msp430 <> PCB
 *  P01 : GND              : GND
 *  PO2 : VCC_EXT          : batt clip
 *  P03 : P2.0             : timer M41T93 VCC
 *  P04 : P2.1             : timer M41T93 SQW 
 *  P05 : P2.2             : timer M41T93 ~IRQ
 *  P06 : P2.3             : timer M41T93 ~RST
 *  P07 : P2.4             : timer M41T93 ~E
 *  P08 : P4.3             : flash M25P64 ~S
 *  P09 : P4.4             : flash M25P64 ~W
 *  P10 : P4.5             : X (nc)
 *  P11 : P4.6             : flash M25P64 ~HOLD
 *  P12 : GND              : GND
 *  P13 : GDO0 (radio)     : X (nc)
 *  P14 : GDO2 (radio)     : X (nc)
 *  P15 : P3.2 SOMI (spi)  : timer SDO + flash Q
 *  P16 : P3.3 CLK  (spi)  : timer SCL + flash SCL
 *  P17 : P3.0 STE  (i2c)  : X (nc)
 *  P18 : P3.1 SOMI (spi)  : timer SDI + flash D
 *
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
#define FLASH      1
#define LED_RED    2
#define LED_GREE   3
#define RADIO      4
#define SERIAL     5
#define LOGO1      6

#define END_DEV           LOGO1
#define BOARD_DEVICE_MAX (END_DEV+1)

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

int devices_options_add(void)
{
  m25p_add_options(FLASH,  0, "flash"  );
  ptty_add_options(SERIAL, 0, "serial0");

  return 0;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

struct mosar_struct_t {
  int flash_cs;
  int radio_cs;
  int timer_cs;
};

#define SYSTEM_DATA      ((struct mosar_struct_t*)(machine.device[SYSTEM].data))
#define SYSTEM_FLASH_CS  (SYSTEM_DATA->flash_cs)
#define SYSTEM_RADIO_CS  (SYSTEM_DATA->radio_cs)
#define SYSTEM_TIMER_CS  (SYSTEM_DATA->timer_cs)

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
  //  struct led_t *dev = (struct led_t*) machine.device[dev_num].data;
  machine.device[dev_num].reset         = system_reset;
  machine.device[dev_num].delete        = system_delete;
  machine.device[dev_num].state_size    = sizeof(struct mosar_struct_t);
  machine.device[dev_num].name          = "System Platform";
  return 0;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

int devices_create(void)
{
  int res = 0;
  int xin_freq, xt2_freq, xosc_freq;
  char cc1100_antenna[] = "omnidirectionnal"; /* used by wsnet2, only this model available in wsnet2 */

  xin_freq  =        0; /*  0 kHz */
  xt2_freq  =        0; /*  0 MHz */
  xosc_freq = 26000000; /* 26 MHz */

  /*********************************/
  /* MSP430 MCU                    */
  /*********************************/

  res += msp430_mcu_create(xin_freq, xt2_freq);

  /*********************************/
  /* fix peripheral sizes          */
  /*********************************/

  machine.device_max             = BOARD_DEVICE_MAX;
  machine.device_size[SYSTEM]    = sizeof(struct mosar_struct_t);
  machine.device_size[FLASH]     = m25p_device_size();   
  machine.device_size[LED_RED]   = led_device_size();    
  machine.device_size[LED_GREEN] = led_device_size();    
  machine.device_size[RADIO]     = cc1100_device_size(); 
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
#  include "wsim.xpm"
#  define WSIM wsim

  res += system_create          (SYSTEM);
  res += led_device_create      (LED_RED,   0xee0000,OFF,BKG,"red");
  res += led_device_create      (LED_GREEN, 0x00ee00,OFF,BKG,"green");
  res += m25p_device_create     (FLASH,     0);
  res += cc1100_device_create   (RADIO,     xosc_freq / 1000000, cc1100_antenna);
  res += ptty_device_create     (SERIAL,    0);
  res += uigfx_device_create    (LOGO1,   wsim);

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
#if defined(GUI)
  int refresh = 0;
#endif

  machine.device[FLASH].write(FLASH, M25P_W, M25P_W);
  SYSTEM_FLASH_CS = 0;
  SYSTEM_RADIO_CS = 0;
  SYSTEM_TIMER_CS = 0;

  REFRESH(LED1);
  REFRESH(LED2);
  REFRESH(LOGO1);
  return 0;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

int devices_update(void)
{
  int res = 0;
#if defined(GUI)
  int refresh = 0;
#endif
  int CC1100_CSn = 0;
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
  /*   P2.7 NC                         */
  /*   P2.6 NC                         */
  /*   P2.5 NC                         */
  /*   P2.4 1wire                      */
  /*   P2.3 NC                         */
  /*   P2.3 NC                         */
  /*   P2.1 7seg selector 1            */
  /*   P2.0 7seg selector 0            */
  if (msp430_digiIO_dev_read(PORT2,&val8))
    {
      etracer_slot_access(0x0, 1, ETRACER_ACCESS_WRITE, ETRACER_ACCESS_BIT, ETRACER_ACCESS_LVL_GPIO, 0);
    }

  /* port 3 :                          */
  /* ========                          */
  /*   P3.7 urxd1 : serial             */
  /*   P3.6 utxd1 : serial             */
  /*   P3.5 urxd0                      */
  /*   P3.4 utxd0                      */ 
  /*   P3.3 SPI cc1100 UCLK            */
  /*   P3.2 SPI cc1100 SOMI            */
  /*   P3.1 SPI cc1100 SIMO            */
  /*   P3.0 NC                         */
#if 0
  if (msp430_digiIO_dev_read(PORT3,&val8))
    {
      /* cc1100 is driven by spi                          */
      /* we could/should check here that the pins are not */
      /* driven by the GPIO                               */
    }
#endif

  /* port 4 :                          */
  /* ========                          */
  /*   P4.7 flash hold                 */
  /*   P4.6 NC                         */
  /*   P4.5 NC                         */
  /*   P4.4 CS flash                   */
  /*   P4.3 NC                         */
  /*   P4.2 CS cc1100                  */
  /*   P4.1 batt status                */
  /*   P4.0 NC                         */
  if (msp430_digiIO_dev_read(PORT4,&val8))
    {
      machine.device[FLASH].write(FLASH, 
				  M25P_H | M25P_S, 
				  (BIT(val8,7) << M25P_H_SHIFT) | 
				  (BIT(val8,4) << M25P_S_SHIFT));
      /* waiting for flash update */
      etracer_slot_access(0x0, 1, ETRACER_ACCESS_WRITE, ETRACER_ACCESS_BIT, ETRACER_ACCESS_LVL_GPIO, 0);

      CC1100_CSn = BIT(val8,2);
      machine.device[RADIO].write(RADIO, CC1100_CSn_MASK, CC1100_CSn << CC1100_CSn_SHIFT);
      etracer_slot_access(0x0, 1, ETRACER_ACCESS_WRITE, ETRACER_ACCESS_BIT, ETRACER_ACCESS_LVL_GPIO, 0);
    }

  /* Usart SPI mode                    */
  /* ==============                    */
  /* SPI 0 : radio                     */
  /* SPI 1 : flash                     */
  switch (MCU.usart0.mode)
    {
    case USART_MODE_SPI:
      if (msp430_usart0_dev_read_spi(&val8))
	{
	  machine.device[RADIO].write(RADIO, CC1100_DATA_MASK, val8);
	  etracer_slot_access(0x0, 1, ETRACER_ACCESS_WRITE, ETRACER_ACCESS_BYTE, ETRACER_ACCESS_LVL_SPI0, 0);
	}      
      break; 
    case USART_MODE_UART:
      break;
    default:
      break;
    }
    
  /* *************************************************************************** */
  /* devices -> MCU                                                              */
  /* *************************************************************************** */


  /* input on radio */
  {
    uint32_t mask  = 0;
    uint32_t value = 0;
    machine.device[ RADIO ].read( RADIO ,&mask,&value);
    if (mask & CC1100_DATA_MASK)
      {
	if (MCU.usart0.mode != USART_MODE_SPI)
	  {
#if defined(DEBUG_ME_HARDER)
	    ERROR("wsn430:devices: read data on radio while not in SPI mode ?\n");
#endif
	  }
	msp430_usart0_dev_write_spi(value & CC1100_DATA_MASK);
	etracer_slot_access(0x0, 1, ETRACER_ACCESS_READ, ETRACER_ACCESS_BYTE, ETRACER_ACCESS_LVL_SPI0, 0);
      }

    if (mask & CC1100_GDO2_MASK) // GDO2 -> P1.4
      { 
	msp430_digiIO_dev_write(PORT1, (CC1100_GDO2_MASK & value) ? 0x10 : 0x00, 0x10);
	etracer_slot_access(0x0, 1, ETRACER_ACCESS_READ, ETRACER_ACCESS_BIT, ETRACER_ACCESS_LVL_GPIO, 0);
      }
    if (mask & CC1100_GDO0_MASK) // GDO0 -> P1.3
      { 
	msp430_digiIO_dev_write(PORT1, (CC1100_GDO0_MASK & value) ? 0x08 : 0x00, 0x08);
	etracer_slot_access(0x0, 1, ETRACER_ACCESS_READ, ETRACER_ACCESS_BIT, ETRACER_ACCESS_LVL_GPIO, 0);
      }
  }


  /* input on flash */
  {
    uint32_t mask  = 0;
    uint32_t value = 0;
    machine.device[ FLASH ].read( FLASH ,&mask,&value);
    if ((mask & M25P_D) != 0)
      {
	if (MCU.usart1.mode != USART_MODE_SPI)
	  {
	    ERROR("wsn430:devices: read data on flash while not in SPI mode ?\n");
	  }
	msp430_usart1_dev_write_spi(value & 0x00FF);
	etracer_slot_access(0x0, 1, ETRACER_ACCESS_READ, ETRACER_ACCESS_BYTE, ETRACER_ACCESS_LVL_SPI1, 0);
      }
  }


  /* input on UART serial */
  if (msp430_usart1_dev_write_uart_ok())
    {
      uint32_t mask,value;
      machine.device[SERIAL].read(SERIAL,&mask,&value);
      if ((mask & PTTY_D) != 0)
	{
	  msp430_usart1_dev_write_uart(value & PTTY_D);
	  /* etracer_slot_access(0x0, 1, ETRACER_ACCESS_READ, ETRACER_ACCESS_BYTE, ETRACER_ACCESS_LVL_OUT, 0); */
	}
    }


  /* input on UI is disabled */
#if defined(GUI) && defined(INPUT_GUI)
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
	    HW_DMSG_UI("wsn430:devices: UI event QUIT\n");
	    MCU_SIGNAL = MCU_SIGINT;
	    break;
	  case UI_EVENT_CMD:
	  case UI_EVENT_NONE:
	    break;
	  default:
	    ERROR("wsn430:devices: unknown ui event\n");
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

  UPDATE(RADIO);
  UPDATE(FLASH);
  UPDATE(SERIAL);

#if defined(GUI)
  if (refresh) 
    {
      ui_refresh();
    }
#endif

  return res;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */
