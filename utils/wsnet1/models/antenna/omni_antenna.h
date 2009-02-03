/*
 *  omni_antenna.c
 *  
 *
 *  Created by Guillaume Chelius on 02/08/05.
 *  Copyright 2005 __WorldSens__. All rights reserved.
 *
 */
#ifndef _OMNI_ANTENNA_H_
#define _OMNI_ANTENNA_H_


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
#include "../common.h"


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
int omni_antenna_instantiate (struct _node * node, FILE * config_fd);
double omni_antenna_compute_tx (struct _node * node, double mW, double x, double y, double z);
double omni_antenna_compute_rx (struct _node * node, double mW, double x, double y, double z);
int omni_antenna_get_ioctl (struct _node * node, int ioctl, void * arg);
int omni_antenna_set_ioctl (struct _node * node, int ioctl, void * arg);
int omni_antenna_complete (struct _node * node);


#endif
