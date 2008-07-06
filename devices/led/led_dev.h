
/**
 *  \file   led_dev.h
 *  \brief  Led display 
 *  \author Antoine Fraboulet
 *  \date   2005
 **/

#ifndef LED_DEVICES_H
#define LED_DEVICES_H

#define LED_DATA 1

/**
 *
 **/
int  led_device_create (int dev_num, uint32_t on, uint32_t off, uint32_t bg);
int  led_device_size   ();

#endif

