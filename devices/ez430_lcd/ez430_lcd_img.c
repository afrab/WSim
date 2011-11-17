/**
 *  \file   ez430_lcd_dev.c
 *  \brief  ez430 Chronos LCD (based upon 7seg)
 *  \author Bernhard Dick
 *  \date   2011
 **/

#include <stdio.h>
#include <stdlib.h>

#include "arch/common/hardware.h"
#include "devices/ez430_lcd/ez430_lcd_img.h"
#include "devices/ez430_lcd/ez430_lcd_dev.h"

#define EZ430_LCD_DEV_NOACTION 0
#define EZ430_LCD_DEV_REFRESH  1

#define EZ430_LCD_MAX  4

struct ez430_lcd_t {
  int n;
  int val;
  int update;
  uint8_t mem[15];
  uint8_t bmem[15];
  struct ez430_lcd_img_t *img;
};

#define ST_N      (((struct ez430_lcd_t*)(machine.device[dev].data))->n)
#define ST_SEL    (((struct ez430_lcd_t*)(machine.device[dev].data))->selector)
#define ST_VAL    (((struct ez430_lcd_t*)(machine.device[dev].data))->val)
#define ST_IMG    (((struct ez430_lcd_t*)(machine.device[dev].data))->img)
#define ST_UPDATE (((struct ez430_lcd_t*)(machine.device[dev].data))->update)
#define ST_MEM    (((struct ez430_lcd_t*)(machine.device[dev].data))->mem)
#define ST_BMEM   (((struct ez430_lcd_t*)(machine.device[dev].data))->bmem)


/***************************************************/
/***************************************************/

/***************************************************/


int ez430_lcd_device_size()
{
  return sizeof (struct ez430_lcd_t);
}

int ez430_lcd_device_create(int dev_num)
{
  int i;
  struct ez430_lcd_t *dev = (struct ez430_lcd_t*) machine.device[dev_num].data;

  dev->img = ez430_lcd_img_create();
  dev->update = 0;
  dev->val = 0;
  for (i = 0; i < 15; i++) {
    dev->mem[i] = 0x00;
    dev->bmem[i] = 0x00;
  }

  machine.device[dev_num].reset = ez430_lcd_reset;
  machine.device[dev_num].delete = ez430_lcd_delete;

  machine.device[dev_num].write = ez430_lcd_write;
  machine.device[dev_num].update = ez430_lcd_update;

  machine.device[dev_num].ui_draw = ez430_lcd_ui_draw;
  machine.device[dev_num].ui_get_size = ez430_lcd_ui_get_size;
  machine.device[dev_num].ui_set_pos = ez430_lcd_ui_set_pos;
  machine.device[dev_num].ui_get_pos = ez430_lcd_ui_get_pos;

  machine.device[dev_num].state_size = ez430_lcd_device_size();
  machine.device[dev_num].name = "EZ430 Chronos LCD Display";

  return 0;
}

/***************************************************/
/***************************************************/

/***************************************************/

int ez430_lcd_reset(int dev)
{
  ST_VAL = 0;
  ST_UPDATE = 1;
  return 0;
}

int ez430_lcd_delete(int dev)
{
  ez430_lcd_img_delete(ST_IMG);
  return 0;
}

/***************************************************/
/***************************************************/

/***************************************************/

void ez430_lcd_write(int dev, uint32_t mask, uint32_t val)
{
  ST_UPDATE = (mask != 0 && (ST_VAL != val)); // prevent heavy load
  ST_VAL = val;
}

void ez430_lcd_regwrite(int dev, uint8_t mem[15], uint8_t bmem[15])
{
  int i, j;
  ST_UPDATE = 0;
  for (i = 0; i < 15; i++) {
    if (ST_MEM[i] != mem[i] || ST_BMEM[i] != bmem[i]) {
      for (j = 0; j < 15; j++) {
        ST_MEM[j] = mem[j];
        ST_BMEM[j] = bmem[j];
      }
      ST_UPDATE = 1;
      return;
    }
  }
}

int ez430_lcd_update(int UNUSED dev)
{
  return 0;
}

/***************************************************/
/***************************************************/

/***************************************************/

int ez430_lcd_ui_draw(int dev)
{
  if (ST_UPDATE == 1) {
    ez430_lcd_img_draw(ST_IMG, ST_MEM, ST_BMEM);
    ST_UPDATE = 0;
    return 1;
  }
  return 0;
}

void ez430_lcd_ui_get_size(int dev, int *w, int *h)
{
  *w = ST_IMG->w;
  *h = ST_IMG->h;
}

void ez430_lcd_ui_set_pos(int dev, int x, int y)
{
  ST_IMG->x = x;
  ST_IMG->y = y;
}

void ez430_lcd_ui_get_pos(int dev, int *x, int *y)
{
  *x = ST_IMG->x;
  *y = ST_IMG->y;
}

/***************************************************/
/***************************************************/
/***************************************************/
