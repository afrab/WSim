/*
 *  range_propagation.c
 *  
 *
 *  Created by Guillaume Chelius on 02/08/05.
 *  Copyright 2005 __WorldSens__. All rights reserved.
 *
 */
#ifndef _RANGE_PROPAGATION_H_
#define _RANGE_PROPAGATION_H_


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
#include "../common.h"


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
int range_propagation_instantiate (FILE * config_fd);
double range_propagation_propagation (double mW, double x0, double y0, double z0, double x1, double y1, double z1);
int range_propagation_complete (void);


#endif
