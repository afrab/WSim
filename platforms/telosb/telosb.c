
/**
 *  \file   telosb.c
 *  \brief  Platform devices handling functions, Telosb platform
 *  \author Antoine Fraboulet
 *  \date   2007
 **/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#undef  DEBUG
#define DEBUG_ME_HARDER 0

#include "arch/common/hardware.h"
#include "arch/msp430/msp430.h"

#include "devices/devices.h"
#include "devices/led/led_dev.h"
#include "devices/m25p80/m25p80_dev.h"
#include "devices/ds2411/ds2411_dev.h"
#include "devices/cc2420/cc2420_dev.h"
#include "devices/ptty/ptty_dev.h"
#include "devices/uigfx/uigfx_dev.h"
#include "src/options.h"


/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

/* ***************************************
 * platform description for Telosb devices
 *
 * Port 1
 * ======
 *   1.7 : Hum_pwr
 *   1.6 : Hum_scl
 *   1.5 : Hum_sda
 *   1.4 : in, cc2420 radio_gio1
 *   1.3 : in, cc2420 radio_gio0
 *   1.2 : p_dvcc
 *   1.1 : UART1TX loopback, bsltx
 *   1.0 : in, cc2420 pkt_in
 *
 * Port 2
 * ======
 *   2.7 : ta0, UserInt = user switch SW2
 *   2.6 : GPIO 3, expansion connector U28
 *   2.5 : NC
 *   2.4 : 1 wire
 *   2.3 : CC2420, Gio2
 *   2.2 : ta0, caout,  + UART1RX loopback (bslrx)
 *   2.1 : GPIO 2, expansion connector U28
 *   2.0 : GPIO 1, expansion connector U28
 *
 * Port 3
 * ======
 *   3.7 : uart1RX
 *   3.6 : uart1TX
 *   3.5 : uart0RX
 *   3.4 : uart0TX
 *   3.3 : uclk0, serial clock, Radio_sclk
 *   3.2 : simo0, Radio_Slave_0ut
 *   3.1 : somi0, Radio_Slave_In
 *   3.0 : ste0 : NC
 *
 * Port 4
 * ======
 *   4.7 : out, flash_hold
 *   4.6 : out, radio_reset
 *   4.5 : out, radio_vref_en
 *   4.4 : out, flash CS
 *   4.3 : NC
 *   4.2 : out, radio_cs
 *   4.1 : in,  radio_sfd
 *   4.0 : NC
 *
 * Port 5
 * ======
 *   5.7 : SVSout (unused)
 *   5.6 : LED3
 *   5.5 : LED2
 *   5.4 : LED1
 *   5.3 : NC
 *   5.2 : NC
 *   5.1 : NC
 *   5.0 : NC
 *
 * Port 6
 * ======
 *   6.7 : ADC7, SVSin
 *   6.6 : ADC6, DAC0
 *   6.5 : ADC5
 *   6.4 : ADC4
 *   6.3 : ADC3
 *   6.2 : ADC2
 *   6.1 : ADC1
 *   6.0 : ADC0
 *
 * ***************************************/

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

#define SYSTEM           0
#define LED1             1
#define LED2             2
#define LED3             3
#define FLASH            4
#define DS24             5
#define RADIO            6
#define SERIAL           7
#define LOGO1            8

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

/* xt2 is not used on telosb */
/*
 * static struct moption_t xt2_opt = {
 *   .longname    = "xt2",
 *   .type        = required_argument,
 *   .helpstring  = "xt2 crystal freq (Hz)",
 *   .value       = NULL
 * };
 */

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

int devices_options_add(void)
{
  options_add(&xt1_opt             );
  options_add(&ds2411_opt          );
  m25p_add_options (FLASH,  0, "flash"   );
  ptty_add_options (SERIAL, 1, "serial1" );

  return 0;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

struct telosb_struct_t {
  int flash_cs;
  int radio_cs;
  int p2dir;
};

#define SYSTEM_DATA      ((struct telosb_struct_t*)(machine.device[0].data))
#define SYSTEM_FLASH_CS  (SYSTEM_DATA->flash_cs)
#define SYSTEM_RADIO_CS  (SYSTEM_DATA->radio_cs)
#define SYSTEM_P2DIR     (SYSTEM_DATA->p2dir)

int system_reset (int UNUSED dev) 
{ 
  SYSTEM_FLASH_CS = 0;
  SYSTEM_RADIO_CS = 0;
  SYSTEM_P2DIR    = msp430_digiIO_dev_read_dir(PORT2);
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
  machine.device[dev_num].state_size    = sizeof(struct telosb_struct_t);
  machine.device[dev_num].name          = "System Platform";

  system_reset(dev_num);
  return 0;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

int devices_create(void)
{
  int res = 0;
  int xin_freq, xt2_freq;
  char cc2420_antenna[] = "omnidirectionnal"; /* used by wsnet2, only this model available in wsnet2 */

  xin_freq = 32768;
  xt2_freq = 0; /* xt2 is not used on telosb */

  if (xt1_opt.value) {
    xin_freq = atoi(xt1_opt.value);
    HW_DMSG_CLOCK("telosb: xt1 external crystal set to %d Hz\n",xin_freq);
  }

  /*********************************/
  /* MSP430 MCU                    */
  /*********************************/

  res += msp430_mcu_create(xin_freq, xt2_freq);

  /*********************************/
  /* fix peripheral sizes          */
  /*********************************/

  machine.device_max          = BOARD_DEVICE_MAX;
  machine.device_size[SYSTEM] = sizeof(struct telosb_struct_t);
  machine.device_size[LED1]   = led_device_size();    // Led1
  machine.device_size[LED2]   = led_device_size();    // Led2
  machine.device_size[LED3]   = led_device_size();    // Led3
  machine.device_size[FLASH]  = m25p_device_size();   // Flash RAM
  machine.device_size[DS24]   = ds2411_device_size(); // dallas ds2411
  machine.device_size[RADIO]  = cc2420_device_size(); // cc2420 radio
  machine.device_size[SERIAL] = ptty_device_size();
  machine.device_size[LOGO1]  = uigfx_device_size();

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
#  define WSIMLOGO wsim
#else
#  define BKG 0x202020
#  define OFF 0x404040
#endif

  res += system_create          (SYSTEM);
  res += led_device_create      (LED1,    0xee0000,OFF,BKG,"red");
  res += led_device_create      (LED2,    0x00ee00,OFF,BKG,"green");
  res += led_device_create      (LED3,    0x0000ee,OFF,BKG,"blue");
  res += m25p_device_create     (FLASH,   0);
  res += ds2411_device_create   (DS24,    ds2411_opt.value);
  res += cc2420_device_create   (RADIO,   16, cc2420_antenna);
  res += ptty_device_create     (SERIAL,  1);
  res += uigfx_device_create    (LOGO1,   WSIMLOGO);

  /*********************************/
  /* place peripherals Gui         */
  /*********************************/

  {
    int lw,lh;
    int log_w,log_h;

    machine.device[LED1].ui_get_size (LED1,&lw,&lh);
    machine.device[LOGO1].ui_get_size(LOGO1, &log_w, &log_h);

    machine.device[LED1 ].ui_set_pos (LED1,    0,   0);
    machine.device[LED2 ].ui_set_pos (LED2, 1*lw,   0);
    machine.device[LED3 ].ui_set_pos (LED3, 2*lw,   0);
    machine.device[LOGO1].ui_set_pos (LOGO1,   0,   0);
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

  /* flash W~ is set to Vcc */
  machine.device[FLASH].write(FLASH, M25P_W, M25P_W);
  SYSTEM_FLASH_CS = 0;
  SYSTEM_RADIO_CS = 0;
  SYSTEM_P2DIR    = msp430_digiIO_dev_read_dir(PORT2);

  REFRESH(LOGO1);
  REFRESH(LED1);
  REFRESH(LED2);
  REFRESH(LED3);
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
  uint8_t  dir8;
  uint8_t  wrt;
  /* HW_DMSG_DEV("telosb: ==========================================\n"); */

  /* *************************************************************************** */
  /* MCU -> devices                                                              */
  /* *************************************************************************** */
  /* Port 1
   * ======
   *   1.7 : Hum_pwr
   *   1.6 : Hum_scl
   *   1.5 : Hum_sda
   *   1.4 : in, cc2420 radio_gio1
   *   1.3 : in, cc2420 radio_gio0
   *   1.2 : p_dvcc
   *   1.1 : UART1TX
   *   1.0 : in, cc2420 pkt_in
   */

   if (msp430_digiIO_dev_read(PORT1,&val8))
    {
      machine.device[FLASH].write(FLASH, M25P_W , (BIT(val8,2) << M25P_W_SHIFT));
    }

  /* port 2 :
   * ========
   *   2.7 : ta0, UserInt = user switch SW2
   *   2.6 : GPIO 3, expansion connector U28
   *   2.5 : NC
   *   2.4 : 1 wire --> ds2411
   *   2.3 : CC2420, Gio2
   *   2.2 : ta0, caout,  + UART1RX loopback
   *   2.1 : GPIO 2, expansion connector U28
   *   2.0 : GPIO 1, expansion connector U28
   */
  dir8 = msp430_digiIO_dev_read_dir(PORT2);
  wrt  = msp430_digiIO_dev_read(PORT2,&val8);

  if (BIT(SYSTEM_P2DIR,4) != BIT(dir8,4))
    {
      if (BIT(dir8,4) != 0) // Set to output, onewire == 0 OR P2OUT
	{
	  machine.device[DS24].write(DS24,DS2411_D,BIT(val8,4)); 
	}
      else // Set to input, external resistor pulls high
	{
	  machine.device[DS24].write(DS24,DS2411_D,1);
	}
    }
  else
    {
      if ((BIT(dir8,4) != 0) && wrt) // output
	{
	  machine.device[DS24].write(DS24,DS2411_D,BIT(val8,4)); 
	}
    }

  SYSTEM_P2DIR = dir8;

  /* port 3 :
   * ========
   *   3.7 : uart1RX
   *   3.6 : uart1TX
   *   3.5 : uart0RX
   *   3.4 : uart0TX
   *   3.3 : uclk0, serial clock, Radio_sclk
   *   3.2 : simo0, Radio_Slave_0ut
   *   3.1 : somi0, Radio_Slave_In
   *   3.0 : ste0 : NC
   */
#if 0
  if (msp430_digiIO_dev_read(PORT3,&val8))
    {
      /* cc2420 is driven by spi                          */
      /* we could/should check here that the pins are not */
      /* driven by the GPIO                               */
    }
#endif

  /* port 4 :
   * ========
   *   4.7 : out, flash_hold~
   *   4.6 : out, radio_reset
   *   4.5 : out, radio_vref_en
   *   4.4 : out, flash CS~
   *   4.3 : NC
   *   4.2 : out, radio_cs
   *   4.1 : in,  radio_sfd
   *   4.0 : NC
   */
  if (msp430_digiIO_dev_read(PORT4,&val8))
    {
      uint32_t msk;
      uint32_t val;

#if (DEBUG_ME_HARDER != 0)
      if (BIT(val8,4) != SYSTEM_FLASH_CS)
	{
	  HW_DMSG_DEV("telosb: flash CSn set to %d\n",BIT(val8,4));
	}
      if (BIT(val8,2) != SYSTEM_RADIO_CS)
	{
	  HW_DMSG_DEV("telosb: cc2420 CSn set to %d\n",BIT(val8,2));
	}
#endif

      msk = M25P_H | M25P_S; 
      val = (BIT(val8,7) << M25P_H_SHIFT) | (BIT(val8,4) << M25P_S_SHIFT);
      HW_DMSG_MISC("telosb: write to flash msk:0x%04x val:0x%04x\n",msk,val);
      machine.device[FLASH].write(FLASH, msk, val); 
      SYSTEM_FLASH_CS = BIT(val8,4);
      /* waiting for flash update */
      
      /* cc2420 CS out, SFD in */
      msk = CC2420_CSn_MASK | CC2420_RESET_MASK | CC2420_VREG_EN_MASK;
      val = 
	(BIT(val8,2) << CC2420_BIT_CSn) | 
	(BIT(val8,5) << CC2420_BIT_VREG_EN) | 
	(BIT(val8,6) << CC2420_BIT_RESET);
      HW_DMSG_MISC("telosb: write to radio msk:0x%04x val:0x%04x\n",msk,val); 
      machine.device[RADIO].write(RADIO, msk, val);
      SYSTEM_RADIO_CS = BIT(val8,2);
      /* waiting for cc2420 update */
    }

  /* port 5 :
   * ========
   *   5.7 : SVSout (unused)
   *   5.6 : LED3
   *   5.5 : LED2
   *   5.4 : LED1
   *   5.3 : NC
   *   5.2 : NC
   *   5.1 : NC
   *   5.0 : NC
   */
  if (msp430_digiIO_dev_read(PORT5,&val8))
    {
      machine.device[LED1].write(LED1,LED_DATA, ! BIT(val8,4));
      UPDATE(LED1);
      REFRESH(LED1);

      machine.device[LED2].write(LED2,LED_DATA, !  BIT(val8,5));
      UPDATE(LED2);
      REFRESH(LED2);

      machine.device[LED3].write(LED3,LED_DATA, ! BIT(val8,6));
      UPDATE(LED3);
      REFRESH(LED3);
    }

  /* port 6 :
   * ========
   *   6.7 : ADC7, SVSin
   *   6.6 : ADC6, DAC0
   *   6.5 : ADC5
   *   6.4 : ADC4
   *   6.3 : ADC3
   *   6.2 : ADC2
   *   6.1 : ADC1
   *   6.0 : ADC0
   */
#if 0
  if (msp430_digiIO_dev_read(PORT6,&val8))
    {
    }
#endif

  /* Usart SPI mode                    */
  /* ==============                    */
  /* USART 0 : radio + flash           */

  switch (MCU.usart0.mode)
    {
    case USART_MODE_SPI:
      if (msp430_usart0_dev_read_spi(&val8))
	{
	  /*
	   * FLASH_CS == 0 -- flash on
	   * RADIO_CS == 0 -- radio on
	   */
	  if ((SYSTEM_FLASH_CS == 0) && (SYSTEM_RADIO_CS == 0))
	    {
	      WARNING("telosb: Flash and Radio SPI are selected at the same time\n");
	    }

	  machine.device[FLASH].write(FLASH, M25P_D, val8);
	  machine.device[RADIO].write(RADIO, CC2420_DATA_MASK, val8);
	}
      break;
    case USART_MODE_UART:
      /* Uart0 on expansion port U28 */
      break;
    case USART_MODE_I2C:
      break;
    }

  /* Usart1                            */
  /* Uart : serial I/O                 */
  /*                                   */
  /*                                   */
  switch (MCU.usart1.mode)
    {
    case USART_MODE_SPI:
      break;
    case USART_MODE_UART:
      if (msp430_usart1_dev_read_uart(&val8))
	{
	  machine.device[SERIAL].write(SERIAL, PTTY_D, val8);
	}
      break;
    case USART_MODE_I2C:
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
      }
  }

  /* input on radio */
  {
    uint32_t mask  = 0;
    uint32_t value = 0;
    machine.device[ RADIO ].read( RADIO ,&mask,&value);
    if ((mask & CC2420_DATA_MASK) != 0)
      {
	if (MCU.usart0.mode != USART_MODE_SPI)
	  {
	    ERROR("devices:internal: read data on CC2420 radio while not in SPI mode\n");
	  }
	if (SYSTEM_RADIO_CS != 0)
	  {
	    ERROR("devices:internal: read data on CC2420 radio with CS not enabled\n");
	  }
	msp430_usart0_dev_write_spi(value & CC2420_DATA_MASK);
	// etracer_slot_access(0x0, 1, ETRACER_ACCESS_READ, ETRACER_ACCESS_BYTE, ETRACER_ACCESS_LVL_SPI0, 0);
      }

    // << RADIO_SFD     - CC2420_SFD   - P4.1
    // << RADIO_PKT_INT - CC2420_FIFOP - P1.0
    // << RADIO_GIO0    - CC2420_FIFO  - P1.3
    // << RADIO_GIO1    - CC2420_CCA   - P1.4

    if (mask & CC2420_SFD_MASK) 
      { 
	msp430_digiIO_dev_write(PORT4, (value & CC2420_SFD_MASK)   ? 0x02 : 0x00, 0x02);
	// etracer_slot_access(0x0, 1, ETRACER_ACCESS_READ, ETRACER_ACCESS_BIT, ETRACER_ACCESS_LVL_GPIO, 0);
      }
    if (mask & CC2420_FIFOP_MASK)
      { 
	msp430_digiIO_dev_write(PORT1, (value & CC2420_FIFOP_MASK) ? 0x01 : 0x00, 0x01);
	// etracer_slot_access(0x0, 1, ETRACER_ACCESS_READ, ETRACER_ACCESS_BIT, ETRACER_ACCESS_LVL_GPIO, 0);
      }
    if (mask & CC2420_FIFO_MASK)
      {
	msp430_digiIO_dev_write(PORT1, (value & CC2420_FIFO_MASK)  ? 0x08 : 0x00, 0x08);
	// etracer_slot_access(0x0, 1, ETRACER_ACCESS_READ, ETRACER_ACCESS_BIT, ETRACER_ACCESS_LVL_GPIO, 0);
      }
    if (mask & CC2420_CCA_MASK)
      {
	msp430_digiIO_dev_write(PORT1, (value & CC2420_CCA_MASK)   ? 0x10 : 0x00, 0x10);
	// etracer_slot_access(0x0, 1, ETRACER_ACCESS_READ, ETRACER_ACCESS_BIT, ETRACER_ACCESS_LVL_GPIO, 0);
      }
  }


  /* input on flash */
  {
    uint32_t mask  = 0;
    uint32_t value = 0;
    machine.device[ FLASH ].read( FLASH ,&mask,&value);
    if ((mask & M25P_D) != 0)
      {
	if (MCU.usart0.mode != USART_MODE_SPI)
	  {
	    ERROR("devices:internal: read data on STM flash while not in SPI mode\n");
	  }
	if (SYSTEM_FLASH_CS != 0)
	  {
	    ERROR("devices:internal: read data on STM flash with CS not enabled\n");
	  }
	msp430_usart0_dev_write_spi(value & M25P_D);
	// ERROR("device: flash write 0x%02x on SPI0 from flash\n",value & M25P_D);
	// etracer_slot_access(0x0, 1, ETRACER_ACCESS_READ, ETRACER_ACCESS_BYTE, ETRACER_ACCESS_LVL_SPI1, 0);
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
	  // etracer_slot_access(0x0, 1, ETRACER_ACCESS_READ, ETRACER_ACCESS_BYTE, ETRACER_ACCESS_LVL_OUT, 0);
	}
    }


  /* input on UI */
  ui_default_input("telosb:");

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
