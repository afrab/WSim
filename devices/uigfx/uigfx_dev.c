
/**
 *  \file   uigfx_dev.c
 *  \brief  UI bitmap display gadget
 *  \author Antoine Fraboulet
 *  \date   2008
 **/

#include <stdio.h>
#include <stdlib.h>

#include "config.h"
#include "arch/common/hardware.h"
#include "devices/uigfx/uigfx_dev.h"
#include "devices/uigfx/uigfx_img.h"

#define UIGFX_DEV_NOACTION 0
#define UIGFX_DEV_REFRESH  1

struct uigfx_t {
  int   val;
  int   update;
  struct uigfx_img_t *img;
};

#define ST_VAL    (((struct uigfx_t*)(machine.device[dev].data))->val)
#define ST_UPDATE (((struct uigfx_t*)(machine.device[dev].data))->update)
#define ST_IMG    (((struct uigfx_t*)(machine.device[dev].data))->img)

int  uigfx_reset       (int dev);
int  uigfx_delete      (int dev);
void uigfx_write       (int dev, uint32_t addr, uint32_t val);
int  uigfx_update      (int dev);
int  uigfx_ui_draw     (int dev);
void uigfx_ui_get_size (int dev, int *w, int *h);
void uigfx_ui_set_pos  (int dev, int x, int y);
void uigfx_ui_get_pos  (int dev, int *x, int *y);

/***************************************************/
/***************************************************/
/***************************************************/

int uigfx_device_size()
{
  return sizeof(struct uigfx_t);
}

int uigfx_device_create(int dev_num, char** xpm)
{
  struct uigfx_t *dev = (struct uigfx_t*) machine.device[dev_num].data;

  dev->img      = uigfx_xpm_create(xpm);
  dev->update   = 0;
  dev->val      = 0;

  machine.device[dev_num].reset         = uigfx_reset;
  machine.device[dev_num].delete        = uigfx_delete;

  machine.device[dev_num].write         = uigfx_write;
  machine.device[dev_num].update        = uigfx_update;

  machine.device[dev_num].ui_draw       = uigfx_ui_draw;
  machine.device[dev_num].ui_get_size   = uigfx_ui_get_size;
  machine.device[dev_num].ui_set_pos    = uigfx_ui_set_pos;
  machine.device[dev_num].ui_get_pos    = uigfx_ui_get_pos;

  machine.device[dev_num].state_size    = uigfx_device_size();
  machine.device[dev_num].name          = NULL;

  return 0;
}

int uigfx_reset(int dev)
{
  ST_VAL    = 0;
  ST_UPDATE = 1;
  return 0;
}

int uigfx_delete(int dev)
{
  uigfx_img_delete(ST_IMG);
  return 0;
}

/***************************************************/
/***************************************************/
/***************************************************/

void uigfx_write(int dev, uint32_t UNUSED addr, uint32_t val)
{
  ST_VAL = val;
  ST_UPDATE = 1;
}

int uigfx_update(int UNUSED dev)
{
  return 0;
}

/***************************************************/
/***************************************************/
/***************************************************/

int uigfx_ui_draw (int dev)
{
  if (ST_UPDATE == 1)
    {
      uigfx_img_draw (ST_IMG);
      ST_UPDATE = 0;
      return 1;
    }
  return 0;
}

void uigfx_ui_get_size (int dev, int *w, int *h)
{
  *w = ST_IMG->w;
  *h = ST_IMG->h;
}

void uigfx_ui_set_pos (int dev, int x, int y)
{
  ST_IMG->x = x;
  ST_IMG->y = y;
}

void uigfx_ui_get_pos (int dev, int *x, int *y)
{
  *x = ST_IMG->x;
  *y = ST_IMG->y;
}


/***************************************************/
/***************************************************/
/***************************************************/
