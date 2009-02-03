/*
 *  antenna.h
 *  
 *
 *  Created by Guillaume Chelius on 20/11/05.
 *  Copyright 2005 __WorldSens__. All rights reserved.
 *
 */
#ifndef _ANTENNA_H
#define _ANTENNA_H


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
#include <public/types.h>


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
int antenna_tx (struct _node * node, struct _packet * packet);
int antenna_get_ioctl (struct _node * node, int ioctl, void * val);
int antenna_set_ioctl (struct _node * node, int ioctl, void * val);
double antenna_get_orientation (struct _node * node);
void antenna_set_orientation (struct _node * node, double degree);
double antenna_get_noise_mW (struct _node * node);
double antenna_get_min_rx_mW (struct _node * node);


#endif //_ANTENNA_H
