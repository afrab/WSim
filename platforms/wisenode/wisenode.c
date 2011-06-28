
/**
 *  \file   wisenode.c
 *  \brief  Platform devices handling functions, CSEM WiseNodes
 *  \author Antoine Fraboulet
 *  \date   2007
 **/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "arch/common/hardware.h"
#include "arch/msp430/msp430.h"

#include "devices/devices.h"
#include "devices/led/led_dev.h"
#include "devices/m25p80/m25p80_dev.h"
#include "devices/cc1100_2500/cc1100_2500_dev.h"
#include "devices/ptty/ptty_dev.h"
#include "devices/uigfx/uigfx_dev.h"
#include "src/options.h"

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

#define SYSTEM  0
#define FLASH   1
#define LED1    2
#define LED2    3
#define LED3    4
#define LED4    5
#define RADIO   6
#define SERIAL  7
#define LOGO1   8

#define END_DEV           LOGO1
#define BOARD_DEVICE_MAX (END_DEV+1)

#define NAME "wisenode"

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

static struct moption_t xt1_opt = {
  .longname    = "xin",
  .type        = required_argument,
  .helpstring  = "xin crystal freq (Hz)",
  .value       = NULL
};

static struct moption_t xt2_opt = {
  .longname    = "xt2",
  .type        = required_argument,
  .helpstring  = "xt2 crystal freq (Hz)",
  .value       = NULL
};

static struct moption_t xosc_opt = {
  .longname    = "xosc",
  .type        = required_argument,
  .helpstring  = "xosc crystal freq (Hz)",
  .value       = NULL
};

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

