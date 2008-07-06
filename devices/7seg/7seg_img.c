
/**
 *  \file   7seg_img.c
 *  \brief  7seg bitmap image definition
 *  \author Antoine Fraboulet
 *  \date   2005
 **/

#include <stdio.h>
#include <stdlib.h>

#include "arch/common/hardware.h"
#include "7seg_img.h"
  

#define P1 '%'     // top
#define P2 '.'     // top right
#define P3 '#'     // bottom right
#define P4 '$'     // bottom
#define P5 '+'     // bottom left 
#define P6 'X'     // top left
#define P7 'O'     // middle
#define P8 'o'     // point

#define BKG2 ' '
#define BKG1 '@'

#define NCOLORS 8

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

struct seg7_img_t *seg7_img_create()
{
  int i,j,k;
  struct seg7_img_t *img;
  int ncolors, bpp;

  char ledcolors[] = { P1, P2, P3, P4, P5, P6, P7, P8 };

#include "7segimg.bitmap"

  img = (struct seg7_img_t*)malloc(sizeof(struct seg7_img_t));

  sscanf(seg7img[0],"%d %d %d %d",&(img->w),&(img->h),&ncolors,&bpp);

  img->img = (short**)malloc(img->w*sizeof(short*));

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
	      if (seg7img[i+1+ncolors][j] == ledcolors[k])
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

void   seg7_img_delete(struct seg7_img_t *img)
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

void   seg7_img_draw  (struct seg7_img_t *img, int value)
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
	      setpixel(pii,0xee,0x00,0x00); /* on */
	    }
	  else if (img->img[j][i] > 0)
	    {
	      setpixel(pii,0x30,0x30,0x30); /* off */
	    }
	  else
	    {
	      setpixel(pii,0x00,0x00,0x00); /* bkg */
	    }
	  pii += 3;
	}
      pixel += machine.ui.width * machine.ui.bpp;
    }
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

#define HW_DEBUG_7SEG(x...) VERBOSE(10,x)

void
seg7_img_print(struct seg7_img_t *img)
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
		      HW_DEBUG_7SEG("%c",k+'0');
		    }
		}
	    }
	  else
	    {
	      HW_DEBUG_7SEG(" ");
	    }
	}
      HW_DEBUG_7SEG("\n");
    }
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */
