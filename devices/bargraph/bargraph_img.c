
/**
 *  \file   bargraph_img.c
 *  \brief  bargraph bitmap image definition
 *  \author Antoine Fraboulet
 *  \date   2005
 **/

#include <stdio.h>
#include <stdlib.h>

#include "arch/common/hardware.h"
#include "bargraph_img.h"
  

#define P1 'o'     // bar led 0
#define P2 '+'     // bar led 1
#define P3 'X'     // bar led 2
#define P4 '#'     // bar led 3
#define P5 '%'     // bar led 4 
#define P6 'O'     // bar led 5
#define P7 '$'     // bar led 6
#define P8 '.'     // bar led 7

#define BKG2 ' '
#define BKG1 '@'

#define NCOLORS 8

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

struct bargraph_img_t *bargraph_img_create(uint32_t on, uint32_t off, uint32_t bg)
{
  int i,j,k;
  struct bargraph_img_t *img;
  int ncolors, bpp;

  char ledcolors[] = { P1, P2, P3, P4, P5, P6, P7, P8 };

#include "bargraph.bitmap"

  img = (struct bargraph_img_t*)malloc(sizeof(struct bargraph_img_t));

  sscanf(noname[0],"%d %d %d %d",&(img->w),&(img->h),&ncolors,&bpp);

  img->img = (short**)malloc(img->w*sizeof(short*));
  img->on  = on;
  img->off = off;
  img->bg  = bg;

  for(i=0; i < img->w; i++)
    {
      img->img[i] = (short*)malloc(img->h*sizeof(short));
    }

  for(i=0; i < img->h; i++)
    {
      for(j=0; j < img->w; j++)
	{
	  img->img[j][i] = 0;
	  for(k=0; k < NCOLORS; k++)
	    {
	      if (noname[i+1+ncolors][j] == ledcolors[k])
		{
		  img->img[j][i] = 1 << k;
		}
	    }
	}
    }
  return img;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void   bargraph_img_delete(struct bargraph_img_t *img)
{
  int i;
  for(i=0; i < img->w; i++)
    {
      free(img->img[i]);
    }
  free(img->img);
  free(img);
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void   bargraph_img_draw  (struct bargraph_img_t *img, int value)
{
  int i,j;
  int pixel;
 
  pixel = (img->x + img->y*machine.ui.width)*machine.ui.bpp;
  for(i=0; i < img->h; i++)
    {
      int pii = pixel;
      for(j=0; j < img->w; j++)
	{
	  if ((value & img->img[j][i]) != 0)
	    {
	      setpixel(pii,(img->on >> 16) & 0xff, (img->on >> 8) & 0xff, img->on & 0xff); /* on */
	    }
	  else if (img->img[j][i] > 0)
	    {
	      setpixel(pii,(img->off >> 16) & 0xff, (img->off >> 8) & 0xff, img->off & 0xff); /* off */
	    }
	  else
	    {
	      setpixel(pii,(img->bg >> 16) & 0xff, (img->bg >> 8) & 0xff, img->bg & 0xff); /* bkg */
	    }
	  pii += 3;
	}
      pixel += machine.ui.width * machine.ui.bpp;
    }
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

#define HW_DEBUG_BGRAPH(x...) VERBOSE(10,x)

void
bargraph_img_print(struct bargraph_img_t *img)
{
  int i,j,k;
  
  for(i=0; i<img->h; i++)
    {
      for(j=0; j<img->w; j++)
	{
	  if (img->img[j][i])
	    {
	      for(k=0; k<=NCOLORS; k++)
		{
		  if (img->img[j][i] & (1 << k))
		    {
		      HW_DEBUG_BGRAPH("%c",k+'0');
		    }
		}
	    }
	  else
	    {
	      HW_DEBUG_BGRAPH(" ");
	    }
	}
      HW_DEBUG_BGRAPH("\n");
    }
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */
