
/**
 *  \file   uigfx_img.c
 *  \brief  XPM image manipulation
 *  \author Antoine Fraboulet
 *  \date   2008
 **/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "config.h"
#include "arch/common/hardware.h"
#include "uigfx_img.h"

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

#if defined(DEBUG)
#define UIIMG_DBG(x...) VERBOSE(3,x)
#else
#define UIIMG_DBG(x...) do { } while (0)
#endif

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

struct uigfx_map_t {
  char name[4];
  uint8_t r;
  uint8_t g;
  uint8_t b;
};

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

#define IMG_LINEAR(i,j) (img->w*i + j)

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

static int uigfx_hexchar2int(unsigned char ch)
{
  if (ch >= '0' && ch <= '9')
    return ch-'0';
  if (ch >= 'a' && ch <= 'f')
    return ch-'a'+10;
  if (ch >= 'A' && ch <= 'F')
    return ch-'A'+10;
  return -1;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

static void uigfx_xpm_parsecolor(char *str, struct uigfx_map_t *cmap, int nc, int cpp)
{
  char l1,l2,l3, r1,r2, g1,g2, b1,b2;
  l1 = 0;
  l2 = 0;
  l3 = 0;

  switch (cpp)
    {
    case 1:
      sscanf(str, "%c c #%c%c%c%c%c%c", &l1, &r1,&r2, &g1,&g2, &b1,&b2);
      // UIIMG_DBG("ui_img_xmp:R: %c c #%c%c%c%c%c%c\n", l1, r1,r2, g1,g2, b1,b2);
      break;
    case 2:
      sscanf(str, "%c%c c #%c%c%c%c%c%c", &l1,&l2, &r1,&r2, &g1,&g2, &b1,&b2);
      // UIIMG_DBG("ui_img_xmp:R: %c%c c #%c%c%c%c%c%c\n", l1,l2, r1,r2, g1,g2, b1,b2);
      break;
    case 3:
      sscanf(str, "%c%c%c c #%c%c%c%c%c%c", &l1,&l2,&l3, &r1,&r2, &g1,&g2, &b1,&b2);
      // UIIMG_DBG("ui_img_xmp:R: %c%c%c c #%c%c%c%c%c%c\n", l1,l2,l3, r1,r2, g1,g2, b1,b2);
      break;
    }

  cmap[nc].name[0] = l1;
  cmap[nc].name[1] = l2;
  cmap[nc].name[2] = l3;
  cmap[nc].name[3] = 0;
  cmap[nc].r       = uigfx_hexchar2int(r1) << 4 | uigfx_hexchar2int(r2);
  cmap[nc].g       = uigfx_hexchar2int(g1) << 4 | uigfx_hexchar2int(g2);
  cmap[nc].b       = uigfx_hexchar2int(b1) << 4 | uigfx_hexchar2int(b2);
  
  /*
  UIIMG_DBG("ui_img_xpm:C: %02d %2s c #%02x%02x%02x\n", nc, 
	    cmap[nc].name, cmap[nc].r, cmap[nc].g, cmap[nc].b);
  */
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

static void uigfx_readccstr(char *xpmline, int col, int cpp, char *str)
{
  int i;
  for (i=0; i<cpp; i++)
    {
      str[i] = xpmline[col*cpp + i];
    }
  str[i] = 0;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

static int uigfx_readpixel(char *str, struct uigfx_map_t *map, int ncolors, struct uigfx_pixel_t *pixel)
{
  int i;
  for(i=0; i<ncolors; i++)
    {
      if (strcmp(map[i].name, str) == 0)
	{
	  pixel->r = map[i].r;
	  pixel->g = map[i].g;
	  pixel->b = map[i].b;
	  return i;
	}
    }
  return 0;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

struct uigfx_img_t* uigfx_xpm_create(char **xpm)
{
  int i,j;
  int cpp;
  struct uigfx_img_t* img = NULL;
  struct uigfx_map_t* map = NULL;

  img = (struct uigfx_img_t*)malloc(sizeof(struct uigfx_img_t));
  if (img == NULL)
    {
      return NULL;
    }
  /* scan size */
  sscanf(xpm[0],"%d %d %d %d",&(img->w),&(img->h),&(img->ncolors),&cpp);
  /* UIIMG_DBG("uigfx_img: width=%d height=%d ncolors=%d cpp=%d\n",img->w,img->h,img->ncolors,cpp); */
  img->pixels = (struct uigfx_pixel_t*)malloc(img->w * img->h * sizeof(struct uigfx_pixel_t));
  if (img->pixels == NULL)
    {
      free(img);
      return NULL;
    }

  map = (struct uigfx_map_t*)malloc(img->ncolors * sizeof(struct uigfx_map_t));
  if (map == NULL)
    {
      free(img);
      return NULL;
    }
  memset(map, 0, img->ncolors * sizeof(struct uigfx_map_t));

  /* scan colors */
  for(i=0; i < img->ncolors; i++)
    {
      uigfx_xpm_parsecolor(xpm[1+i], map, i, cpp);
    }

  /* scan pixels */
  for(i=0; i < img->h; i++)
    {
      for(j=0; j < img->w; j++)
	{
	  char str[4];
	  int ncolor;
	  uigfx_readccstr(xpm[1+img->ncolors+i], j, cpp, str);
	  ncolor = uigfx_readpixel(str, map, img->ncolors, & img->pixels[IMG_LINEAR(i,j)]);
	  /* ERROR("%02d",ncolor); */
	}
      /* ERROR("\n"); */
    }

  return img;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void uigfx_img_delete(struct uigfx_img_t *img)
{
  if (img->pixels)
    {
      free(img->pixels);
      img->pixels = NULL;
    }
  free(img);
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void uigfx_img_draw(struct uigfx_img_t *img)
{
  int i,j;
  int pixel;
  static int once = 1;

  if ((img == NULL) && (once == 1))
    {
      ERROR("uigfx: img_draw NULL pointer defined image\n");
      once = 0;
      return;
    }

  pixel = (img->x + img->y*machine.ui.width)*machine.ui.bpp;
  for(i=0; i < img->h; i++)
    {
      int pii = pixel;
      for(j=0; j < img->w; j++)
	{
	  struct uigfx_pixel_t *p;
	  p = & img->pixels[IMG_LINEAR(i,j)];
	  setpixel(pii,p->r,p->g,p->b);
	  pii += 3;
	}
      pixel += machine.ui.width * machine.ui.bpp;
    }
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */
