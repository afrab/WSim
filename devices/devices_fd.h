
/**
 *  \file   devices_fd.h
 *  \brief  Platform devices forward definitions
 *  \author Antoine Fraboulet
 *  \date   2005
 **/

#ifndef HW_DEVICES_H
#define HW_DEVICES_H

/**
 * peripheral description
 **/

struct device_t
{
  int  (*update)        (int self);
  void (*read)          (int self, uint32_t *mask, uint32_t *value);
  void (*write)         (int self, uint32_t  mask, uint32_t  value);

  // create is static
  // size   is static
  int  (*delete)        (int self);

  int  (*reset)         (int self);
  int  (*power_up)      (int self);
  int  (*power_down)    (int self);

  int  (*ui_draw)       (int self);
  void (*ui_set_pos)    (int self, int  x, int  y);
  void (*ui_get_pos)    (int self, int *x, int *y);
  void (*ui_get_size)   (int self, int *width, int *height);

  void (*statdump)      (int self, int64_t user_nanotime);

  int state_size;
  int dev_num;

  char* name;
  void* data;
};

/**
 * Device update function called after each instruction step.
 * This function is defined in devices/devices.c
 **/
int devices_update();

#endif
