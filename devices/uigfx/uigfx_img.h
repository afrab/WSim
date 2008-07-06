
/**
 *  \file   uigfx_img.h
 *  \brief  XPM image manipulation
 *  \author Antoine Fraboulet
 *  \date   2008
 **/

#ifndef _UIGFX_IMG_H
#define _UIGFX_IMG_H

struct uigfx_pixel_t {
  unsigned char r;
  unsigned char g;
  unsigned char b;
};

struct uigfx_img_t {
  int x;
  int y;
  int w;
  int h;
  int ncolors;
  struct uigfx_pixel_t* pixels;
};

struct uigfx_img_t* uigfx_xpm_create(char **xpm);
void                uigfx_img_delete(struct uigfx_img_t *img);
void                uigfx_img_draw  (struct uigfx_img_t *img);

#endif
