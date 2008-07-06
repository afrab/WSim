
/**
 *  \file   bargraph_dev.c
 *  \brief  Bargraph display line
 *  \author Antoine Fraboulet
 *  \date   2005
 **/

#include <stdio.h>
#include <stdlib.h>

#include "arch/common/hardware.h"
#include "devices/bargraph/bargraph_img.h"
#include "devices/bargraph/bargraph_dev.h"

#define BARGRAPH_DEV_NOACTION 0
#define BARGRAPH_DEV_REFRESH  1

#define BARGRAPH_MAX  4

struct bargraph_t {
  int   n;
  int   val;
  int   update;
  struct bargraph_img_t *img;
};

#define ST_N      (((struct bargraph_t*)(machine.device[dev].data))->n)
#define ST_SEL    (((struct bargraph_t*)(machine.device[dev].data))->selector)
#define ST_VAL    (((struct bargraph_t*)(machine.device[dev].data))->val)
#define ST_IMG    (((struct bargraph_t*)(machine.device[dev].data))->img)
#define ST_UPDATE (((struct bargraph_t*)(machine.device[dev].data))->update)

int  bargraph_reset       (int dev);
int  bargraph_delete      (int dev);
void bargraph_write       (int dev, uint32_t mask, uint32_t val);
int  bargraph_update      (int dev);
int  bargraph_ui_draw     (int dev);
void bargraph_ui_get_size (int dev, int *w, int *h);
void bargraph_ui_set_pos  (int dev, int  x, int  y);
void bargraph_ui_get_pos  (int dev, int *x, int *y);

/***************************************************/
/***************************************************/
/***************************************************/


int bargraph_device_size()
{
  return sizeof(struct bargraph_t);
}


int bargraph_device_create(int dev_num, uint32_t on, uint32_t off, uint32_t bg)
{
  struct bargraph_t *dev = (struct bargraph_t*) machine.device[dev_num].data;

  /* 7 segs */
  dev->img      = bargraph_img_create(on,off,bg);
  dev->update   = 0;
  dev->val      = 0;

  machine.device[dev_num].reset         = bargraph_reset;
  machine.device[dev_num].delete        = bargraph_delete;

  machine.device[dev_num].write         = bargraph_write;
  machine.device[dev_num].update        = bargraph_update;

  machine.device[dev_num].ui_draw       = bargraph_ui_draw;
  machine.device[dev_num].ui_get_size   = bargraph_ui_get_size;
  machine.device[dev_num].ui_set_pos    = bargraph_ui_set_pos;
  machine.device[dev_num].ui_get_pos    = bargraph_ui_get_pos;

  machine.device[dev_num].state_size    = bargraph_device_size();
  machine.device[dev_num].name          = "Bargraph display";

  return 0;
}

/***************************************************/
/***************************************************/
/***************************************************/

int bargraph_reset(int dev)
{
  ST_VAL    = 0;
  ST_UPDATE = 1;
  return 0;
}

int bargraph_delete(int dev)
{
  bargraph_img_delete(ST_IMG);
  return 0;
}

/***************************************************/
/***************************************************/
/***************************************************/

void bargraph_write(int dev, uint32_t mask, uint32_t val)
{
  ST_VAL = val;
  ST_UPDATE = (mask != 0);
}

int bargraph_update(int UNUSED dev)
{
  return 0;
}

/***************************************************/
/***************************************************/
/***************************************************/

int bargraph_ui_draw (int dev)
{
  if (ST_UPDATE == 1)
    {
      bargraph_img_draw (ST_IMG, ST_VAL);
      ST_UPDATE = 0;
      return 1;
    }
  return 0;
}

void bargraph_ui_get_size (int dev, int *w, int *h)
{
  *w = ST_IMG->w;
  *h = ST_IMG->h;
}

void bargraph_ui_set_pos (int dev, int x, int y)
{
  ST_IMG->x = x;
  ST_IMG->y = y;
}

void bargraph_ui_get_pos (int dev, int *x, int *y)
{
  *x = ST_IMG->x;
  *y = ST_IMG->y;
}

/***************************************************/
/***************************************************/
/***************************************************/
