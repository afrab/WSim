
/**
 *  \file   led_dev.c
 *  \brief  Led display 
 *  \author Antoine Fraboulet
 *  \date   2005
 **/

#include <stdio.h>
#include <stdlib.h>

#include "arch/common/hardware.h"
#include "devices/led/led_img.h"
#include "devices/led/led_dev.h"

#define LED_DEV_NOACTION 0
#define LED_DEV_REFRESH  1

struct led_t {
  int   val;
  int   update;
  struct led_img_t *img;
};

#define ST_VAL    (((struct led_t*)(machine.device[dev].data))->val)
#define ST_UPDATE (((struct led_t*)(machine.device[dev].data))->update)
#define ST_IMG    (((struct led_t*)(machine.device[dev].data))->img)

int  led_reset       (int dev);
int  led_delete      (int dev);
void led_write       (int dev, uint32_t addr, uint32_t val);
int  led_update      (int dev);
int  led_ui_draw     (int dev);
void led_ui_get_size (int dev, int *w, int *h);
void led_ui_set_pos  (int dev, int x, int y);
void led_ui_get_pos  (int dev, int *x, int *y);

/***************************************************/
/***************************************************/
/***************************************************/

int led_device_size()
{
  return sizeof(struct led_t);
}

int led_device_create(int dev_num, uint32_t on, uint32_t off, uint32_t bg)
{
  struct led_t *dev = (struct led_t*) machine.device[dev_num].data;
  
  dev->img      = led_img_create(on,off,bg);
  dev->update   = 0;
  dev->val      = 0;

  machine.device[dev_num].reset         = led_reset;
  machine.device[dev_num].delete        = led_delete;

  machine.device[dev_num].write         = led_write;
  machine.device[dev_num].update        = led_update;

  machine.device[dev_num].ui_draw       = led_ui_draw;
  machine.device[dev_num].ui_get_size   = led_ui_get_size;
  machine.device[dev_num].ui_set_pos    = led_ui_set_pos;
  machine.device[dev_num].ui_get_pos    = led_ui_get_pos;

  machine.device[dev_num].state_size    = led_device_size();
  machine.device[dev_num].name          = "Led display";

  return 0;
}

int led_reset(int dev)
{
  ST_VAL    = 0;
  ST_UPDATE = 1;
  return 0;
}

int led_delete(int dev)
{
  led_img_delete(ST_IMG);
  return 0;
}

/***************************************************/
/***************************************************/
/***************************************************/

void led_write(int dev, uint32_t UNUSED addr, uint32_t val)
{
  ST_VAL = val;
  ST_UPDATE = 1;
}

int led_update(int UNUSED dev)
{
  return 0;
}

/***************************************************/
/***************************************************/
/***************************************************/

int led_ui_draw (int dev)
{
  if (ST_UPDATE == 1)
    {
      led_img_draw (ST_IMG, ST_VAL);
      ST_UPDATE = 0;
      return 1;
    }
  return 0;
}

void led_ui_get_size (int dev, int *w, int *h)
{
  *w = ST_IMG->w;
  *h = ST_IMG->h;
}

void led_ui_set_pos (int dev, int x, int y)
{
  ST_IMG->x = x;
  ST_IMG->y = y;
}

void led_ui_get_pos (int dev, int *x, int *y)
{
  *x = ST_IMG->x;
  *y = ST_IMG->y;
}


/***************************************************/
/***************************************************/
/***************************************************/
