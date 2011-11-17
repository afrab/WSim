/**
 *  \file   ez430_lcd_img.h
 *  \brief  ez430_lcd bitmap image definition
 *  \author Bernhard Dick
 *  \date   2011
 **/

#ifndef _EZ430_LCD_IMG_H
#define _EZ430_LCD_IMG_H

struct ez430_lcd_img_t {
  int x;
  int y;
  int w;
  int h;
  uint8_t*** img;
};

struct ez430_lcd_img_t* ez430_lcd_img_create();
void ez430_lcd_img_delete(struct ez430_lcd_img_t *img);
void ez430_lcd_img_print(struct ez430_lcd_img_t *img);
void ez430_lcd_img_draw(struct ez430_lcd_img_t *img, uint8_t mem[15], uint8_t bmem[15]);

#endif
