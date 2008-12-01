
/**
 *  \file   wsn430.c
 *  \brief  Platform devices handling functions, WSN430 / Capnet platform
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
 *   1.4 : GDO_2, cc1100
 *   1.3 : GDO_0, cc1100
 *   1.2 : flash Write protect
 *
 * Port 2
 * ======
 *   2.4 : 1 wire
 *
 * Port 3
 * ======
 *   3.7 : urxd1
 *   3.6 : utxd1
 *   3.5 : urxd0
 *   3.4 : utxd0
 *   3.3 : uclk0, serial clock, cc1100
 *   3.2 : simo0, serial in, cc1100
 *   3.1 : somi0, serial out, cc1100
 *   3.0 : ste0 : NC
 *
 * Port 4
 * ======
 *   4.7 : flash Hold
 *   4.6 : NC
 *   4.5 : NC
 *   4.4 : flash CS
 *   4.3 : Enable -> battery charger
 *   4.2 : CS_radio, cc1100
 *   4.1 : STATUS -> battery charger
 *
 * Port 5
 * ======
 *   5.6 : LED3
 *   5.5 : LED2
 *   5.4 : LED1
 *   5.3 : uclk1, flash clock
 *   5.2 : somi1, flash out
 *   5.1 : simo1, flash in
 *   5.0 : ste1 : NC
 *
 * ***************************************/

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

#define SYSTEM 0
#define FLASH  1
#define LED1   2
#define LED2   3
#define LED3   4
#define DS24   5
#define RADIO  6
#define SERIAL 7
#define LOGO1  8

#define END_DEV           LOGO1
#define BOARD_DEVICE_MAX (END_DEV+1)

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

static struct moption_t ds2411_opt = {
  .longname    = "ds2411",
  .type        = required_argument,
  .helpstring  = "ds2411 serial number",
  .value       = NULL
};


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
  options_add( &xt1_opt         );
  options_add( &xt2_opt         );
  options_add( &xosc_opt        );
  options_add( &ds2411_opt      );
  m25p_add_options(FLASH,  0, "flash"  );
  ptty_add_options(SERIAL, 1, "serial1");

  return 0;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

struct wsn430_struct_t {
  int flash_cs;
  int radio_cs;
};

#define SYSTEM_DATA      ((struct wsn430_struct_t*)(machine.device[SYSTEM].data))
#define SYSTEM_FLASH_CS  (SYSTEM_DATA->flash_cs)
#define SYSTEM_RADIO_CS  (SYSTEM_DATA->radio_cs)

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
  machine.device[dev_num].state_size    = sizeof(struct wsn430_struct_t);
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

  xin_freq  =    32768; /* 32 kHz */
  xt2_freq  =  8000000; /*  8 MHz */
  xosc_freq = 27000000; /* 27 MHz */

  if (xt1_opt.value) {
    xin_freq = atoi(xt1_opt.value);
    VERBOSE(1,"wsn430: xt1 external crystal set to %d Hz\n",xin_freq);
  }

  if (xt2_opt.value) {
    xt2_freq = atoi(xt2_opt.value);
    VERBOSE(1,"wsn430: xt2 external crystal set to %d Hz\n",xt2_freq);
  }

  if (xosc_opt.value) {
    xosc_freq = atoi(xosc_opt.value);
    VERBOSE(1,"wsn430: xosc external crystal set to %d Hz\n",xosc_freq);
  }

  /*********************************/
  /* MSP430 MCU                    */
  /*********************************/

  res += msp430_mcu_create(xin_freq, xt2_freq);

  /*********************************/
  /* fix peripheral sizes          */
  /*********************************/

  machine.device_max          = BOARD_DEVICE_MAX;
  machine.device_size[SYSTEM] = sizeof(struct wsn430_struct_t);
  machine.device_size[FLASH]  = m25p_device_size();   // Flash RAM
  machine.device_size[LED1]   = led_device_size();    // Led1
  machine.device_size[LED2]   = led_device_size();    // Led2
  machine.device_size[LED3]   = led_device_size();    // Led3
  machine.device_size[DS24]   = ds2411_device_size(); // dallas ds2411
  machine.device_size[RADIO]  = cc1100_device_size(); // cc1100 radio
  machine.device_size[SERIAL] = ptty_device_size();
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
#  include "wsim.xpm"
#  define WSIM wsim
#else
#  define BKG 0x000000
#  define OFF 0x202020
#endif

  res += system_create          (SYSTEM);
  res += led_device_create      (LED1,    0xee0000,OFF,BKG);
  res += led_device_create      (LED2,    0x00ee00,OFF,BKG);
  res += led_device_create      (LED3,    0x0000ee,OFF,BKG);
  res += m25p_device_create     (FLASH,   0);
  res += ds2411_device_create   (DS24,    ds2411_opt.value);
  res += cc1100_device_create   (RADIO,   xosc_freq / 1000000);
  res += ptty_device_create     (SERIAL,  1);
#if defined(LOGO1)
  res += uigfx_device_create    (LOGO1,   wsim);
#endif

  /*********************************/
  /* place peripherals Gui         */
  /*********************************/

  {
    int led_w,led_h;
    int log_w,log_h;

    machine.device[LED1].ui_get_size(LED1,   &led_w, &led_h);
#if defined(LOGO1)
    machine.device[LOGO1].ui_get_size(LOGO1, &log_w, &log_h);
#endif

    machine.device[LED1].ui_set_pos  (LED1,         0,   0);
    machine.device[LED2].ui_set_pos  (LED2,   1*led_w,   0);
    machine.device[LED3].ui_set_pos  (LED3,   2*led_w,   0);
#if defined(LOGO1)
    machine.device[LOGO1].ui_set_pos (LOGO1,        0,   0);
#endif
  }

  /*********************************/
  /* end of platform specific part */
  /*********************************/

  tracer_event_add_id(TRACER_LED1, 1, "led1", "leds");
  tracer_event_add_id(TRACER_LED2, 1, "led2", "leds");
  tracer_event_add_id(TRACER_LED3, 1, "led3", "leds");

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

  REFRESH(LED1);
  REFRESH(LED2);
  REFRESH(LED3);
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
  /*   P1.7 NC                         */
  /*   P1.6 NC                         */
  /*   P1.5 NC                         */
  /*   P1.4 cc1100 gd2                 */
  /*   P1.3 cc1100 gd0                 */
  /*   P1.2 P_Dvcc -> flash write      */
  /*   P1.1 NC                         */
  /*   P1.0 NC                         */
  if (msp430_digiIO_dev_read(PORT1,&val8))
    {
      machine.device[FLASH].write(FLASH, M25P_W, BIT(val8,2) << M25P_W_SHIFT);
      etracer_slot_access(0x0, 1, ETRACER_ACCESS_WRITE, ETRACER_ACCESS_BIT, ETRACER_ACCESS_LVL_GPIO, 0);
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
      machine.device[DS24].write(DS24,DS2411_D,BIT(val8,4)); 
      tracer_event_record(TRACER_DS2411,BIT(val8,4));
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

  /* port 5 :                          */
  /* ========                          */
  /*   P5.7 NC                         */
  /*   P5.6 led 3 (Blue)               */
  /*   P5.5 led 2 (Green)              */
  /*   P5.4 led 1 (Red)                */
  /*   P5.3 SPI flash ram UCLK         */
  /*   P5.2 SPI flash ram SOMI         */
  /*   P5.1 SPI flash ram SIMO         */
  /*   P5.0 NC                         */
  if (msp430_digiIO_dev_read(PORT5,&val8))
    {
      machine.device[LED1].write(LED1,LED_DATA, ! BIT(val8,4));
      etracer_slot_access(0x0, 1, ETRACER_ACCESS_WRITE, ETRACER_ACCESS_BIT, ETRACER_ACCESS_LVL_GPIO, 0);
      tracer_event_record(TRACER_LED1,!BIT(val8,4));
      UPDATE(LED1);
      REFRESH(LED1);

      machine.device[LED2].write(LED2,LED_DATA, !  BIT(val8,5));
      etracer_slot_access(0x0, 1, ETRACER_ACCESS_WRITE, ETRACER_ACCESS_BIT, ETRACER_ACCESS_LVL_GPIO, 0);
      tracer_event_record(TRACER_LED2,!BIT(val8,5));
      UPDATE(LED2);
      REFRESH(LED2);

      machine.device[LED3].write(LED3,LED_DATA, ! BIT(val8,6));
      etracer_slot_access(0x0, 1, ETRACER_ACCESS_WRITE, ETRACER_ACCESS_BIT, ETRACER_ACCESS_LVL_GPIO, 0);
      tracer_event_record(TRACER_LED3,!BIT(val8,6));
      UPDATE(LED3);
      REFRESH(LED3);
    }

  /* port 6 :                          */
  /* ========                          */
  /*   P6.7 ...                        */
  /*   P6.6 ...                        */
  /*   P6.5 ...                        */
  /*   P6.4 ...                        */
  /*   P6.3 ...                        */
  /*   P6.2 ...                        */
  /*   P6.1 ...                        */
  /*   P6.0 ...                        */
  /*
  if (msp430_digiIO_dev_read(PORT6,&val8))
    {

    }
  */

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

  /* Usart1                            */
  /* SPI  : flash                      */
  /* Uart : serial I/O                 */
  switch (MCU.usart1.mode)
    {
    case USART_MODE_SPI:
      if (msp430_usart1_dev_read_spi(&val8))
	{
	  machine.device[FLASH].write(FLASH, M25P_D, val8);
	  etracer_slot_access(0x0, 1, ETRACER_ACCESS_WRITE, ETRACER_ACCESS_BYTE, ETRACER_ACCESS_LVL_SPI1, 0);
	}
      break;
    case USART_MODE_UART:
      if (msp430_usart1_dev_read_uart(&val8))
	{
	  machine.device[SERIAL].write(SERIAL, PTTY_D, val8);
	  /* etracer_slot_access(0x0, 1, ETRACER_ACCESS_WRITE, ETRACER_ACCESS_BYTE, ETRACER_ACCESS_LVL_OUT, 0); */
	}
      break;
    default:
      break;
    }
    
  /* *************************************************************************** */
  /* devices -> MCU                                                              */
  /* *************************************************************************** */

  /* input on ds2411 */
  {
    uint32_t mask;
    uint32_t value;
    machine.device[DS24].read(DS24,&mask,&value);
    if (mask & DS2411_D)
      {
	msp430_digiIO_dev_write(PORT2,(value & DS2411_D) << 4, BIT4_MASK); // P2.4
	etracer_slot_access(0x0, 1, ETRACER_ACCESS_READ, ETRACER_ACCESS_BIT, ETRACER_ACCESS_LVL_GPIO, 0);
	tracer_event_record(TRACER_DS2411,value & DS2411_D);
      }
  }


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
  UPDATE(DS24);
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
