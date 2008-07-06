
/**
 *  \file   bargraph_dev.h
 *  \brief  Bargraph display line
 *  \author Antoine Fraboulet
 *  \date   2005
 **/

#ifndef BARGRAPH_DEVICES_H
#define BARGRAPH_DEVICES_H

/**
 *
 **/
int  bargraph_device_create (int dev_num, uint32_t on, uint32_t off, uint32_t bg);
int  bargraph_device_size   (void);

#endif

