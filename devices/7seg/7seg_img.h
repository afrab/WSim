
/**
 *  \file   7seg_img.h
 *  \brief  7seg bitmap image definition
 *  \author Antoine Fraboulet
 *  \date   2005
 **/

#ifndef _7SEG_IMG_H
#define _7SEG_IMG_H

struct seg7_img_t {
  int x;
  int y;
  int w;
  int h;
  short** img;
};

struct seg7_img_t* seg7_img_create();
void               seg7_img_delete(struct seg7_img_t *img);
void               seg7_img_print (struct seg7_img_t *img);
void               seg7_img_draw  (struct seg7_img_t *img, int value);

#endif