int devices_options_add(void)
{
  options_add(& xt1_opt            );
  options_add(& xt2_opt            );
  options_add(& xosc_opt           );
  m25p_add_options(FLASH,  0, "flash"  );
  ptty_add_options(SERIAL, 1, "serial1");

  return 0;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

struct wisenode_struct_t {
  int flash_cs;
  int radio_cs;
  int buttons_lastvalue;
};

#define SYSTEM_DATA      ((struct wisenode_struct_t*)(machine.device[SYSTEM].data))
#define SYSTEM_FLASH_CS  SYSTEM_DATA->flash_cs
#define SYSTEM_RADIO_CS  SYSTEM_DATA->radio_cs
#define BUTTONS_LAST     SYSTEM_DATA->buttons_lastvalue

int system_reset (int UNUSED dev) 
{ 
  BUTTONS_LAST  = 0xC0;
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
  machine.device[dev_num].state_size    = sizeof(struct wisenode_struct_t);
  machine.device[dev_num].name          = "System Platform";

  STDOUT("%s:\n", NAME);
  STDOUT("%s: =========================\n", NAME);
  STDOUT("%s: button 1 = '1'\n", NAME);
  STDOUT("%s: button 2 = '2'\n", NAME);
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
  int xin_freq, xt2_freq, xosc_freq;
  char cc1100_antenna[] = "omnidirectionnal"; /* used by wsnet2, only this model available in wsnet2 */

  xin_freq  = 32768;    /* 32 kHz */
  xt2_freq  = 0;        /* unused */
  xosc_freq = 26000000; /* 26 MHz */

  if (xt1_opt.value) {
    xin_freq = atoi(xt1_opt.value);
    INFO("%s: xt1 external crystal set to %d Hz\n",NAME,xin_freq);
  }

  if (xt2_opt.value) {
    xt2_freq = atoi(xt2_opt.value);
    INFO("%s: xt2 external crystal set to %d Hz\n",NAME,xt2_freq);
  }

  if (xosc_opt.value) {
    xosc_freq = atoi(xosc_opt.value);
    INFO("%s: xosc external crystal set to %d Hz\n",NAME,xosc_freq);
  }

  /*********************************/
  /* MSP430 MCU                    */
  /*********************************/

  res += msp430_mcu_create(xin_freq, xt2_freq);

  /*********************************/
  /* fix peripheral sizes          */
  /*********************************/

  machine.device_max          = BOARD_DEVICE_MAX;
  machine.device_size[SYSTEM] = sizeof(struct wisenode_struct_t);
  machine.device_size[FLASH]  = m25p_device_size();   // 1 Flash RAM
  machine.device_size[LED1]   = led_device_size();    // 2 Led1
  machine.device_size[LED2]   = led_device_size();    // 3 Led2
  machine.device_size[LED3]   = led_device_size();    // 4 Led3
  machine.device_size[LED4]   = led_device_size();    // 5 Led4
  machine.device_size[RADIO]  = cc1100_device_size(); // 6 cc1100 radio
  machine.device_size[SERIAL] = ptty_device_size();   // 7 serial
#if defined(LOGO1)
  machine.device_size[LOGO1]  = uigfx_device_size();
#endif

  /*********************************/
  /* allocate memory               */
  /*********************************/

  res += devices_memory_allocate();

  /*********************************/
  /* create peripherals            */
  /*********************************/

#if defined(LOGO1)
#  define BKG 0xffffff
#  define OFF 0x202020
#  include "wisenode.xpm"
#  define XPMNAME wisenode
#else
#  define BKG 0x000000
#  define OFF 0x202020
#endif

  res += system_create          (SYSTEM);
  res += m25p_device_create     (FLASH, 0);
  res += led_device_create      (LED1,    0xff3232,OFF,BKG, "led1");
  res += led_device_create      (LED2,    0x32ff32,OFF,BKG, "led2");
  res += led_device_create      (LED3,    0x3232ff,OFF,BKG, "led3");
  res += led_device_create      (LED4,    0x00ffff,OFF,BKG, "led4");
  res += cc1100_device_create   (RADIO, xosc_freq / 1000000, cc1100_antenna);
  res += ptty_device_create     (SERIAL,1);
#if defined(LOGO1)
  res += uigfx_device_create    (LOGO1,  XPMNAME);
#endif

  /*********************************/
  /* place peripherals Gui         */
  /*********************************/

  {
    int lw,lh;
#if defined(LOGO1)
    int log_w,log_h;
#endif

    machine.device[LED1].ui_get_size(LED1,&lw,&lh);
#if defined(LOGO1)
    machine.device[LOGO1].ui_get_size(LOGO1, &log_w, &log_h);
#endif

    machine.device[LED1].ui_set_pos(LED1,    0,   0);
    machine.device[LED2].ui_set_pos(LED2, 1*lw,   0);
    machine.device[LED3].ui_set_pos(LED3, 2*lw,   0);
    machine.device[LED3].ui_set_pos(LED4, 3*lw,   0);
#if defined(LOGO1)
    machine.device[LOGO1].ui_set_pos(LOGO1,   0,   0);
#endif
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

  SYSTEM_FLASH_CS = 0;
  SYSTEM_RADIO_CS = 0;

#if defined(LOGO1)
  REFRESH(LOGO1);
#endif
  REFRESH(LED1);
  REFRESH(LED2);
  REFRESH(LED3);
  ui_refresh(refresh);
  return 0;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

int devices_update(void)
{
  int res        = 0;
  uint8_t  val8  = 0;
  int refresh    = 0;


  /* *************************************************************************** */
  /* MCU -> devices                                                              */
  /* *************************************************************************** */

  /*  Port 1                           */
  /*  ======                           */
  /*    1.0 : GDO_0, cc1100            */
  /*    1.1 :                          */
  /*    1.2 :                          */
  /*    1.3 : GDO_2, cc1100            */
  /*    1.4 :                          */
  /*    1.5 : SPI MISO,                */
  /*    1.6 :                          */
  /*    1.7 :                          */ 
#if 0
  /* port 1 is ised as input */
#endif
  
  /*  Port 2                           */
  /*  ======                           */
  /*    2.0 :                          */
  /*    2.1 :                          */
  /*    2.2 :                          */
  /*    2.3 :                          */
  /*    2.4 :                          */
  /*    2.5 :                          */
  /*    2.6 : button2                  */
  /*    2.7 : button1                  */
#if 0
  /* port 2 is used as input only */
#endif

  /*  port 3 :                         */
  /*  ========                         */
  /*    3.0 : ste0 : NC                */
  /*    3.1 : SPI MOSI                 */
  /*    3.2 : SPI MISO                 */
  /*    3.3 : SPI CLK                  */
  /*    3.4 : utxd0, X1                */
  /*    3.5 : urxd0, X2                */
  /*    3.6 : utxd1                    */
  /*    3.7 : urxd1                    */
#if 0
  /* cc1100 is driven by spi       */
  /* software spi is not supported */
#endif

  /*  port 4 :                         */
  /*  ========                         */
  /*    4.0 :                          */
  /*    4.1 : NC                       */
  /*    4.2 : cc1100 CS                */
  /*    4.3 : X1                       */
  /*    4.4 : X2                       */
  /*    4.5 : flash WP    = M25P_W   */
  /*    4.6 : flash CS    = M25P_S   */
  /*    4.7 : flash hold  = M25P_H   */
  if (msp430_digiIO_dev_read(PORT4,&val8))
    {
#if defined(DEBUG)
      if (BIT(val8,6) != SYSTEM_FLASH_CS)
	{
	  HW_DMSG_DEV("%s: flash CS set to %d\n",NAME,BIT(val8,6));
	}
      if (BIT(val8,2) != SYSTEM_RADIO_CS)
	{
	  HW_DMSG_DEV("%s: radio CS set to %d\n",NAME,BIT(val8,2));
	}
#endif

      machine.device[FLASH].write(FLASH, 
				  M25P_W | M25P_S | M25P_H,
				  (BIT(val8,5) << M25P_W_SHIFT) |
				  (BIT(val8,6) << M25P_S_SHIFT) | 
				  (BIT(val8,7) << M25P_H_SHIFT));
      SYSTEM_FLASH_CS = BIT(val8,6);
      /* waiting for flash update */


      machine.device[RADIO].write(RADIO, CC1100_CSn_MASK,  (BIT(val8,2) << CC1100_CSn_SHIFT));
      SYSTEM_RADIO_CS = BIT(val8,2);
      /* waiting for radio update */

      etracer_slot_access(0x0, 1, ETRACER_ACCESS_WRITE, ETRACER_ACCESS_BIT, ETRACER_ACCESS_LVL_GPIO, 0);
      etracer_slot_access(0x0, 1, ETRACER_ACCESS_WRITE, ETRACER_ACCESS_BIT, ETRACER_ACCESS_LVL_GPIO, 0);
    }

  /*  port 5 :                         */
  /*  ========                         */
  /*    5.0 :                          */
  /*    5.1 :                          */
  /*    5.2 :                          */
  /*    5.3 :                          */
  /*    5.4 : led1                     */
  /*    5.5 : led2                     */
  /*    5.6 : led3                     */
  /*    5.7 : led4                     */
  if (msp430_digiIO_dev_read(PORT5,&val8))
    {
      machine.device[LED1].write(LED1,LED_DATA, ! BIT(val8,4));
      etracer_slot_access(0x0, 1, ETRACER_ACCESS_WRITE, ETRACER_ACCESS_BIT, ETRACER_ACCESS_LVL_GPIO, 0);
      UPDATE(LED1);
      REFRESH(LED1);

      machine.device[LED2].write(LED2,LED_DATA, !  BIT(val8,5));
      etracer_slot_access(0x0, 1, ETRACER_ACCESS_WRITE, ETRACER_ACCESS_BIT, ETRACER_ACCESS_LVL_GPIO, 0);
      UPDATE(LED2);
      REFRESH(LED2);

      machine.device[LED3].write(LED3,LED_DATA, ! BIT(val8,6));
      etracer_slot_access(0x0, 1, ETRACER_ACCESS_WRITE, ETRACER_ACCESS_BIT, ETRACER_ACCESS_LVL_GPIO, 0);
      UPDATE(LED3);
      REFRESH(LED3);

      machine.device[LED4].write(LED4,LED_DATA, ! BIT(val8,7));
      etracer_slot_access(0x0, 1, ETRACER_ACCESS_WRITE, ETRACER_ACCESS_BIT, ETRACER_ACCESS_LVL_GPIO, 0);
      UPDATE(LED4);
      REFRESH(LED4);
    }

  /*  port 6 :                         */
  /*  ========                         */
  /*    6.0 :                          */
  /*    6.1 :                          */
  /*    6.2 :                          */
  /*    6.3 :                          */
  /*    6.4 :                          */
  /*    6.5 : AD Vsense                */
  /*    6.6 :                          */
  /*    6.7 :                          */
#if 0
  /* port 6 is used as input only */
#endif


  /* Usart0                            */
  /* ================================= */
  /* Usart SPI mode                    */
  /* SPI0 : radio + flash              */
  switch (MCU.usart0.mode)
    {
    case USART_MODE_SPI:
      if (msp430_usart0_dev_read_spi(&val8))
	{
	  /*
	   * SUSTEM_FLASH_CS == 0 -- flash on
	   * SYSTEM_RADIO_CS == 0 -- radio on
	   */
	  if ((SYSTEM_FLASH_CS == 0) && (SYSTEM_RADIO_CS == 0))
	    {
	      WARNING("%s: Flash and Radio SPI are selected at the same time\n",NAME);
	    }

	  machine.device[FLASH].write(FLASH, M25P_D, val8);
	  machine.device[RADIO].write(RADIO, CC1100_DATA_MASK, val8);
	  etracer_slot_access(0x0, 1, ETRACER_ACCESS_WRITE, ETRACER_ACCESS_BYTE, ETRACER_ACCESS_LVL_SPI0, 0);
	}
      break;
    case USART_MODE_UART:
      /* Uart0 on expansion port U28 */
      break;
    case USART_MODE_I2C:
      /* not used */
      break;
    }

  /* Usart1                            */
  /* ================================= */
  /* Uart : serial I/O                 */
  switch (MCU.usart1.mode)
    {
    case USART_MODE_SPI:
      break;
    case USART_MODE_UART:
      if (msp430_usart1_dev_read_uart(&val8))
	{
	  machine.device[SERIAL].write(SERIAL, PTTY_D, val8);
	  etracer_slot_access(0x0, 1, ETRACER_ACCESS_WRITE, ETRACER_ACCESS_BYTE, ETRACER_ACCESS_LVL_OUT, 0);
	}
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
	    ERROR("devices: read data on radio while not in SPI mode ?\n");
	  }
	msp430_usart0_dev_write_spi(value & CC1100_DATA_MASK);
	etracer_slot_access(0x0, 1, ETRACER_ACCESS_READ, ETRACER_ACCESS_BYTE, ETRACER_ACCESS_LVL_SPI0, 0);
      }

    if (mask & CC1100_SO_MASK)   // SOMI -> P3.2
      {
	msp430_digiIO_dev_write(PORT3, (CC1100_SO_MASK & value) ? 0x04 : 0x00, 0x04);
	/* no etracer */
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
	    ERROR("devices: read data on flash while not in SPI mode ?\n");
	  }
	msp430_usart1_dev_write_spi(value & M25P_D);
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


#if defined(GUI)
  {
    int ev;
    switch ((ev = ui_getevent()))
      {
      case UI_EVENT_USER:
	{
	  uint8_t b = 0xc0; /* 1 when released  */
	  /*    2.6 : button2 'z'               */
	  /*    2.7 : button1 'a'               */
	  if ((machine.ui.b_down & UI_BUTTON_1) != 0)
	    {
	      b &= ~0x80;
	      INFO("%s: button 1 pressed\n",NAME);
	    }
	  if ((machine.ui.b_down & UI_BUTTON_2) != 0)
	    {
	      b &= ~0x40;
	      INFO("%s: button 2 pressed\n",NAME);
	    }

	  msp430_digiIO_dev_write(PORT2, b, 0xC0);
	  BUTTONS_LAST = b;
	}
	break; /* UI_EVENT_USER */

      case UI_EVENT_QUIT:
	mcu_signal_add(SIG_UI);
	break;
      case UI_EVENT_NONE:
	break;
      default:
	ERROR("%s: unknown ui event\n",NAME);
	break;
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

  ui_refresh(refresh);

  return res;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */
