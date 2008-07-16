
/**
 *  \file   mica2.c
 *  \brief  Platform devices handling functions, Mica2 platform
 *  \author Antoine Fraboulet
 *  \date   2007
 **/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "arch/common/hardware.h"
#include "arch/atmega/atmega128.h"

#include "devices/devices.h"
#include "devices/led/led_dev.h"
#include "src/options.h"
#include "devices/cc2420/cc2420_dev.h"
#include "devices/ptty/ptty_dev.h"

#define LED1             0
#define LED2             1
#define LED3             2
#define SERIAL           3
#define MICA2_DEVICE_MAX 4

/*
 * Mica2
 * -----
 * Atmel   AtMega128L     - 7.3268MHz + 32.768KHz
 * Chipcon CC1000
 * Maxim   DS2401P
 * Atmel   AT45DB04       - 4Mb Flash
 *
 * Mica2dot
 * --------
 * Atmel   AtMega128LMLF  - 4MHz + 32.768KHz
 * Chipcon CC1100
 * Atmel   AT45DB041      - 4Mb Flash
 * / no ID ship /
 *
 */

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

static struct moption_t xtal_opt = {
  .longname    = "xtal",
  .type        = required_argument,
  .helpstring  = "xtal crystal freq (Hz)",
  .value       = NULL
};

static struct moption_t xosc_opt = {
  .longname    = "xosc",
  .type        = required_argument,
  .helpstring  = "xosc crystal freq (Hz)",
  .value       = NULL
};

int devices_options_add(void)
{
  options_add(&xtal_opt            );
  options_add(&xosc_opt            );
  ptty_add_options(SERIAL, 1, "serial1");
  return 0;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

int devices_create(void)
{
  int res = 0;
  int xtal_freq;
  int xosc_freq;

  xtal_freq = 7372800; /* 7.3728MHZ */
  xosc_freq =   32768; /* 32.768KHZ */

  if (xtal_opt.value) 
    {
      xtal_freq = atoi(xtal_opt.value);
      HW_DMSG_DEV("atmega: xtal external crystal set to %d Hz\n",xtal_freq);
    }

  if (xosc_opt.value) 
    {
      xosc_freq = atoi(xosc_opt.value);
      HW_DMSG_DEV("atmega: xtal external crystal set to %d Hz\n",xosc_freq);
    }

  /*********************************/
  /* MCU                           */
  /*********************************/

  res += atmega128_mcu_create(xtal_freq,xosc_freq);

  /*********************************/
  /* fix peripheral sizes          */
  /*********************************/

  machine.device_max          = MICA2_DEVICE_MAX;
  machine.device_size[LED1]   = led_device_size  ();    // Led1
  machine.device_size[LED2]   = led_device_size  ();    // Led2
  machine.device_size[LED3]   = led_device_size  ();    // Led3
  machine.device_size[SERIAL] = ptty_device_size ();

  /*********************************/
  /* allocate memory               */
  /*********************************/

  res += devices_memory_allocate();

  /*********************************/
  /* create peripherals            */
  /*********************************/

  res += led_device_create      (LED1,0xee0000,0,0);
  res += led_device_create      (LED2,0x00ee00,0,0);
  res += led_device_create      (LED3,0xee0000,0,0);
  res += ptty_device_create     (SERIAL,1);

  /*********************************/
  /* place peripherals Gui         */
  /*********************************/

#if defined(GUI)
  {
    int lw,lh;

    machine.device[LED1].ui_get_size(LED1,&lw,&lh);

    machine.device[LED1].ui_set_pos(LED1,    0,   0);
    machine.device[LED2].ui_set_pos(LED2, 1*lw,   0);
    machine.device[LED3].ui_set_pos(LED3, 2*lw,   0);
  }
#endif

  /*********************************/
  /* end of platform specific part */
  /*********************************/

  tracer_event_add_id(TRACER_LED1,  "led1", 1);
  tracer_event_add_id(TRACER_LED2,  "led2", 1);
  tracer_event_add_id(TRACER_LED3,  "led3", 1);

  return res;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

int devices_reset_post(void)
{
  return 0;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

int devices_update(void)
{
  return 0;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

