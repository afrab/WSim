
/**
 *  \file   senslab.c
 *  \brief  Platform devices handling functions, WSN430 / Senslab platforms
 *  \author Antoine Fraboulet
 *  \date   2008
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
#include "devices/m25p80/m25p80_dev.h"
#include "devices/ds2411/ds2411_dev.h"

#if defined(SLABV13B)
#include "devices/cc1100_2500/cc1100_2500_dev.h"
#define RADIO_CSn_MASK  CC1100_CSn_MASK
#define RADIO_CSn_SHIFT CC1100_CSn_SHIFT
#define RADIO_DATA_MASK CC1100_DATA_MASK
#define RADIO_GDO0_MASK CC1100_GDO0_MASK   /* GDO0 -> P1.3 */
#define RADIO_GDO2_MASK CC1100_GDO2_MASK   /* GDO2 -> P1.4 */
#elif defined(SLABV14)
#include "devices/cc2420/cc2420_dev.h"
#define RADIO_CSn_MASK  CC2420_CSn_MASK
#define RADIO_CSn_SHIFT CC2420_BIT_CSn
#define RADIO_DATA_MASK CC2420_DATA_MASK
#define RADIO_GDO0_MASK CC2420_FIFO_MASK   /* FIFO -> P1.3 */
#define RADIO_GDO2_MASK CC2420_FIFOP_MASK  /* FIFOP -> P1.4 */
#define RADIO_SFD_MASK  CC2420_SFD_MASK    /* SFD -> P1.5 */
#define RADIO_CCA_MASK  CC2420_CCA_MASK    /* CCA -> P1.6 */
#define RADIO_RESET_MASK CC2420_RESET_MASK /* RESETn -> P1.7 */
#define RADIO_VREG_EN_MASK CC2420_VREG_EN_MASK  /* VREG_EN  -> P3.0 */
#endif

#include "devices/ptty/ptty_dev.h"
#include "devices/uigfx/uigfx_dev.h"
#include "src/options.h"

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

/* ****************************************
 * platform description for Senslab devices V1.3B/V1.4
 *
 * Port 1
 * ======
 *   1.7 : RESETn cc2420(V1.4)
 *   1.6 : CCA cc2420(V1.4)
 *   1.5 : SFD cc2420(V1.4)
 *   1.4 : GDO_2 cc1100(V1.3B)/cc2420(V1.4)
 *   1.3 : GDO_0 cc1100(V1.3B)/cc2420(V1.4)
 *   1.2 : flash Write protect
 *
 *
 * Port 2
 * ======
 *   2.4 : 1 wire
 *
 * Port 3
 * ======
 *   3.7 (urxd1)
 *   3.6 (utxd1)
 *   3.5 (urxd0)
 *   3.4 (utxd0)
 *   3.3 (uclk0) : serial clock, tsl2550
 *   3.2 (simo0) : serial in, tsl2550
 *   3.1 (somi0) : serial out
 *   3.0 : VREG_EN cc2420(V1.4)
 *
 * Port 4
 * ======
 *   4.7 : flash Hold
 *   4.6 : NC
 *   4.5 : tsl2550 Supply
 *   4.4 : flash CS
 *   4.3 : Enable -> battery charger
 *   4.2 : CS_radio cc1100(V1.3B)/cc2420(V1.4)
 *   4.1 : STATUS -> battery charger
 *
 * Port 5
 * ======
 *   5.6 : LED3
 *   5.5 : LED2
 *   5.4 : LED1
 *   5.3 (uclk1) : flash clock, CLOCK cc1100(V1.3B)/cc2420(V1.4)
 *   5.2 (somi1) : flash out, SDO cc1100(V1.3B)/cc2420(V1.4)
 *   5.1 (simo1) : flash in, SDI cc1100(V1.3B)/cc2420(V1.4)
 *   5.0 (ste1) : NC
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

#define FLASH_ID_0 0
#define SERIAL_ID_0 0

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

//no needs to customize xosc crystal freq for senslab platform
/*static struct moption_t xosc_opt = {
  .longname    = "xosc",
  .type        = required_argument,
  .helpstring  = "xosc crystal freq (Hz)",
  .value       = NULL
};*/

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

int devices_options_add(void)
{
  options_add( &xt1_opt         );
  options_add( &xt2_opt         );
  //options_add( &xosc_opt        );
  options_add( &ds2411_opt      );
  m25p_add_options(FLASH,  FLASH_ID_0, "flash"  );
  ptty_add_options(SERIAL, SERIAL_ID_0, "serial0");

  return 0;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

struct senslab_struct_t {
  int flash_cs;
  int radio_cs;
};

#define SYSTEM_DATA      ((struct senslab_struct_t*)(machine.device[SYSTEM].data))
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
  machine.device[dev_num].state_size    = sizeof(struct senslab_struct_t);
  machine.device[dev_num].name          = "System Platform";
  return 0;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

int devices_create(void)
{
#define M25P_DATA        (machine.device[FLASH].data)
  int res = 0;
  int xin_freq, xt2_freq, xosc_freq;

  xin_freq  =    32768; /* 32 kHz */
  xt2_freq  =  8000000; /*  8 MHz */

#if defined(SLABV13B)
  xosc_freq = 27000000; /* 27 MHz */
  char cc1100_antenna[] = "omnidirectionnal"; /* used by wsnet2, only this model available in wsnet2 */
#elif defined(SLABV14)
  xosc_freq = 16000000; /* 16 MHz */
  char cc2420_antenna[] = "omnidirectionnal"; /* used by wsnet2, only this model available in wsnet2 */
#endif

  if (xt1_opt.value) {
    xin_freq = atoi(xt1_opt.value);
    INFO("senslab: xt1 external crystal set to %d Hz\n",xin_freq);
  }

  if (xt2_opt.value) {
    xt2_freq = atoi(xt2_opt.value);
    INFO("senslab: xt2 external crystal set to %d Hz\n",xt2_freq);
  }

  /*if (xosc_opt.value) {
    xosc_freq = atoi(xosc_opt.value);
    INFO("senslab: xosc external crystal set to %d Hz\n",xosc_freq);
  }*/

  /*********************************/
  /* MSP430 MCU                    */
  /*********************************/

  res += msp430_mcu_create(xin_freq, xt2_freq);

  /*********************************/
  /* fix peripheral sizes          */
  /*********************************/

  machine.device_max          = BOARD_DEVICE_MAX;
  machine.device_size[SYSTEM] = sizeof(struct senslab_struct_t);
  machine.device_size[FLASH]  = m25p_device_size();   // Flash RAM
  machine.device_size[LED1]   = led_device_size();    // Led1
  machine.device_size[LED2]   = led_device_size();    // Led2
  machine.device_size[LED3]   = led_device_size();    // Led3
  machine.device_size[DS24]   = ds2411_device_size(); // dallas ds2411
  machine.device_size[SERIAL] = ptty_device_size();

#if defined(SLABV13B)
  machine.device_size[RADIO]  = cc1100_device_size(); // cc1100 radio
#elif defined(SLABV14)
  machine.device_size[RADIO]  = cc2420_device_size(); // cc2420 radio
#endif

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

  //#  include "img-senslab-small.xpm"
  //#  define IMG img_senslab_small
  //#  include "img-senslab.xpm"
  //#  define IMG img_senslab
#if defined(SLABV13B)
  #include "wsim13b.xpm"
  #define  IMG wsim__b
#elif defined(SLABV14)
  #include "wsim14.xpm"
  #define  IMG wsim__
#endif



  res += system_create          (SYSTEM);
  res += led_device_create      (LED1,    0xee0000,OFF,BKG,"red");
  res += led_device_create      (LED2,    0x00ee00,OFF,BKG,"green");
  res += led_device_create      (LED3,    0x0000ee,OFF,BKG,"blue");
  res += m25p_device_create     (FLASH,   FLASH_ID_0);
  res += ds2411_device_create   (DS24,    ds2411_opt.value);
#if defined(SLABV13B)
  res += cc1100_device_create   (RADIO,   xosc_freq / 1000000, cc1100_antenna);
#elif defined(SLABV14)
  res += cc2420_device_create   (RADIO,   xosc_freq / 1000000, cc2420_antenna);
#endif
  res += ptty_device_create     (SERIAL,  SERIAL_ID_0);
  res += uigfx_device_create    (LOGO1,   IMG);

  /*********************************/
  /* place peripherals Gui         */
  /*********************************/

  {
    int led_w,led_h;
    int log_w,log_h;

    machine.device[LED1 ].ui_get_size (LED1,   &led_w, &led_h);
    machine.device[LOGO1].ui_get_size (LOGO1,  &log_w, &log_h);
#if 1
    machine.device[LED1 ].ui_set_pos  (LED1,         0,   0);
    machine.device[LED2 ].ui_set_pos  (LED2,   1*led_w,   0);
    machine.device[LED3 ].ui_set_pos  (LED3,   2*led_w,   0);
#else
    machine.device[LED1 ].ui_set_pos  (LED1, log_w - 3*led_w,   0);
    machine.device[LED2 ].ui_set_pos  (LED2, log_w - 2*led_w,   0);
    machine.device[LED3 ].ui_set_pos  (LED3, log_w - 1*led_w,   0);
#endif
    machine.device[LOGO1].ui_set_pos (LOGO1,         0,   0);
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

  machine.device[FLASH].write(FLASH, M25P_W, M25P_W);
  SYSTEM_FLASH_CS = 0;
  SYSTEM_RADIO_CS = 0;

  REFRESH(LED1);
  REFRESH(LED2);
  REFRESH(LED3);
  REFRESH(LOGO1);
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
  int RADIO_CSn = 0;
  int FLASH_CS = 0;
  uint8_t  val8;

  /* *************************************************************************** */
  /* MCU -> devices                                                              */
  /* *************************************************************************** */

  /* port 1 :                               */
  /* ========                               */
  /*   P1.7 RESETn cc2420(V1.4)             */
  /*   P1.6 CCA cc2420(V1.4)                */
  /*   P1.5 SFD cc2420(V1.4)                */
  /*   P1.4 GDO_2 cc1100(V1.3B)/cc2420(V1.4)*/
  /*   P1.3 GDO_0 cc1100(V1.3B)/cc2420(V1.4)*/
  /*   P1.2 flash Write protect             */
  /*   P1.1 NC                              */
  /*   P1.0 NC                              */
  if (msp430_digiIO_dev_read(PORT1,&val8))
    {
      machine.device[FLASH].write(FLASH, M25P_W, BIT(val8,2) << M25P_W_SHIFT);
      etracer_slot_access(0x0, 1, ETRACER_ACCESS_WRITE, ETRACER_ACCESS_BIT, ETRACER_ACCESS_LVL_GPIO, 0);

#if defined(SLABV14)
      machine.device[RADIO].write(RADIO, RADIO_RESET_MASK, BIT(val8,7) << CC2420_BIT_RESET);
      etracer_slot_access(0x0, 1, ETRACER_ACCESS_WRITE, ETRACER_ACCESS_BIT, ETRACER_ACCESS_LVL_GPIO, 0);
#endif
    }
  
  /* port 2 :                               */
  /* ========                               */
  /*   P2.7 NC                              */
  /*   P2.6 NC                              */
  /*   P2.5 NC                              */
  /*   P2.4 1wire                           */
  /*   P2.3 NC                              */
  /*   P2.3 NC                              */
  /*   P2.1 7seg selector 1                 */
  /*   P2.0 7seg selector 0                 */
  if (msp430_digiIO_dev_read(PORT2,&val8))
    {
      machine.device[DS24].write(DS24,DS2411_D,BIT(val8,4)); 
      etracer_slot_access(0x0, 1, ETRACER_ACCESS_WRITE, ETRACER_ACCESS_BIT, ETRACER_ACCESS_LVL_GPIO, 0);
    }

  /* port 3 :                               */
  /* ========                               */
  /*   P3.7 urxd1 : serial                  */
  /*   P3.6 utxd1 : serial                  */
  /*   P3.5 urxd0                           */
  /*   P3.4 utxd0                           */ 
  /*   P3.3 NC                              */
  /*   P3.2 NC                              */
  /*   P3.1 NC                              */
  /*   P3.0 VREG_EN cc2420(V1.4)            */
#if defined(SLABV14)
  if (msp430_digiIO_dev_read(PORT3,&val8))
    {
      machine.device[RADIO].write(RADIO, RADIO_VREG_EN_MASK, BIT(val8,0) << CC2420_BIT_VREG_EN);
      etracer_slot_access(0x0, 1, ETRACER_ACCESS_WRITE, ETRACER_ACCESS_BIT, ETRACER_ACCESS_LVL_GPIO, 0);    
    }
#endif

  /* port 4 :                               */
  /* ========                               */
  /*   P4.7 flash hold                      */
  /*   P4.6 NC                              */
  /*   P4.5 NC                              */
  /*   P4.4 CS flash                        */
  /*   P4.3 NC                              */
  /*   P4.2 CS_radio cc1100(V1.3B)/cc2420(V1.4)*/
  /*   P4.1 batt status                     */
  /*   P4.0 NC                              */
  if (msp430_digiIO_dev_read(PORT4,&val8))
    {
      FLASH_CS = BIT(val8,4);
      machine.device[FLASH].write(FLASH, 
				  M25P_H | M25P_S, 
				  (BIT(val8,7) << M25P_H_SHIFT) | 
				  (BIT(val8,4) << M25P_S_SHIFT));
      /* waiting for flash update */
      etracer_slot_access(0x0, 1, ETRACER_ACCESS_WRITE, ETRACER_ACCESS_BIT, ETRACER_ACCESS_LVL_GPIO, 0);

      RADIO_CSn = BIT(val8,2);
      machine.device[RADIO].write(RADIO, RADIO_CSn_MASK, RADIO_CSn << RADIO_CSn_SHIFT);
      etracer_slot_access(0x0, 1, ETRACER_ACCESS_WRITE, ETRACER_ACCESS_BIT, ETRACER_ACCESS_LVL_GPIO, 0);
    }
  else
    {
      FLASH_CS = BIT(val8,4);
      RADIO_CSn = BIT(val8,2);
    }

  /* port 5 :                          */
  /* ========                          */
  /*   P5.7 NC                         */
  /*   P5.6 led 3 (Blue)               */
  /*   P5.5 led 2 (Green)              */
  /*   P5.4 led 1 (Red)                */
  /*   P5.3 flash clock, CLOCK cc1100(V1.3B)/cc2420(V1.4)*/
  /*   P5.2 flash out, SDO cc1100(V1.3B)/cc2420(V1.4)*/
  /*   P5.1 flash in, SDI cc1100(V1.3B)/cc2420(V1.4)*/
  /*   P5.0 NC                         */
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

      /* cc1100/cc2420 is driven by spi                   */
      /* we could/should check here that the pins are not */
      /* driven by the GPIO                               */
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

  /* Usart0                            */
  /* ==============                    */
  /* SPI 0 : NC                        */
  /* Uart 0 : serial I/O               */
  switch (MCU.usart0.mode)
    {
    case USART_MODE_SPI:
      break;
    case USART_MODE_UART:
      if (msp430_usart0_dev_read_uart(&val8))
	{
	  machine.device[SERIAL].write(SERIAL, PTTY_D, val8);
	  /* etracer_slot_access(0x0, 1, ETRACER_ACCESS_WRITE, ETRACER_ACCESS_BYTE, ETRACER_ACCESS_LVL_OUT, 0); */
	}
      break;
    default:
      break;
    }

  /* Usart1                            */
  /* ==============                    */
  /* SPI 1 : flash, radio              */
  /* Uart 1 : NC                       */
  switch (MCU.usart1.mode)
    {
    case USART_MODE_SPI:
      if (msp430_usart1_dev_read_spi(&val8))
	{
          if (!FLASH_CS && !RADIO_CSn)
	    {
#if defined(DEBUG_ME_HARDER)
              ERROR("senslab:devices: flash chip select and radio chip select enabled at the same time on SPI1\n");
#endif
            }
          machine.device[FLASH].write(FLASH, M25P_D, val8);
	  etracer_slot_access(0x0, 1, ETRACER_ACCESS_WRITE, ETRACER_ACCESS_BYTE, ETRACER_ACCESS_LVL_SPI1, 0);
	  machine.device[RADIO].write(RADIO, RADIO_DATA_MASK, val8);
	  etracer_slot_access(0x0, 1, ETRACER_ACCESS_WRITE, ETRACER_ACCESS_BYTE, ETRACER_ACCESS_LVL_SPI1, 0);
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

  /* input on ds2411 */
  {
    uint32_t mask;
    uint32_t value;
    machine.device[DS24].read(DS24,&mask,&value);
    if (mask & DS2411_D)
      {
	msp430_digiIO_dev_write(PORT2,(value & DS2411_D) << 4, BIT4_MASK); // P2.4
	etracer_slot_access(0x0, 1, ETRACER_ACCESS_READ, ETRACER_ACCESS_BIT, ETRACER_ACCESS_LVL_GPIO, 0);
      }
  }


  /* input on radio */
  {
    uint32_t mask  = 0;
    uint32_t value = 0;
    machine.device[ RADIO ].read(RADIO ,&mask,&value);
    if (mask & RADIO_DATA_MASK)
      {
	if (MCU.usart1.mode != USART_MODE_SPI)
	  {
#if defined(DEBUG_ME_HARDER)
	    ERROR("senslab:devices: read data on radio while not in SPI mode ?\n");
#endif
	  }
	msp430_usart1_dev_write_spi(value & RADIO_DATA_MASK);

	etracer_slot_access(0x0, 1, ETRACER_ACCESS_READ, ETRACER_ACCESS_BYTE, ETRACER_ACCESS_LVL_SPI1, 0);
      }

    /* TODO :: need to handle SO pin on CC1100 */

    if (mask & RADIO_GDO2_MASK) // GDO2 -> P1.4
      { 
	msp430_digiIO_dev_write(PORT1, (RADIO_GDO2_MASK & value) ? 0x10 : 0x00, 0x10);
	etracer_slot_access(0x0, 1, ETRACER_ACCESS_READ, ETRACER_ACCESS_BIT, ETRACER_ACCESS_LVL_GPIO, 0);
      }
    if (mask & RADIO_GDO0_MASK) // GDO0 -> P1.3
      { 
	msp430_digiIO_dev_write(PORT1, (RADIO_GDO0_MASK & value) ? 0x08 : 0x00, 0x08);
	etracer_slot_access(0x0, 1, ETRACER_ACCESS_READ, ETRACER_ACCESS_BIT, ETRACER_ACCESS_LVL_GPIO, 0);
      }

#if defined(SLABV14)
    if (mask & RADIO_SFD_MASK) // SFD -> P1.5
      { 
	msp430_digiIO_dev_write(PORT1, (RADIO_SFD_MASK & value) ? 0x20 : 0x00, 0x20);
	etracer_slot_access(0x0, 1, ETRACER_ACCESS_READ, ETRACER_ACCESS_BIT, ETRACER_ACCESS_LVL_GPIO, 0);
      }
    if (mask & RADIO_CCA_MASK) // CCA -> P1.6
      { 
	msp430_digiIO_dev_write(PORT1, (RADIO_CCA_MASK & value) ? 0x40 : 0x00, 0x40);
	etracer_slot_access(0x0, 1, ETRACER_ACCESS_READ, ETRACER_ACCESS_BIT, ETRACER_ACCESS_LVL_GPIO, 0);
      }
#endif
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
	    ERROR("senslab:devices: read data on flash while not in SPI mode ?\n");
	  }
	msp430_usart1_dev_write_spi(value & 0x00FF);
	etracer_slot_access(0x0, 1, ETRACER_ACCESS_READ, ETRACER_ACCESS_BYTE, ETRACER_ACCESS_LVL_SPI1, 0);
      }

   }


  /* input on UART serial */
  if (msp430_usart0_dev_write_uart_ok())
    {
      uint32_t mask,value;
      machine.device[SERIAL].read(SERIAL,&mask,&value);
      if ((mask & PTTY_D) != 0)
	{
	  msp430_usart0_dev_write_uart(value & PTTY_D);
	  /* etracer_slot_access(0x0, 1, ETRACER_ACCESS_READ, ETRACER_ACCESS_BYTE, ETRACER_ACCESS_LVL_OUT, 0); */
	}
    }


  /* input on UI */
  ui_default_input("senslab:");

  /* *************************************************************************** */
  /* update                                                                      */
  /* *************************************************************************** */
  LIBSELECT_UPDATE();
  LIBWSNET_UPDATE();

  UPDATE(RADIO);
  UPDATE(FLASH);
  UPDATE(DS24);
  UPDATE(SERIAL);

  ui_refresh(refresh);

  return res;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */
