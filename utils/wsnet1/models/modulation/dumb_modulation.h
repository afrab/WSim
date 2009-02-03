/*
 *  dumb_modulation.h
 *  
 *
 *  Created by Guillaume Chelius on 02/08/05.
 *  Copyright 2005 __WorldSens__. All rights reserved.
 *
 */
#ifndef _DUMB_MODULATION_H_
#define _DUMB_MODULATION_H_


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
#include "../common.h"


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
int dumb_modulation_instantiate (FILE * config_fd);
double dumb_modulation_compute_BER (double SiNR);
int dumb_modulation_complete (void);


#endif
