/*
 *  no_fading_propagation.h
 *  
 *
 *  Created by Guillaume Chelius on 02/08/05.
 *  Copyright 2005 __WorldSens__. All rights reserved.
 *
 */
#ifndef _NO_FADING_PROPAGATION_H_
#define _NO_FADING_PROPAGATION_H_


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
#include "../common.h"


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
int no_fading_propagation_instantiate (FILE * config_fd);
double no_fading_propagation_propagation (double mW, double x0, double y0, double z0, double x1, double y1, double z1);
int no_fading_propagation_complete (void);


#endif
