
/**
 *  \file   led_img.c
 *  \brief  Led bitmap image definition
 *  \author Antoine Fraboulet
 *  \date   2005
 **/

#include <stdio.h>
#include <stdlib.h>

#include "arch/common/hardware.h"
#include "led_img.h"
  
#define P1 '.'     // points

#define BKG2 ' '
#define BKG1 'X'

#define NCOLORS 1

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

struct led_img_t *led_img_create(uint32_t on, uint32_t off, uint32_t bg)
{
  int i,j,k;
  struct led_img_t *img;
  int ncolors, bpp;

  char ledcolors[] = { P1 };

#include "led.bitmap"

  img = (struct led_img_t*)malloc(sizeof(struct led_img_t));

  sscanf(ledimg[0],"%d %d %d %d",&(img->w),&(img->h),&ncolors,&bpp);

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
	      if (ledimg[i+1+ncolors][j] == ledcolors[k])
		{
		  img->img[j][i] = 1 << k;
		}
	    }
	}
    }
  
  //  led_img_print(img);
  return img;
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

void   led_img_delete(struct led_img_t *img)
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

void   led_img_draw  (struct led_img_t *img, int value)
{
  int i,j;
  int pixel;

  pixel = (img->x + img->y*machine.ui.width)*machine.ui.bpp;
  for(i=0; i < img->h; i++)
    {
      int pii = pixel;
      for(j=0; j < img->w; j++)
	{
	  if (value && (img->img[j][i] > 0))
	    {
	      setpixel(pii,(img->on  >> 16) & 0xff, (img->on >> 8) & 0xff,  img->on & 0xff); /* on */
	    }
	  else if (img->img[j][i] > 0)
	    {
	      setpixel(pii,(img->off >> 16) & 0xff, (img->off >> 8) & 0xff, img->off & 0xff); /* off */
	    }
	  else
	    {
	      setpixel(pii,(img->bg >> 16) & 0xff, (img->bg >> 8) & 0xff,   img->bg & 0xff); /* bkg */
	    }
	  pii += 3;
	}
      pixel += machine.ui.width * machine.ui.bpp;
    }
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */

#define HW_DEBUG_LED(x...) VERBOSE(10,x)

void
led_img_print(struct led_img_t *img)
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
		      HW_DEBUG_LED("%c",k+'0');
		    }
		}
	    }
	  else
	    {
	      HW_DEBUG_LED(" ");
	    }
	}
      HW_DEBUG_LED("\n");
    }
}

/* ************************************************** */
/* ************************************************** */
/* ************************************************** */
