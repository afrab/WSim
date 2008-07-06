
/**
 *  \file   hd44780_dev.h
 *  \brief  HD44780U (LCD-II) Hitachi dot-matrix LCD
 *  \author Antoine Fraboulet
 *  \date   2008
 **/

#ifndef HD44_DEVICES_H
#define HD44_DEVICES_H

/**
 *
 */

#define HD44_RS_S          8
#define HD44_RW_S          9
#define HD44_E_S          10
#define HD44_D0D3_S        0
#define HD44_D4D7_S        4

#define HD44_RS       0x0100
#define HD44_RW       0x0200
#define HD44_E        0x0400
#define HD44_D0D3     0x000f
#define HD44_D4D7     0x00f0
#define HD44_D0D7     0x00ff


int  hd44_device_create (int dev_num, uint32_t on, uint32_t off, uint32_t bg);
int  hd44_device_size   (void);

#endif
