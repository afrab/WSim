/**
 *  \file   ez430_lcd_dev.h
 *  \brief  ez430-chronos LCD
 *  \author Bernhard Dick
 *  \date   2011
 **/

#ifndef EZ430_LCD_DEV_H
#define EZ430_LCD_DEV_H

/**
 *
 **/
int  ez430_lcd_device_create (int dev_num);
int  ez430_lcd_device_size   ();

int  ez430_lcd_reset       (int dev);
int  ez430_lcd_delete      (int dev);
void ez430_lcd_write       (int dev, uint32_t mask, uint32_t val);
int  ez430_lcd_update      (int dev);
int  ez430_lcd_ui_draw     (int dev);
void ez430_lcd_ui_get_size (int dev, int *w, int *h);
void ez430_lcd_ui_set_pos  (int dev, int  x, int  y);
void ez430_lcd_ui_get_pos  (int dev, int *x, int *y);

void ez430_lcd_regwrite    (int dev, uint8_t mem[15], uint8_t bmem[15]);

#endif

