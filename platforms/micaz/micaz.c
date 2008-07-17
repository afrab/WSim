
/**
 *  \file   micaz.c
 *  \brief  Platform devices handling functions, MicaZ platform
 *  \author Antoine Fraboulet
 *  \date   2007
 **/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "arch/common/hardware.h"
#include "arch/atmega/atmega128.h"

#include "devices/cc2420/cc2420_dev.h"
#include "devices/ptty/ptty_dev.h"
#include "devices/devices.h"
#include "devices/led/led_dev.h"
#include "src/options.h"

#define SYSTEM           0
#define LED1             1
#define LED2             2
#define LED3             3
#define SERIAL           4
#define MICAZ_DEVICE_MAX 5

/*
 * MicaZ
 * -----
 * Atmel   AtMega128L     - 7.3268MHz + 32.768KHz
 * Chipcon CC2420
 * Maxim   DS2401P
 * Atmel   AT45DB014      - 4Mb Flash
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

struct micaz_struct_t {
  int flash_cs;
  int radio_cs;
};

#define SYSTEM_DATA      ((struct micaz_struct_t*)(machine.device[0].data))
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
  machine.device[dev_num].state_size    = sizeof(struct micaz_struct_t);
  machine.device[dev_num].name          = "System Platform";
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

  machine.device_max          = MICAZ_DEVICE_MAX;
  machine.device_size[SYSTEM] = sizeof(struct micaz_struct_t);
  machine.device_size[LED1]   = led_device_size();    // Led1
  machine.device_size[LED2]   = led_device_size();    // Led2
  machine.device_size[LED3]   = led_device_size();    // Led3
  machine.device_size[SERIAL] = ptty_device_size();

  /*********************************/
  /* allocate memory               */
  /*********************************/

  res += devices_memory_allocate();

  /*********************************/
  /* create peripherals            */
  /*********************************/

  res += system_create          (SYSTEM);
  res += led_device_create      (LED1,0xee0000,0,0);
  res += led_device_create      (LED2,0x00ee00,0,0);
  res += led_device_create      (LED3,0x0000ee,0,0);
  res += ptty_device_create     (SERIAL,1);

  /*********************************/
  /* place peripherals Gui         */
  /*********************************/

  {
    int lw,lh;

    machine.device[LED1].ui_get_size(LED1,&lw,&lh);

    machine.device[LED1].ui_set_pos(LED1,    0,   0);
    machine.device[LED2].ui_set_pos(LED2, 1*lw,   0);
    machine.device[LED3].ui_set_pos(LED3, 2*lw,   0);
  }

  /*********************************/
  /* end of platform specific part */
  /*********************************/

  tracer_event_add_id(TRACER_LED1, 1, "led1", "");
  tracer_event_add_id(TRACER_LED2, 1, "led2", "");
  tracer_event_add_id(TRACER_LED3, 1, "led3", "");

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

