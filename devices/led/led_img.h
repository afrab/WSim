
/**
 *  \file   led_img.h
 *  \brief  led bitmap image definition
 *  \author Antoine Fraboulet
 *  \date   2006
 **/

#ifndef _LED_IMG_H
#define _LED_IMG_H

struct led_img_t {
  int x;
  int y;
  int w;
  int h;
  uint32_t on;
  uint32_t off;
  uint32_t bg;
  short** img;
};

struct led_img_t* led_img_create(uint32_t on, uint32_t off, uint32_t bg);
void              led_img_delete(struct led_img_t *img);
void              led_img_print (struct led_img_t *img);
void              led_img_draw  (struct led_img_t *img, int value);

#endif
