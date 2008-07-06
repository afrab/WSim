
/**
 *  \file   ptty_dev.h
 *  \brief  serial ptty IO 
 *  \author Antoine Fraboulet
 *  \date   2005
 **/

#ifndef PTTY_DEVICES_H
#define PTTY_DEVICES_H

/**
 *
 **/

#define PTTY_D 0xffu

int  ptty_device_size   ();
int  ptty_device_create (int dev_num, int id, const char *filename);

#endif
