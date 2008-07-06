
/**
 *  \file   bargraph_img.h
 *  \brief  bargraph bitmap image definition
 *  \author Antoine Fraboulet
 *  \date   2005
 **/

#ifndef _BARGRAPH_IMG_H
#define _BARGRAPH_IMG_H

struct bargraph_img_t {
  int x;
  int y;
  int w;
  int h;
  uint32_t on;
  uint32_t off;
  uint32_t bg;
  short** img;
};

struct bargraph_img_t* bargraph_img_create(uint32_t on, uint32_t off, uint32_t bg);
void               bargraph_img_delete(struct bargraph_img_t *img);
void               bargraph_img_print (struct bargraph_img_t *img);
void               bargraph_img_draw  (struct bargraph_img_t *img, int value);

#endif
