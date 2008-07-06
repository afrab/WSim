
/**
 *  \file   7seg_dev.c
 *  \brief  7 Segments display line
 *  \author Antoine Fraboulet
 *  \date   2005
 **/

#include <stdio.h>
#include <stdlib.h>

#include "arch/common/hardware.h"
#include "devices/7seg/7seg_img.h"
#include "devices/7seg/7seg_dev.h"

#define SEG7_DEV_NOACTION 0
#define SEG7_DEV_REFRESH  1

#define SEG7_MAX  4

struct seg7_t {
  int   n;
  int   val;
  int   update;
  struct seg7_img_t *img;
};

#define ST_N      (((struct seg7_t*)(machine.device[dev].data))->n)
#define ST_SEL    (((struct seg7_t*)(machine.device[dev].data))->selector)
#define ST_VAL    (((struct seg7_t*)(machine.device[dev].data))->val)
#define ST_IMG    (((struct seg7_t*)(machine.device[dev].data))->img)
#define ST_UPDATE (((struct seg7_t*)(machine.device[dev].data))->update)

int  seg7_reset       (int dev);
int  seg7_delete      (int dev);
void seg7_write       (int dev, uint32_t mask, uint32_t val);
int  seg7_update      (int dev);
int  seg7_ui_draw     (int dev);
void seg7_ui_get_size (int dev, int *w, int *h);
void seg7_ui_set_pos  (int dev, int  x, int  y);
void seg7_ui_get_pos  (int dev, int *x, int *y);

/***************************************************/
/***************************************************/
/***************************************************/


int sevenseg_device_size()
{
  return sizeof(struct seg7_t);
}


int sevenseg_device_create(int dev_num)
{
  struct seg7_t *dev = (struct seg7_t*) machine.device[dev_num].data;

  /* 7 segs */
  dev->img      = seg7_img_create();
  dev->update   = 0;
  dev->val      = 0;

  machine.device[dev_num].reset         = seg7_reset;
  machine.device[dev_num].delete        = seg7_delete;

  machine.device[dev_num].write         = seg7_write;
  machine.device[dev_num].update        = seg7_update;

  machine.device[dev_num].ui_draw       = seg7_ui_draw;
  machine.device[dev_num].ui_get_size   = seg7_ui_get_size;
  machine.device[dev_num].ui_set_pos    = seg7_ui_set_pos;
  machine.device[dev_num].ui_get_pos    = seg7_ui_get_pos;

  machine.device[dev_num].state_size    = sevenseg_device_size();
  machine.device[dev_num].name          = "7 segments display";

  return 0;
}

/***************************************************/
/***************************************************/
/***************************************************/

int seg7_reset(int dev)
{
  ST_VAL    = 0;
  ST_UPDATE = 1;
  return 0;
}

int seg7_delete(int dev)
{
  seg7_img_delete(ST_IMG);
  return 0;
}

/***************************************************/
/***************************************************/
/***************************************************/

void seg7_write(int dev, uint32_t mask, uint32_t val)
{
  ST_VAL = val;
  ST_UPDATE = (mask != 0);
}

int seg7_update(int UNUSED dev)
{
  return 0;
}

/***************************************************/
/***************************************************/
/***************************************************/

int seg7_ui_draw (int dev)
{
  if (ST_UPDATE == 1)
    {
      seg7_img_draw (ST_IMG, ST_VAL);
      ST_UPDATE = 0;
      return 1;
    }
  return 0;
}

void seg7_ui_get_size (int dev, int *w, int *h)
{
  *w = ST_IMG->w;
  *h = ST_IMG->h;
}

void seg7_ui_set_pos (int dev, int x, int y)
{
  ST_IMG->x = x;
  ST_IMG->y = y;
}

void seg7_ui_get_pos (int dev, int *x, int *y)
{
  *x = ST_IMG->x;
  *y = ST_IMG->y;
}

/***************************************************/
/***************************************************/
/***************************************************/
